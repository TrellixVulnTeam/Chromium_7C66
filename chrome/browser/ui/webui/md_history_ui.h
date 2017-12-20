// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_MD_HISTORY_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MD_HISTORY_UI_H_

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "content/public/browser/web_ui_controller.h"
#include "ui/base/layout.h"

class Profile;

namespace base {
class ListValue;
class RefCountedMemory;
}


namespace user_prefs {
class PrefRegistrySyncable;
}

class MdHistoryUI : public content::WebUIController {
 public:
  explicit MdHistoryUI(content::WebUI* web_ui);
  ~MdHistoryUI() override;

  static bool IsEnabled(Profile* profile);

  // Reset the current list of features and explicitly set MD History enabled.
  static void SetEnabledForTesting(bool enabled);

  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  static base::RefCountedMemory* GetFaviconResourceBytes(
      ui::ScaleFactor scale_factor);

 private:
  FRIEND_TEST_ALL_PREFIXES(ContinueWhereILeftOffTest, MDHistoryUpgrade);

  static bool use_test_title_;

  void CreateDataSource();

  // Handler for the "menuPromoShown" message from the page. No arguments.
  void HandleMenuPromoShown(const base::ListValue* args);

  DISALLOW_COPY_AND_ASSIGN(MdHistoryUI);
};

#endif  // CHROME_BROWSER_UI_WEBUI_MD_HISTORY_UI_H_
