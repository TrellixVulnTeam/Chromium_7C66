// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/sync/sync_promo_ui.h"

#include <memory>

#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "chrome/browser/signin/fake_signin_manager_builder.h"
#include "chrome/browser/signin/signin_manager_factory.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/testing_profile.h"
#include "components/browser_sync/browser_sync_switches.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"

class SyncPromoUITest : public testing::Test {
 public:
  SyncPromoUITest() {}

  // testing::Test:
  void SetUp() override {
    testing::Test::SetUp();
    TestingProfile::Builder builder;
    builder.AddTestingFactory(SigninManagerFactory::GetInstance(),
                              BuildFakeSigninManagerBase);
    profile_ = builder.Build();
  }

 protected:
  void DisableSync() {
    base::CommandLine::ForCurrentProcess()->AppendSwitch(
        switches::kDisableSync);
  }

  content::TestBrowserThreadBundle thread_bundle_;
  std::unique_ptr<TestingProfile> profile_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SyncPromoUITest);
};

// Verifies that ShouldShowSyncPromo returns false if sync is disabled by
// policy.
TEST_F(SyncPromoUITest, ShouldShowSyncPromoSyncDisabled) {
  DisableSync();
  EXPECT_FALSE(SyncPromoUI::ShouldShowSyncPromo(profile_.get()));
}

// Verifies that ShouldShowSyncPromo returns true if all conditions to
// show the promo are met.
TEST_F(SyncPromoUITest, ShouldShowSyncPromoSyncEnabled) {
#if defined(OS_CHROMEOS)
  // No sync promo on CrOS.
  EXPECT_FALSE(SyncPromoUI::ShouldShowSyncPromo(profile_.get()));
#else
  EXPECT_TRUE(SyncPromoUI::ShouldShowSyncPromo(profile_.get()));
#endif
}
