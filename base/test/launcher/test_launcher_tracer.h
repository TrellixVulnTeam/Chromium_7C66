// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_LAUNCHER_TEST_LAUNCHER_TRACER_H_
#define BASE_TEST_LAUNCHER_TEST_LAUNCHER_TRACER_H_

#include <string>
#include <vector>

#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

namespace base {

class FilePath;

// Records traces of test execution, e.g. to analyze performance.
// Thread safe.
class TestLauncherTracer {
 public:
  TestLauncherTracer();
  ~TestLauncherTracer();

  // Records an event corresponding to test process execution.
  void RecordProcessExecution(TimeTicks start_time, TimeDelta duration);

  // Dumps trace data as JSON. Returns true on success.
  bool Dump(const FilePath& path) WARN_UNUSED_RESULT;

 private:
  // Simplified version of base::TraceEvent.
  struct Event {
    std::string name;            // Displayed name.
    TimeTicks timestamp;         // Timestamp when this event began.
    TimeDelta duration;          // How long was this event.
    PlatformThreadId thread_id;  // Thread ID where event was reported.
  };

  // Timestamp when tracing started.
  TimeTicks trace_start_time_;

  // Log of trace events.
  std::vector<Event> events_;

  // Lock to protect all member variables.
  Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(TestLauncherTracer);
};

}  // namespace base

#endif  // BASE_TEST_LAUNCHER_TEST_LAUNCHER_TRACER_H_
