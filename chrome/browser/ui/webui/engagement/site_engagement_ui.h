// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_ENGAGEMENT_SITE_ENGAGEMENT_UI_H_
#define CHROME_BROWSER_UI_WEBUI_ENGAGEMENT_SITE_ENGAGEMENT_UI_H_

#include "base/macros.h"
#include "chrome/browser/ui/webui/engagement/site_engagement.mojom.h"
#include "chrome/browser/ui/webui/mojo_web_ui_controller.h"

// The UI for chrome://site-engagement/.
class SiteEngagementUI
    : public MojoWebUIController<mojom::SiteEngagementUIHandler> {
 public:
  explicit SiteEngagementUI(content::WebUI* web_ui);
  ~SiteEngagementUI() override;

 private:
  // MojoWebUIController overrides:
  void BindUIHandler(
      mojo::InterfaceRequest<mojom::SiteEngagementUIHandler> request) override;

  std::unique_ptr<mojom::SiteEngagementUIHandler> ui_handler_;

  DISALLOW_COPY_AND_ASSIGN(SiteEngagementUI);
};

#endif  // CHROME_BROWSER_UI_WEBUI_ENGAGEMENT_SITE_ENGAGEMENT_UI_H_
