// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_RENDER_MEDIA_LOG_H_
#define CONTENT_RENDERER_MEDIA_RENDER_MEDIA_LOG_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"
#include "content/common/content_export.h"
#include "media/base/media_log.h"
#include "url/gurl.h"

namespace base {
class TickClock;
}

namespace content {

// RenderMediaLog is an implementation of MediaLog that forwards events to the
// browser process, throttling as necessary.
//
// It also caches the last error events to support renderer-side reporting to
// entities like HTMLMediaElement and devtools console.
//
// To minimize the number of events sent over the wire, only the latest event
// added is sent for high frequency events (e.g., BUFFERED_EXTENTS_CHANGED).
//
// It must be constructed on the render thread.
class CONTENT_EXPORT RenderMediaLog : public media::MediaLog {
 public:
  explicit RenderMediaLog(const GURL& security_origin);

  // MediaLog implementation.
  void AddEvent(std::unique_ptr<media::MediaLogEvent> event) override;
  std::string GetLastErrorMessage() override;
  void RecordRapporWithSecurityOrigin(const std::string& metric) override;

  // Will reset |last_ipc_send_time_| with the value of NowTicks().
  void SetTickClockForTesting(std::unique_ptr<base::TickClock> tick_clock);
  void SetTaskRunnerForTesting(
      const scoped_refptr<base::SingleThreadTaskRunner>& task_runner);

 private:
  ~RenderMediaLog() override;

  // Posted as a delayed task on |task_runner_| to throttle ipc message
  // frequency.
  void SendQueuedMediaEvents();

  // Security origin of the current frame.
  const GURL security_origin_;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  // |lock_| protects access to all of the following member variables.  It
  // allows any render process thread to AddEvent(), while preserving their
  // sequence for throttled send on |task_runner_| and coherent retrieval by
  // GetLastErrorMessage().
  mutable base::Lock lock_;
  std::unique_ptr<base::TickClock> tick_clock_;
  base::TimeTicks last_ipc_send_time_;
  std::vector<media::MediaLogEvent> queued_media_events_;

  // For enforcing max 1 pending send.
  bool ipc_send_pending_;

  // Limits the number buffered extents changed events we send over IPC to one.
  std::unique_ptr<media::MediaLogEvent> last_buffered_extents_changed_event_;

  // Holds a copy of the most recent MEDIA_ERROR_LOG_ENTRY, if any.
  std::unique_ptr<media::MediaLogEvent> last_media_error_log_entry_;

  // Holds a copy of the most recent PIPELINE_ERROR, if any.
  std::unique_ptr<media::MediaLogEvent> last_pipeline_error_;

  DISALLOW_COPY_AND_ASSIGN(RenderMediaLog);
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_RENDER_MEDIA_LOG_H_
