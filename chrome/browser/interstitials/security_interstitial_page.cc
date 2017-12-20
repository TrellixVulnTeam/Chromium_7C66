// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/interstitials/security_interstitial_page.h"

#include <utility>

#include "base/i18n/rtl.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/interstitials/chrome_controller_client.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/grit/components_resources.h"
#include "components/prefs/pref_service.h"
#include "components/safe_browsing_db/safe_browsing_prefs.h"
#include "components/security_interstitials/core/common_string_util.h"
#include "components/security_interstitials/core/metrics_helper.h"
#include "content/public/browser/interstitial_page.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"
#include "ui/base/webui/web_ui_util.h"

SecurityInterstitialPage::SecurityInterstitialPage(
    content::WebContents* web_contents,
    const GURL& request_url,
    std::unique_ptr<security_interstitials::MetricsHelper> metrics_helper)
    : web_contents_(web_contents),
      request_url_(request_url),
      interstitial_page_(NULL),
      create_view_(true),
      controller_(
          new ChromeControllerClient(web_contents, std::move(metrics_helper))) {
  // Creating interstitial_page_ without showing it leaks memory, so don't
  // create it here.
}

SecurityInterstitialPage::~SecurityInterstitialPage() {
}

content::InterstitialPage* SecurityInterstitialPage::interstitial_page() const {
  return interstitial_page_;
}

content::WebContents* SecurityInterstitialPage::web_contents() const {
  return web_contents_;
}

GURL SecurityInterstitialPage::request_url() const {
  return request_url_;
}

void SecurityInterstitialPage::DontCreateViewForTesting() {
  create_view_ = false;
}

void SecurityInterstitialPage::Show() {
  DCHECK(!interstitial_page_);
  interstitial_page_ = content::InterstitialPage::Create(
      web_contents_, ShouldCreateNewNavigation(), request_url_, this);
  if (!create_view_)
    interstitial_page_->DontCreateViewForTesting();

  // Determine if any prefs need to be updated prior to showing the security
  // interstitial.
  safe_browsing::UpdatePrefsBeforeSecurityInterstitial(profile()->GetPrefs());
  interstitial_page_->Show();

  controller_->set_interstitial_page(interstitial_page_);
  AfterShow();
}

Profile* SecurityInterstitialPage::profile() {
  return Profile::FromBrowserContext(web_contents()->GetBrowserContext());
}

bool SecurityInterstitialPage::IsPrefEnabled(const char* pref) {
  return profile()->GetPrefs()->GetBoolean(pref);
}

ChromeControllerClient* SecurityInterstitialPage::controller() {
  return controller_.get();
}

security_interstitials::MetricsHelper*
SecurityInterstitialPage::metrics_helper() {
  return controller_->metrics_helper();
}

base::string16 SecurityInterstitialPage::GetFormattedHostName() const {
  return security_interstitials::common_string_util::GetFormattedHostName(
      request_url_);
}

std::string SecurityInterstitialPage::GetHTMLContents() {
  base::DictionaryValue load_time_data;
  PopulateInterstitialStrings(&load_time_data);
  const std::string& app_locale = g_browser_process->GetApplicationLocale();
  webui::SetLoadTimeDataDefaults(app_locale, &load_time_data);
  std::string html = ResourceBundle::GetSharedInstance()
                         .GetRawDataResource(IDR_SECURITY_INTERSTITIAL_HTML)
                         .as_string();
  webui::AppendWebUiCssTextDefaults(&html);
  return webui::GetI18nTemplateHtml(html, &load_time_data);
}
