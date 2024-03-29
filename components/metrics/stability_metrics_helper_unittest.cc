// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/stability_metrics_helper.h"

#include "base/macros.h"
#include "base/test/histogram_tester.h"
#include "components/metrics/proto/system_profile.pb.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/prefs/testing_pref_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace metrics {

namespace {

enum RendererType {
  RENDERER_TYPE_RENDERER = 1,
  RENDERER_TYPE_EXTENSION,
  // NOTE: Add new action types only immediately above this line. Also,
  // make sure the enum list in tools/metrics/histograms/histograms.xml is
  // updated with any change in here.
  RENDERER_TYPE_COUNT
};

class StabilityMetricsHelperTest : public testing::Test {
 protected:
  StabilityMetricsHelperTest() : prefs_(new TestingPrefServiceSimple) {
    StabilityMetricsHelper::RegisterPrefs(prefs()->registry());
  }

  TestingPrefServiceSimple* prefs() { return prefs_.get(); }

 private:
  std::unique_ptr<TestingPrefServiceSimple> prefs_;

  DISALLOW_COPY_AND_ASSIGN(StabilityMetricsHelperTest);
};

}  // namespace

TEST_F(StabilityMetricsHelperTest, BrowserChildProcessCrashed) {
  StabilityMetricsHelper helper(prefs());

  helper.BrowserChildProcessCrashed();
  helper.BrowserChildProcessCrashed();

  // Call ProvideStabilityMetrics to check that it will force pending tasks to
  // be executed immediately.
  metrics::SystemProfileProto system_profile;

  helper.ProvideStabilityMetrics(&system_profile);

  // Check current number of instances created.
  const metrics::SystemProfileProto_Stability& stability =
      system_profile.stability();

  EXPECT_EQ(2, stability.child_process_crash_count());
}

TEST_F(StabilityMetricsHelperTest, LogRendererCrash) {
  StabilityMetricsHelper helper(prefs());
  base::HistogramTester histogram_tester;

  // Crash and abnormal termination should increment renderer crash count.
  helper.LogRendererCrash(false, base::TERMINATION_STATUS_PROCESS_CRASHED, 1);

  helper.LogRendererCrash(false, base::TERMINATION_STATUS_ABNORMAL_TERMINATION,
                          1);

  // OOM should increment renderer crash count.
  helper.LogRendererCrash(false, base::TERMINATION_STATUS_OOM, 1);

  // Kill does not increment renderer crash count.
  helper.LogRendererCrash(false, base::TERMINATION_STATUS_PROCESS_WAS_KILLED,
                          1);

  // Failed launch increments failed launch count.
  helper.LogRendererCrash(false, base::TERMINATION_STATUS_LAUNCH_FAILED, 1);

  metrics::SystemProfileProto system_profile;

  // Call ProvideStabilityMetrics to check that it will force pending tasks to
  // be executed immediately.
  helper.ProvideStabilityMetrics(&system_profile);

  EXPECT_EQ(3, system_profile.stability().renderer_crash_count());
  EXPECT_EQ(1, system_profile.stability().renderer_failed_launch_count());
  EXPECT_EQ(0, system_profile.stability().extension_renderer_crash_count());

  helper.ClearSavedStabilityMetrics();

  // Crash and abnormal termination should increment extension crash count.
  helper.LogRendererCrash(true, base::TERMINATION_STATUS_PROCESS_CRASHED, 1);

  // OOM should increment extension renderer crash count.
  helper.LogRendererCrash(true, base::TERMINATION_STATUS_OOM, 1);

  // Failed launch increments extension failed launch count.
  helper.LogRendererCrash(true, base::TERMINATION_STATUS_LAUNCH_FAILED, 1);

  system_profile.Clear();
  helper.ProvideStabilityMetrics(&system_profile);

  EXPECT_EQ(0, system_profile.stability().renderer_crash_count());
  EXPECT_EQ(2, system_profile.stability().extension_renderer_crash_count());
  EXPECT_EQ(
      1, system_profile.stability().extension_renderer_failed_launch_count());

  // TERMINATION_STATUS_PROCESS_CRASHED, TERMINATION_STATUS_ABNORMAL_TERMINATION
  // and TERMINATION_STATUS_OOM = 3.
  histogram_tester.ExpectUniqueSample("CrashExitCodes.Renderer", 1, 3);
  histogram_tester.ExpectBucketCount("BrowserRenderProcessHost.ChildCrashes",
                                     RENDERER_TYPE_RENDERER, 3);

  // TERMINATION_STATUS_PROCESS_CRASHED and TERMINATION_STATUS_OOM = 2.
  histogram_tester.ExpectUniqueSample("CrashExitCodes.Extension", 1, 2);
  histogram_tester.ExpectBucketCount("BrowserRenderProcessHost.ChildCrashes",
                                     RENDERER_TYPE_EXTENSION, 2);

  // One launch failure each.
  histogram_tester.ExpectBucketCount(
      "BrowserRenderProcessHost.ChildLaunchFailures", RENDERER_TYPE_RENDERER,
      1);
  histogram_tester.ExpectBucketCount(
      "BrowserRenderProcessHost.ChildLaunchFailures", RENDERER_TYPE_EXTENSION,
      1);
  histogram_tester.ExpectBucketCount(
      "BrowserRenderProcessHost.ChildLaunchFailureCodes", 1, 2);

  // TERMINATION_STATUS_PROCESS_WAS_KILLED for a renderer.
  histogram_tester.ExpectBucketCount("BrowserRenderProcessHost.ChildKills",
                                     RENDERER_TYPE_RENDERER, 1);
  histogram_tester.ExpectBucketCount("BrowserRenderProcessHost.ChildKills",
                                     RENDERER_TYPE_EXTENSION, 0);
}

}  // namespace metrics
