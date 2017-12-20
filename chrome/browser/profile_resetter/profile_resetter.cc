// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/profile_resetter/profile_resetter.h"

#include <stddef.h>

#include <string>

#include "base/macros.h"
#include "base/synchronization/cancellation_flag.h"
#include "build/build_config.h"
#include "chrome/browser/browsing_data/browsing_data_helper.h"
#include "chrome/browser/browsing_data/browsing_data_remover.h"
#include "chrome/browser/browsing_data/browsing_data_remover_factory.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/google/google_url_tracker_factory.h"
#include "chrome/browser/profile_resetter/brandcoded_default_settings.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/pref_names.h"
#include "chrome/installer/util/browser_distribution.h"
#include "components/content_settings/core/browser/content_settings_info.h"
#include "components/content_settings/core/browser/content_settings_registry.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/browser/website_settings_info.h"
#include "components/content_settings/core/browser/website_settings_registry.h"
#include "components/google/core/browser/google_url_tracker.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/search_engines/search_engines_pref_names.h"
#include "components/search_engines/template_url_prepopulate_data.h"
#include "components/search_engines/template_url_service.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/management_policy.h"

#if defined(OS_WIN)
#include "base/base_paths.h"
#include "base/path_service.h"
#include "chrome/installer/util/shell_util.h"

namespace {

void ResetShortcutsOnFileThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);
  // Get full path of chrome.
  base::FilePath chrome_exe;
  if (!PathService::Get(base::FILE_EXE, &chrome_exe))
    return;
  BrowserDistribution* dist = BrowserDistribution::GetSpecificDistribution(
      BrowserDistribution::CHROME_BROWSER);
  for (int location = ShellUtil::SHORTCUT_LOCATION_FIRST;
       location < ShellUtil::NUM_SHORTCUT_LOCATIONS; ++location) {
    ShellUtil::ShortcutListMaybeRemoveUnknownArgs(
        static_cast<ShellUtil::ShortcutLocation>(location),
        dist,
        ShellUtil::CURRENT_USER,
        chrome_exe,
        true,
        NULL,
        NULL);
  }
}

}  // namespace
#endif  // defined(OS_WIN)

ProfileResetter::ProfileResetter(Profile* profile)
    : profile_(profile),
      template_url_service_(TemplateURLServiceFactory::GetForProfile(profile_)),
      pending_reset_flags_(0),
      cookies_remover_(nullptr),
      weak_ptr_factory_(this) {
  DCHECK(CalledOnValidThread());
  DCHECK(profile_);
}

ProfileResetter::~ProfileResetter() {
  if (cookies_remover_)
    cookies_remover_->RemoveObserver(this);
}

void ProfileResetter::Reset(
    ProfileResetter::ResettableFlags resettable_flags,
    std::unique_ptr<BrandcodedDefaultSettings> master_settings,
    const base::Closure& callback) {
  DCHECK(CalledOnValidThread());
  DCHECK(master_settings);

  // We should never be called with unknown flags.
  CHECK_EQ(static_cast<ResettableFlags>(0), resettable_flags & ~ALL);

  // We should never be called when a previous reset has not finished.
  CHECK_EQ(static_cast<ResettableFlags>(0), pending_reset_flags_);

  if (!resettable_flags) {
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                     callback);
    return;
  }

  master_settings_.swap(master_settings);
  callback_ = callback;

  // These flags are set to false by the individual reset functions.
  pending_reset_flags_ = resettable_flags;

  struct {
    Resettable flag;
    void (ProfileResetter::*method)();
  } flagToMethod[] = {
    {DEFAULT_SEARCH_ENGINE, &ProfileResetter::ResetDefaultSearchEngine},
    {HOMEPAGE, &ProfileResetter::ResetHomepage},
    {CONTENT_SETTINGS, &ProfileResetter::ResetContentSettings},
    {COOKIES_AND_SITE_DATA, &ProfileResetter::ResetCookiesAndSiteData},
    {EXTENSIONS, &ProfileResetter::ResetExtensions},
    {STARTUP_PAGES, &ProfileResetter::ResetStartupPages},
    {PINNED_TABS, &ProfileResetter::ResetPinnedTabs},
    {SHORTCUTS, &ProfileResetter::ResetShortcuts},
  };

  ResettableFlags reset_triggered_for_flags = 0;
  for (size_t i = 0; i < arraysize(flagToMethod); ++i) {
    if (resettable_flags & flagToMethod[i].flag) {
      reset_triggered_for_flags |= flagToMethod[i].flag;
      (this->*flagToMethod[i].method)();
    }
  }

  DCHECK_EQ(resettable_flags, reset_triggered_for_flags);
}

