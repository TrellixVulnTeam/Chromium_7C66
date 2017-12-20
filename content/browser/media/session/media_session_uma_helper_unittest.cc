// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/media/session/media_session_uma_helper.h"

#include "base/metrics/histogram_samples.h"
#include "base/test/histogram_tester.h"
#include "base/test/simple_test_tick_clock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

using MediaSessionSuspendedSource =
    MediaSessionUmaHelper::MediaSessionSuspendedSource;

namespace {

class MediaSessionUmaHelperTest : public testing::Test {
 public:
  MediaSessionUmaHelperTest() = default;

  void SetUp() override {
    clock_ = new base::SimpleTestTickClock();
    clock_->SetNowTicks(base::TimeTicks::Now());
    media_session_uma_helper_.SetClockForTest(
        std::unique_ptr<base::SimpleTestTickClock>(clock_));
  }

  void TearDown() override {
    clock_ = nullptr;
  }

  base::SimpleTestTickClock* clock() { return clock_; }

  MediaSessionUmaHelper& media_session_uma_helper() {
    return media_session_uma_helper_;
  };

  std::unique_ptr<base::HistogramSamples> GetHistogramSamplesSinceTestStart(
      const std::string& name) {
    return histogram_tester_.GetHistogramSamplesSinceCreation(name);
  }

 private:
  base::SimpleTestTickClock* clock_ = nullptr;
  MediaSessionUmaHelper media_session_uma_helper_;
  base::HistogramTester histogram_tester_;
};

}  // anonymous namespace

TEST_F(MediaSessionUmaHelperTest, CreateAndKillDoesNothing) {
  {
    MediaSessionUmaHelper* uma_helper = new MediaSessionUmaHelper();
    delete uma_helper;
  }

  {
    std::unique_ptr<base::HistogramSamples> samples(
        GetHistogramSamplesSinceTestStart("Media.Session.Suspended"));
    EXPECT_EQ(0, samples->TotalCount());
  }

  {
    std::unique_ptr<base::HistogramSamples> samples(
        GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
    EXPECT_EQ(0, samples->TotalCount());
  }
}

TEST_F(MediaSessionUmaHelperTest, SuspendRegisterImmediately) {
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemTransient);

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.Suspended"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(0)); // System Transient
  EXPECT_EQ(0, samples->GetCount(1)); // System Permanent
  EXPECT_EQ(0, samples->GetCount(2)); // UI
}

TEST_F(MediaSessionUmaHelperTest, MultipleSuspend) {
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemTransient);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemPermanent);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::UI);

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.Suspended"));
  EXPECT_EQ(3, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(0)); // System Transient
  EXPECT_EQ(1, samples->GetCount(1)); // System Permanent
  EXPECT_EQ(1, samples->GetCount(2)); // UI
}

TEST_F(MediaSessionUmaHelperTest, MultipleSuspendSame) {
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemPermanent);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemTransient);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::UI);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemTransient);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::SystemPermanent);
  media_session_uma_helper().RecordSessionSuspended(
      MediaSessionSuspendedSource::UI);

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.Suspended"));
  EXPECT_EQ(6, samples->TotalCount());
  EXPECT_EQ(2, samples->GetCount(0)); // System Transient
  EXPECT_EQ(2, samples->GetCount(1)); // System Permanent
  EXPECT_EQ(2, samples->GetCount(2)); // UI
}

TEST_F(MediaSessionUmaHelperTest, ActivationNotTerminatedDoesNotCommit) {
  media_session_uma_helper().OnSessionActive();
  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(0, samples->TotalCount());
}

TEST_F(MediaSessionUmaHelperTest, SuspendActivationNotTerminatedDoesNotCommit) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(0, samples->TotalCount());
}

TEST_F(MediaSessionUmaHelperTest, FullActivation) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionInactive();

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(1000));
}

TEST_F(MediaSessionUmaHelperTest, ActivationCycleWithSuspend) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionInactive();

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(2000));
}

TEST_F(MediaSessionUmaHelperTest, ActivationCycleWithMultipleSuspend) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionInactive();

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(3000));
}

TEST_F(MediaSessionUmaHelperTest, MultipleActivations) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionInactive();

  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionInactive();

  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(2, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(2000));
  EXPECT_EQ(1, samples->GetCount(3000));
}

TEST_F(MediaSessionUmaHelperTest, MultipleActivationCalls) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(500));
  media_session_uma_helper().OnSessionInactive();

  // Calling OnSessionActive() multiple times reset the start time of the
  // session.
  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(500));
}

TEST_F(MediaSessionUmaHelperTest, MultipleSuspendCalls_WhileSuspended) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(500));
  media_session_uma_helper().OnSessionSuspended();

  media_session_uma_helper().OnSessionInactive();

  // If the session is already suspended, OnSessionSuspended() calls are
  // ignored.
  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(1000));
}

TEST_F(MediaSessionUmaHelperTest, MultipleSuspendCalls_WhileInactive) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(1000));
  media_session_uma_helper().OnSessionInactive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionSuspended();

  clock()->Advance(base::TimeDelta::FromMilliseconds(500));
  media_session_uma_helper().OnSessionSuspended();

  media_session_uma_helper().OnSessionInactive();

  // If the session is already inactive, OnSessionSuspended() calls are
  // ignored.
  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(1000));
}

TEST_F(MediaSessionUmaHelperTest, MultipleInactiveCalls) {
  media_session_uma_helper().OnSessionActive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(3000));
  media_session_uma_helper().OnSessionInactive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(2000));
  media_session_uma_helper().OnSessionInactive();

  clock()->Advance(base::TimeDelta::FromMilliseconds(500));
  media_session_uma_helper().OnSessionInactive();

  // If the session is already inactive, OnSessionInactive() calls are ignored.
  std::unique_ptr<base::HistogramSamples> samples(
      GetHistogramSamplesSinceTestStart("Media.Session.ActiveTime"));
  EXPECT_EQ(1, samples->TotalCount());
  EXPECT_EQ(1, samples->GetCount(3000));
}

}  // namespace content
