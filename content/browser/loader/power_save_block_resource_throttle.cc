// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/loader/power_save_block_resource_throttle.h"

#include "device/power_save_blocker/power_save_blocker.h"

namespace content {

namespace {

const int kPowerSaveBlockDelaySeconds = 30;

}  // namespace

PowerSaveBlockResourceThrottle::PowerSaveBlockResourceThrottle(
    const std::string& host,
    scoped_refptr<base::SequencedTaskRunner> ui_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> blocking_task_runner)
    : host_(host),
      ui_task_runner_(ui_task_runner),
      blocking_task_runner_(blocking_task_runner) {}

PowerSaveBlockResourceThrottle::~PowerSaveBlockResourceThrottle() {
}

void PowerSaveBlockResourceThrottle::WillStartRequest(bool* defer) {
  // Delay PowerSaveBlocker activation to dismiss small requests.
  timer_.Start(FROM_HERE,
               base::TimeDelta::FromSeconds(kPowerSaveBlockDelaySeconds),
               this,
               &PowerSaveBlockResourceThrottle::ActivatePowerSaveBlocker);
}

void PowerSaveBlockResourceThrottle::WillProcessResponse(bool* defer) {
  // Stop blocking power save after request finishes.
  power_save_blocker_.reset();
  timer_.Stop();
}

const char* PowerSaveBlockResourceThrottle::GetNameForLogging() const {
  return "PowerSaveBlockResourceThrottle";
}

void PowerSaveBlockResourceThrottle::ActivatePowerSaveBlocker() {
  power_save_blocker_.reset(new device::PowerSaveBlocker(
      device::PowerSaveBlocker::kPowerSaveBlockPreventAppSuspension,
      device::PowerSaveBlocker::kReasonOther, "Uploading data to " + host_,
      ui_task_runner_, blocking_task_runner_));
}

}  // namespace content