bool ProfileResetter::IsActive() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return pending_reset_flags_ != 0;
}

void ProfileResetter::MarkAsDone(Resettable resettable) {
  DCHECK(CalledOnValidThread());

  // Check that we are never called twice or unexpectedly.
  CHECK(pending_reset_flags_ & resettable);

  pending_reset_flags_ &= ~resettable;

  if (!pending_reset_flags_) {
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
                                     callback_);
    callback_.Reset();
    master_settings_.reset();
    template_url_service_sub_.reset();
  }
}

void ProfileResetter::ResetDefaultSearchEngine() {
  DCHECK(CalledOnValidThread());
  DCHECK(template_url_service_);
  // If TemplateURLServiceFactory is ready we can clean it right now.
  // Otherwise, load it and continue from ProfileResetter::Observe.
  if (template_url_service_->loaded()) {
    PrefService* prefs = profile_->GetPrefs();
    DCHECK(prefs);
    TemplateURLPrepopulateData::ClearPrepopulatedEnginesInPrefs(
        profile_->GetPrefs());
    std::unique_ptr<base::ListValue> search_engines(
        master_settings_->GetSearchProviderOverrides());
    if (search_engines) {
      // This Chrome distribution channel provides a custom search engine. We
      // must reset to it.
      ListPrefUpdate update(prefs, prefs::kSearchProviderOverrides);
      update->Swap(search_engines.get());
    }

    template_url_service_->RepairPrepopulatedSearchEngines();

    // Reset Google search URL.
    const TemplateURL* default_search_provider =
        template_url_service_->GetDefaultSearchProvider();
    if (default_search_provider &&
        default_search_provider->HasGoogleBaseURLs(
            template_url_service_->search_terms_data())) {
      GoogleURLTracker* tracker =
          GoogleURLTrackerFactory::GetForProfile(profile_);
      if (tracker)
        tracker->RequestServerCheck(true);
    }

    MarkAsDone(DEFAULT_SEARCH_ENGINE);
  } else {
    template_url_service_sub_ =
        template_url_service_->RegisterOnLoadedCallback(
            base::Bind(&ProfileResetter::OnTemplateURLServiceLoaded,
                       weak_ptr_factory_.GetWeakPtr()));
    template_url_service_->Load();
  }
}

void ProfileResetter::ResetHomepage() {
  DCHECK(CalledOnValidThread());
  PrefService* prefs = profile_->GetPrefs();
  DCHECK(prefs);
  std::string homepage;
  bool homepage_is_ntp, show_home_button;

  if (master_settings_->GetHomepage(&homepage))
    prefs->SetString(prefs::kHomePage, homepage);

  if (master_settings_->GetHomepageIsNewTab(&homepage_is_ntp))
    prefs->SetBoolean(prefs::kHomePageIsNewTabPage, homepage_is_ntp);
  else
    prefs->ClearPref(prefs::kHomePageIsNewTabPage);

  if (master_settings_->GetShowHomeButton(&show_home_button))
    prefs->SetBoolean(prefs::kShowHomeButton, show_home_button);
  else
    prefs->ClearPref(prefs::kShowHomeButton);
  MarkAsDone(HOMEPAGE);
}

void ProfileResetter::ResetContentSettings() {
  DCHECK(CalledOnValidThread());
  HostContentSettingsMap* map =
      HostContentSettingsMapFactory::GetForProfile(profile_);

  for (const content_settings::WebsiteSettingsInfo* info :
       *content_settings::WebsiteSettingsRegistry::GetInstance()) {
    map->ClearSettingsForOneType(info->type());
  }

  // TODO(raymes): The default value isn't really used for website settings
  // right now, but if it were we should probably reset that here too.
  for (const content_settings::ContentSettingsInfo* info :
       *content_settings::ContentSettingsRegistry::GetInstance()) {
    map->SetDefaultContentSetting(info->website_settings_info()->type(),
                                  CONTENT_SETTING_DEFAULT);
  }
  MarkAsDone(CONTENT_SETTINGS);
}

