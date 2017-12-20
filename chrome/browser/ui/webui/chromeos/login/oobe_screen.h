// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_OOBE_SCREEN_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_OOBE_SCREEN_H_

#include <string>

namespace chromeos {

// Different screens in the Oobe. If you update this enum, *make sure* to
// update kScreenNames in the cc file as well.
enum class OobeScreen : unsigned int {
  SCREEN_OOBE_HID_DETECTION = 0,
  SCREEN_OOBE_NETWORK,
  SCREEN_OOBE_EULA,
  SCREEN_OOBE_UPDATE,
  SCREEN_OOBE_ENABLE_DEBUGGING,
  SCREEN_OOBE_ENROLLMENT,
  SCREEN_OOBE_RESET,
  SCREEN_GAIA_SIGNIN,
  SCREEN_ACCOUNT_PICKER,
  SCREEN_KIOSK_AUTOLAUNCH,
  SCREEN_KIOSK_ENABLE,
  SCREEN_ERROR_MESSAGE,
  SCREEN_USER_IMAGE_PICKER,
  SCREEN_TPM_ERROR,
  SCREEN_PASSWORD_CHANGED,
  SCREEN_CREATE_SUPERVISED_USER_FLOW,
  SCREEN_TERMS_OF_SERVICE,
  SCREEN_ARC_TERMS_OF_SERVICE,
  SCREEN_WRONG_HWID,
  SCREEN_AUTO_ENROLLMENT_CHECK,
  SCREEN_APP_LAUNCH_SPLASH,
  SCREEN_CONFIRM_PASSWORD,
  SCREEN_FATAL_ERROR,
  SCREEN_OOBE_CONTROLLER_PAIRING,
  SCREEN_OOBE_HOST_PAIRING,
  SCREEN_DEVICE_DISABLED,
  SCREEN_UNRECOVERABLE_CRYPTOHOME_ERROR,
  SCREEN_UNKNOWN  // This must always be the last element.
};

// Returns the JS name for the given screen.
std::string GetOobeScreenName(OobeScreen screen);

// Converts the JS name for the given sreen into a Screen instance.
OobeScreen GetOobeScreenFromName(const std::string& name);

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_OOBE_SCREEN_H_
