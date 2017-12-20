// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_TEST_APP_WINDOW_WAITER_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_TEST_APP_WINDOW_WAITER_H_

#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/run_loop.h"
#include "extensions/browser/app_window/app_window_registry.h"

namespace extensions {
class AppWindow;
}

namespace chromeos {

// Helper class that monitors app windows to wait for a window to appear.
// Use a new instance for each use, one instance will only work for one Wait.
class AppWindowWaiter : public extensions::AppWindowRegistry::Observer {
 public:
  AppWindowWaiter(extensions::AppWindowRegistry* registry,
                  const std::string& app_id);
  ~AppWindowWaiter() override;

  // Waits for an AppWindow of the app to be added.
  extensions::AppWindow* Wait();

  // Waits for an AppWindow of the app to be shown.
  extensions::AppWindow* WaitForShown();

  // AppWindowRegistry::Observer:
  void OnAppWindowAdded(extensions::AppWindow* app_window) override;
  void OnAppWindowShown(extensions::AppWindow* app_window,
                        bool was_hidden) override;

 private:
  enum WaitType {
    WAIT_FOR_NONE,
    WAIT_FOR_ADDED,
    WAIT_FOR_SHOWN,
  };

  extensions::AppWindowRegistry* const registry_;
  const std::string app_id_;
  std::unique_ptr<base::RunLoop> run_loop_;
  WaitType wait_type_ = WAIT_FOR_NONE;
  extensions::AppWindow* window_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(AppWindowWaiter);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_TEST_APP_WINDOW_WAITER_H_