void ProfileResetter::ResetCookiesAndSiteData() {
  DCHECK(CalledOnValidThread());
  DCHECK(!cookies_remover_);

  cookies_remover_ = BrowsingDataRemoverFactory::GetForBrowserContext(profile_);
  cookies_remover_->AddObserver(this);
  int remove_mask = BrowsingDataRemover::REMOVE_SITE_DATA |
                    BrowsingDataRemover::REMOVE_CACHE;
  PrefService* prefs = profile_->GetPrefs();
  DCHECK(prefs);

  // Don't try to clear LSO data if it's not supported.
  if (!prefs->GetBoolean(prefs::kClearPluginLSODataEnabled))
    remove_mask &= ~BrowsingDataRemover::REMOVE_PLUGIN_DATA;
  cookies_remover_->RemoveAndReply(BrowsingDataRemover::Unbounded(),
                                   remove_mask,
                                   BrowsingDataHelper::UNPROTECTED_WEB, this);
}

void ProfileResetter::ResetExtensions() {
  DCHECK(CalledOnValidThread());

  std::vector<std::string> brandcode_extensions;
  master_settings_->GetExtensions(&brandcode_extensions);

  ExtensionService* extension_service =
      extensions::ExtensionSystem::Get(profile_)->extension_service();
  DCHECK(extension_service);
  extension_service->DisableUserExtensions(brandcode_extensions);

  MarkAsDone(EXTENSIONS);
}

void ProfileResetter::ResetStartupPages() {
  DCHECK(CalledOnValidThread());
  PrefService* prefs = profile_->GetPrefs();
  DCHECK(prefs);
  std::unique_ptr<base::ListValue> url_list(
      master_settings_->GetUrlsToRestoreOnStartup());
  if (url_list)
    ListPrefUpdate(prefs, prefs::kURLsToRestoreOnStartup)->Swap(url_list.get());

  int restore_on_startup;
  if (master_settings_->GetRestoreOnStartup(&restore_on_startup))
    prefs->SetInteger(prefs::kRestoreOnStartup, restore_on_startup);
  else
    prefs->ClearPref(prefs::kRestoreOnStartup);

  MarkAsDone(STARTUP_PAGES);
}

void ProfileResetter::ResetPinnedTabs() {
  // Unpin all the tabs.
  for (auto* browser : *BrowserList::GetInstance()) {
    if (browser->is_type_tabbed() && browser->profile() == profile_) {
      TabStripModel* tab_model = browser->tab_strip_model();
      // Here we assume that indexof(any mini tab) < indexof(any normal tab).
      // If we unpin the tab, it can be moved to the right. Thus traversing in
      // reverse direction is correct.
      for (int i = tab_model->count() - 1; i >= 0; --i) {
        if (tab_model->IsTabPinned(i))
          tab_model->SetTabPinned(i, false);
      }
    }
  }
  MarkAsDone(PINNED_TABS);
}

void ProfileResetter::ResetShortcuts() {
#if defined(OS_WIN)
  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&ResetShortcutsOnFileThread),
      base::Bind(&ProfileResetter::MarkAsDone,
                 weak_ptr_factory_.GetWeakPtr(),
                 SHORTCUTS));
#else
  MarkAsDone(SHORTCUTS);
#endif
}

void ProfileResetter::OnTemplateURLServiceLoaded() {
  // TemplateURLService has loaded. If we need to clean search engines, it's
  // time to go on.
  DCHECK(CalledOnValidThread());
  template_url_service_sub_.reset();
  if (pending_reset_flags_ & DEFAULT_SEARCH_ENGINE)
    ResetDefaultSearchEngine();
}

void ProfileResetter::OnBrowsingDataRemoverDone() {
  cookies_remover_->RemoveObserver(this);
  cookies_remover_ = nullptr;
  MarkAsDone(COOKIES_AND_SITE_DATA);
}

std::vector<ShortcutCommand> GetChromeLaunchShortcuts(
    const scoped_refptr<SharedCancellationFlag>& cancel) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);
#if defined(OS_WIN)
  // Get full path of chrome.
  base::FilePath chrome_exe;
  if (!PathService::Get(base::FILE_EXE, &chrome_exe))
    return std::vector<ShortcutCommand>();
  BrowserDistribution* dist = BrowserDistribution::GetSpecificDistribution(
      BrowserDistribution::CHROME_BROWSER);
  std::vector<ShortcutCommand> shortcuts;
  for (int location = ShellUtil::SHORTCUT_LOCATION_FIRST;
       location < ShellUtil::NUM_SHORTCUT_LOCATIONS; ++location) {
    if (cancel.get() && cancel->data.IsSet())
      break;
    ShellUtil::ShortcutListMaybeRemoveUnknownArgs(
        static_cast<ShellUtil::ShortcutLocation>(location),
        dist,
        ShellUtil::CURRENT_USER,
        chrome_exe,
        false,
        cancel,
        &shortcuts);
  }
  return shortcuts;
#else
  return std::vector<ShortcutCommand>();
#endif
}
