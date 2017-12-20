// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "chrome/browser/chromeos/device/input_service_test_helper.h"
#include "chrome/browser/chromeos/login/screens/base_screen.h"
#include "chrome/browser/chromeos/login/screens/hid_detection_screen.h"
#include "chrome/browser/chromeos/login/test/wizard_in_process_browser_test.h"
#include "chrome/browser/chromeos/login/wizard_controller.h"

namespace chromeos {

class HIDDetectionScreenTest : public WizardInProcessBrowserTest {
 public:
  HIDDetectionScreenTest()
      : WizardInProcessBrowserTest(WizardController::kHIDDetectionScreenName) {}

 protected:
  void SetUpOnMainThread() override {
    helper_.reset(new InputServiceTestHelper);
    WizardInProcessBrowserTest::SetUpOnMainThread();
    ASSERT_TRUE(WizardController::default_controller());

    hid_detection_screen_ = static_cast<HIDDetectionScreen*>(
        WizardController::default_controller()->GetScreen(
            WizardController::kHIDDetectionScreenName));
    ASSERT_TRUE(hid_detection_screen_);
    ASSERT_EQ(WizardController::default_controller()->current_screen(),
              hid_detection_screen_);
    ASSERT_TRUE(hid_detection_screen_->view_);

    helper_->SetProxy(&hid_detection_screen_->input_service_proxy_);
  }

  void TearDownOnMainThread() override {
    helper_->ClearProxy();
    WizardInProcessBrowserTest::TearDownOnMainThread();
  }

  InputServiceTestHelper* helper() { return helper_.get(); }

  const ::login::ScreenContext* context() const {
    return &hid_detection_screen_->context_;
  }

 private:
  HIDDetectionScreen* hid_detection_screen_;
  std::unique_ptr<InputServiceTestHelper> helper_;

  DISALLOW_COPY_AND_ASSIGN(HIDDetectionScreenTest);
};

IN_PROC_BROWSER_TEST_F(HIDDetectionScreenTest, MouseKeyboardStates) {
  // NOTE: State strings match those in hid_detection_screen.cc.
  // No devices added yet
  EXPECT_EQ("searching",
            context()->GetString(HIDDetectionModel::kContextKeyMouseState));
  EXPECT_EQ("searching",
            context()->GetString(HIDDetectionModel::kContextKeyKeyboardState));
  EXPECT_FALSE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));

  // Generic connection types. Unlike the pointing device, which may be a tablet
  // or touchscreen, the keyboard only reports usb and bluetooth states.
  helper()->AddDeviceToService(true, InputDeviceInfo::TYPE_SERIO);
  EXPECT_TRUE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));

  helper()->AddDeviceToService(false, InputDeviceInfo::TYPE_SERIO);
  EXPECT_EQ("connected",
            context()->GetString(HIDDetectionModel::kContextKeyMouseState));
  EXPECT_EQ("usb",
            context()->GetString(HIDDetectionModel::kContextKeyKeyboardState));
  EXPECT_TRUE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));

  // Remove generic devices, add usb devices.
  helper()->RemoveDeviceFromService(true);
  helper()->RemoveDeviceFromService(false);
  EXPECT_FALSE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));

  helper()->AddDeviceToService(true, InputDeviceInfo::TYPE_USB);
  helper()->AddDeviceToService(false, InputDeviceInfo::TYPE_USB);
  EXPECT_EQ("usb",
            context()->GetString(HIDDetectionModel::kContextKeyMouseState));
  EXPECT_EQ("usb",
            context()->GetString(HIDDetectionModel::kContextKeyKeyboardState));
  EXPECT_TRUE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));

  // Remove usb devices, add bluetooth devices.
  helper()->RemoveDeviceFromService(true);
  helper()->RemoveDeviceFromService(false);
  EXPECT_FALSE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));

  helper()->AddDeviceToService(true, InputDeviceInfo::TYPE_BLUETOOTH);
  helper()->AddDeviceToService(false, InputDeviceInfo::TYPE_BLUETOOTH);
  EXPECT_EQ("paired",
            context()->GetString(HIDDetectionModel::kContextKeyMouseState));
  EXPECT_EQ("paired",
            context()->GetString(HIDDetectionModel::kContextKeyKeyboardState));
  EXPECT_TRUE(context()->GetBoolean(
      HIDDetectionModel::kContextKeyContinueButtonEnabled));
}

}  // namespace chromeos
