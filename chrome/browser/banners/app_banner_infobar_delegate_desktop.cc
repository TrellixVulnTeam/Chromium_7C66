// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/banners/app_banner_infobar_delegate_desktop.h"

#include <memory>

#include "base/bind.h"
#include "chrome/browser/banners/app_banner_manager.h"
#include "chrome/browser/banners/app_banner_metrics.h"
#include "chrome/browser/banners/app_banner_settings_helper.h"
#include "chrome/browser/extensions/bookmark_app_helper.h"
#include "chrome/browser/infobars/infobar_service.h"
#include "chrome/common/web_application_info.h"
#include "chrome/grit/generated_resources.h"
#include "components/infobars/core/infobar.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/vector_icons_public.h"

namespace banners {

// static
infobars::InfoBar* AppBannerInfoBarDelegateDesktop::Create(
    content::WebContents* web_contents,
    base::WeakPtr<AppBannerManager> weak_manager,
    extensions::BookmarkAppHelper* bookmark_app_helper,
    const content::Manifest& manifest,
    int event_request_id) {
  InfoBarService* infobar_service =
      InfoBarService::FromWebContents(web_contents);
  return infobar_service->AddInfoBar(infobar_service->CreateConfirmInfoBar(
      std::unique_ptr<ConfirmInfoBarDelegate>(
          new AppBannerInfoBarDelegateDesktop(weak_manager, bookmark_app_helper,
                                              manifest, event_request_id))));
}

AppBannerInfoBarDelegateDesktop::AppBannerInfoBarDelegateDesktop(
    base::WeakPtr<AppBannerManager> weak_manager,
    extensions::BookmarkAppHelper* bookmark_app_helper,
    const content::Manifest& manifest,
    int event_request_id)
    : ConfirmInfoBarDelegate(),
      weak_manager_(weak_manager),
      bookmark_app_helper_(bookmark_app_helper),
      manifest_(manifest),
      event_request_id_(event_request_id),
      has_user_interaction_(false) {
}

AppBannerInfoBarDelegateDesktop::~AppBannerInfoBarDelegateDesktop() {
  if (!has_user_interaction_)
    TrackUserResponse(USER_RESPONSE_WEB_APP_IGNORED);
}

infobars::InfoBarDelegate::Type
AppBannerInfoBarDelegateDesktop::GetInfoBarType() const {
  return PAGE_ACTION_TYPE;
}

infobars::InfoBarDelegate::InfoBarIdentifier
AppBannerInfoBarDelegateDesktop::GetIdentifier() const {
  return APP_BANNER_INFOBAR_DELEGATE_DESKTOP;
}

gfx::VectorIconId AppBannerInfoBarDelegateDesktop::GetVectorIconId() const {
  return gfx::VectorIconId::APPS;
}

void AppBannerInfoBarDelegateDesktop::InfoBarDismissed() {
  TrackUserResponse(USER_RESPONSE_WEB_APP_DISMISSED);
  has_user_interaction_ = true;

  content::WebContents* web_contents =
      InfoBarService::WebContentsFromInfoBar(infobar());
  if (web_contents) {
    if (weak_manager_)
      weak_manager_->SendBannerDismissed(event_request_id_);

    AppBannerSettingsHelper::RecordBannerDismissEvent(
        web_contents, manifest_.start_url.spec(), AppBannerSettingsHelper::WEB);
  }
}

base::string16 AppBannerInfoBarDelegateDesktop::GetMessageText() const {
  return l10n_util::GetStringUTF16(IDS_ADD_TO_SHELF_INFOBAR_TITLE);
}

int AppBannerInfoBarDelegateDesktop::GetButtons() const {
  return BUTTON_OK;
}

base::string16 AppBannerInfoBarDelegateDesktop::GetButtonLabel(
    InfoBarButton button) const {
  return l10n_util::GetStringUTF16(IDS_ADD_TO_SHELF_INFOBAR_ADD_BUTTON);
}

bool AppBannerInfoBarDelegateDesktop::Accept() {
  TrackUserResponse(USER_RESPONSE_WEB_APP_ACCEPTED);
  has_user_interaction_ = true;

  bookmark_app_helper_->CreateFromAppBanner(
      base::Bind(&AppBannerManager::DidFinishCreatingBookmarkApp,
                 weak_manager_),
      manifest_);
  return true;
}

}  // namespace banners
