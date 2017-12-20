// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SIGNIN_SIGNIN_ERROR_UI_H_
#define CHROME_BROWSER_UI_WEBUI_SIGNIN_SIGNIN_ERROR_UI_H_

#include "base/macros.h"
#include "ui/web_dialogs/web_dialog_ui.h"

class SigninErrorHandler;

namespace ui {
class WebUI;
}

class SigninErrorUI : public ui::WebDialogUI {
 public:
  explicit SigninErrorUI(content::WebUI* web_ui);
  // Used to inject a SigninErrorHandler in tests.
  SigninErrorUI(content::WebUI* web_ui, SigninErrorHandler* handler);
  ~SigninErrorUI() override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(SigninErrorUI);
};

#endif  // CHROME_BROWSER_UI_WEBUI_SIGNIN_SIGNIN_ERROR_UI_H_
