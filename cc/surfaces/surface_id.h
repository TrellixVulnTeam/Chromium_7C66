// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_SURFACES_SURFACE_ID_H_
#define CC_SURFACES_SURFACE_ID_H_

#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <string>

#include "base/format_macros.h"
#include "base/hash.h"
#include "base/strings/stringprintf.h"
#include "cc/surfaces/frame_sink_id.h"
#include "cc/surfaces/local_frame_id.h"

namespace cc {

class SurfaceId {
 public:
  constexpr SurfaceId() = default;

  constexpr SurfaceId(const SurfaceId& other) = default;

  // A SurfaceId consists of two components: FrameSinkId and LocalFrameId.
  // A |frame_sink_id| consists of two components; one is allocated by the
  // display compositor service and one is allocated by the client. The
  // |frame_sink_id| uniquely identifies a FrameSink (and frame source).
  // A |local_frame_id| is a sequentially allocated ID generated by the frame
  // source that uniquely identifies a sequential set of frames of the same size
  // and device scale factor.
  constexpr SurfaceId(const FrameSinkId& frame_sink_id,
                      const LocalFrameId& local_frame_id)
      : frame_sink_id_(frame_sink_id), local_frame_id_(local_frame_id) {}

  bool is_valid() const {
    return frame_sink_id_.is_valid() && local_frame_id_.is_valid();
  }

  size_t hash() const {
    return base::HashInts(static_cast<uint64_t>(frame_sink_id_.hash()),
                          static_cast<uint64_t>(local_frame_id_.hash()));
  }

  const FrameSinkId& frame_sink_id() const { return frame_sink_id_; }

  const LocalFrameId& local_frame_id() const { return local_frame_id_; }

  std::string ToString() const {
    return base::StringPrintf("SurfaceId(%s, %s)",
                              frame_sink_id_.ToString().c_str(),
                              local_frame_id_.ToString().c_str());
  }

  bool operator==(const SurfaceId& other) const {
    return frame_sink_id_ == other.frame_sink_id_ &&
           local_frame_id_ == other.local_frame_id_;
  }

  bool operator!=(const SurfaceId& other) const { return !(*this == other); }

  bool operator<(const SurfaceId& other) const {
    return std::tie(frame_sink_id_, local_frame_id_) <
           std::tie(other.frame_sink_id_, other.local_frame_id_);
  }

 private:
  // See SurfaceIdAllocator::GenerateId.
  FrameSinkId frame_sink_id_;
  LocalFrameId local_frame_id_;
};

struct SurfaceIdHash {
  size_t operator()(const SurfaceId& key) const { return key.hash(); }
};

}  // namespace cc

#endif  // CC_SURFACES_SURFACE_ID_H_
