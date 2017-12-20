// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_ON_STARTUP_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_ON_STARTUP_HANDLER_H_

#include "base/macros.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"

namespace base {
class ListValue;
}

namespace settings {

class OnStartupHandler : public SettingsPageUIHandler {
 public:
  OnStartupHandler();
  ~OnStartupHandler() override;

  // SettingsPageUIHandler:
  void RegisterMessages() override;
  void OnJavascriptAllowed() override {}
  void OnJavascriptDisallowed() override {}

 private:
  // Handler for the "getNtpExtension" message. No arguments.
  void HandleGetNtpExtension(const base::ListValue* /*args*/);

  DISALLOW_COPY_AND_ASSIGN(OnStartupHandler);
};

}  // namespace settings

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_ON_STARTUP_HANDLER_H_
