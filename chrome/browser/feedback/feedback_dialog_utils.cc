// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/feedback/feedback_dialog_utils.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/ui/ash/multi_user/multi_user_util.h"
#include "chrome/browser/ui/ash/multi_user/multi_user_window_manager.h"
#include "chrome/browser/ui/browser_window.h"
#include "components/signin/core/account_id/account_id.h"
#endif

namespace chrome {

GURL GetTargetTabUrl(int session_id, int index) {
  Browser* browser = chrome::FindBrowserWithID(session_id);
  // Sanity checks.
  if (!browser || index >= browser->tab_strip_model()->count())
    return GURL();

  if (index >= 0) {
    content::WebContents* target_tab =
        browser->tab_strip_model()->GetWebContentsAt(index);
    if (target_tab)
      return target_tab->GetURL();
  }

  return GURL();
}

Profile* GetFeedbackProfile(Browser* browser) {
  Profile* profile =
      browser ? browser->profile()
              : ProfileManager::GetLastUsedProfileAllowedByPolicy();
  if (!profile)
    return nullptr;

  // We do not want to launch on an OTR profile.
  profile = profile->GetOriginalProfile();
  DCHECK(profile);

#if defined(OS_CHROMEOS)
  // Obtains the display profile ID on which the Feedback window should show.
  chrome::MultiUserWindowManager* const window_manager =
      chrome::MultiUserWindowManager::GetInstance();
  const AccountId display_account_id =
      window_manager && browser
          ? window_manager->GetUserPresentingWindow(
                browser->window()->GetNativeWindow())
          : EmptyAccountId();
  if (display_account_id.is_valid())
    profile = multi_user_util::GetProfileFromAccountId(display_account_id);
#endif
  return profile;
}

}  // namespace chrome
