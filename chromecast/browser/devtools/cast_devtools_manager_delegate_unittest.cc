// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/browser/devtools/cast_devtools_manager_delegate.h"

#include <memory>
#include <unordered_set>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_content_client_initializer.h"
#include "content/public/test/test_renderer_host.h"
#include "content/public/test/web_contents_tester.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromecast {
namespace shell {

class CastDevToolsManagerDelegateTest
    : public content::RenderViewHostTestHarness {
 public:
  using WebContentsSet = std::unordered_set<content::WebContents*>;

  CastDevToolsManagerDelegateTest() {}
  ~CastDevToolsManagerDelegateTest() override {}

  void SetUp() override {
    initializer_ = base::MakeUnique<content::TestContentClientInitializer>();
    content::RenderViewHostTestHarness::SetUp();
    devtools_manager_delegate_ =
        base::MakeUnique<CastDevToolsManagerDelegate>();
  }

  void TestDiscoveredTargets(const WebContentsSet& enabled_web_contents,
                             content::DevToolsAgentHost::List targets) {
    EXPECT_EQ(enabled_web_contents.size(), targets.size());

    for (const auto& target : targets) {
      EXPECT_TRUE(enabled_web_contents.find(target->GetWebContents()) !=
                  enabled_web_contents.end())
          << "Discovered target not found in enabled WebContents.";
    }
  }

 protected:
  std::unique_ptr<content::TestContentClientInitializer> initializer_;
  std::unique_ptr<CastDevToolsManagerDelegate> devtools_manager_delegate_;

 private:
  DISALLOW_COPY_AND_ASSIGN(CastDevToolsManagerDelegateTest);
};

// TODO(derekjchow): Make the tests pass on cast testbots.
// crbug.com/665118
TEST_F(CastDevToolsManagerDelegateTest, DISABLED_TestSingletonGetter) {
  EXPECT_EQ(devtools_manager_delegate_.get(),
            CastDevToolsManagerDelegate::GetInstance());
}

TEST_F(CastDevToolsManagerDelegateTest, DISABLED_NoWebContents) {
  EXPECT_TRUE(devtools_manager_delegate_->DiscoverTargets(
      base::Bind(&CastDevToolsManagerDelegateTest::TestDiscoveredTargets,
                 base::Unretained(this), WebContentsSet())));
}

TEST_F(CastDevToolsManagerDelegateTest, DISABLED_DisabledWebContents) {
  EXPECT_TRUE(devtools_manager_delegate_->DiscoverTargets(
      base::Bind(&CastDevToolsManagerDelegateTest::TestDiscoveredTargets,
                 base::Unretained(this), WebContentsSet())));
}

TEST_F(CastDevToolsManagerDelegateTest, DISABLED_EnabledWebContents) {
  devtools_manager_delegate_->EnableWebContentsForDebugging(web_contents());
  WebContentsSet enabled_web_contents({web_contents()});
  EXPECT_TRUE(devtools_manager_delegate_->DiscoverTargets(
      base::Bind(&CastDevToolsManagerDelegateTest::TestDiscoveredTargets,
                 base::Unretained(this), enabled_web_contents)));
}

}  // namespace shell
}  // namespace chromecast
