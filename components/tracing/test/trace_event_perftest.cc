// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted_memory.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/trace_event_argument.h"
#include "perf_test_helpers.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace tracing {
namespace {

using base::Bind;
using base::Thread;
using base::Unretained;
using base::WaitableEvent;
using base::trace_event::TraceConfig;
using base::trace_event::TraceLog;
using base::trace_event::TraceRecordMode;
using base::trace_event::TracedValue;

const int kNumRuns = 10;

class TraceEventPerfTest : public ::testing::Test {
 public:
  void BeginTrace() {
    TraceConfig config("*", "");
    config.SetTraceRecordMode(TraceRecordMode::RECORD_CONTINUOUSLY);
    TraceLog::GetInstance()->SetEnabled(config, TraceLog::RECORDING_MODE);
  }

  void EndTraceAndFlush() {
    ScopedStopwatch stopwatch("flush");
    base::RunLoop run_loop;
    TraceLog::GetInstance()->SetDisabled();
    TraceLog::GetInstance()->Flush(
        Bind(&OnTraceDataCollected, run_loop.QuitClosure()));
    run_loop.Run();
  }

  static void OnTraceDataCollected(
      base::Closure quit_closure,
      const scoped_refptr<base::RefCountedString>& events_str,
      bool has_more_events) {

    if (!has_more_events)
      quit_closure.Run();
  }

  std::unique_ptr<TracedValue> MakeTracedValue(int counter) {
    auto value = base::MakeUnique<TracedValue>();
    value->SetInteger("counter", counter);
    value->BeginDictionary("test_dict");
    value->BeginArray("nodes");
    for (int i = 0; i < 10; i++) {
      value->BeginDictionary();
      value->SetInteger("id", i);
      value->SetBoolean("valid", true);
      value->SetString("value", "foo");
      value->EndDictionary();
    }
    value->EndArray();
    value->SetInteger("count", 10);
    value->EndDictionary();
    return value;
  }

  static void SubmitTraceEventsAndSignal(WaitableEvent* complete_event) {
    for (int i = 0; i < 10000; i++) {
      TRACE_EVENT0("test_category", "some call");
    }
    complete_event->Signal();
  }

 private:
  base::MessageLoop _message_loop;
};

TEST_F(TraceEventPerfTest, Submit_10000_TRACE_EVENT0) {
  BeginTrace();
  IterableStopwatch stopwatch("events");
  for (int lap = 0; lap < kNumRuns; lap++) {
    for (int i = 0; i < 10000; i++) {
      TRACE_EVENT0("test_category", "TRACE_EVENT0 call");
    }
    stopwatch.NextLap();
  }
  EndTraceAndFlush();
}

TEST_F(TraceEventPerfTest, Long_TRACE_EVENT0) {
  BeginTrace();
  IterableStopwatch stopwatch("long_event");
  for (int lap = 0; lap < kNumRuns; lap++) {
    TRACE_EVENT0("test_category", "Outer event");
    for (int i = 0; i < 10000; i++) {
      TRACE_EVENT0("test_category", "TRACE_EVENT0 call");
    }
    stopwatch.NextLap();
  }
  EndTraceAndFlush();
}

TEST_F(TraceEventPerfTest, Create_10000_TracedValue) {
  std::unique_ptr<TracedValue> value;
  {
    ScopedStopwatch value_sw("create_traced_values");
    for (int i = 0; i < 10000; i++) {
      value = MakeTracedValue(i);
    }
  }
}

TEST_F(TraceEventPerfTest, Submit_10000_TRACE_EVENT_with_TracedValue) {
  BeginTrace();
  // Time reported by this timer includes TracedValue creation as well.
  IterableStopwatch trace_sw("events_with_value");
  for (int lap = 0; lap < kNumRuns; lap++) {
    for (int i = 0; i < 10000; i++) {
      TRACE_EVENT_INSTANT1("test_category", "event_with_value",
          TRACE_EVENT_SCOPE_THREAD, "value", MakeTracedValue(i));
    }
    trace_sw.NextLap();
  }
  EndTraceAndFlush();
}

TEST_F(TraceEventPerfTest, Submit_10000_TRACE_EVENT0_multithreaded) {
  BeginTrace();
  const int kNumThreads = 4;

  std::vector<std::unique_ptr<Thread>> threads;
  std::vector<std::unique_ptr<WaitableEvent>> complete_events;

  for (int i = 0; i < kNumThreads; i++) {
    Thread* thread = new Thread(std::string("thread_%d") + std::to_string(i));
    WaitableEvent* complete_event =
        new WaitableEvent(WaitableEvent::ResetPolicy::AUTOMATIC,
                          WaitableEvent::InitialState::NOT_SIGNALED);
    threads.push_back(base::WrapUnique(thread));
    complete_events.push_back(base::WrapUnique(complete_event));
    thread->Start();
  }

  {
    ScopedStopwatch stopwatch("events_over_multiple_threads");
    for (int i = 0; i < kNumThreads; i++) {
      threads[i]->task_runner()->PostTask(
          FROM_HERE,
          base::Bind(&SubmitTraceEventsAndSignal, complete_events[i].get()));
    }
    for (int i = 0; i < kNumThreads; i++) {
      complete_events[i]->Wait();
    }
  }

  EndTraceAndFlush();
  for (int i = 0; i < kNumThreads; i++) {
    threads[i]->Stop();
  }
}

}  // namespace
}  // namespace tracing
