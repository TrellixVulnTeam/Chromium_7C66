// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TRACING_CORE_TRACE_BUFFER_WRITER_H_
#define COMPONENTS_TRACING_CORE_TRACE_BUFFER_WRITER_H_

#include "base/macros.h"
#include "components/tracing/core/proto_zero_message.h"
#include "components/tracing/core/proto_zero_message_handle.h"
#include "components/tracing/core/scattered_stream_writer.h"
#include "components/tracing/core/trace_ring_buffer.h"
#include "components/tracing/tracing_export.h"

// Fwd declaration from the generated components/tracing/proto/event.pbzero.h.
namespace pbzero {
namespace tracing {
namespace proto {
class Event;
}  // namespace proto
}  // namespace tracing
}  // namespace pbzero

namespace tracing {
namespace v2 {

using TraceEventHandle = ProtoZeroMessageHandle<pbzero::tracing::proto::Event>;

// This class is the entry-point to add events to the TraceRingBuffer.
// It acts as a glue layer between the protobuf classes (ProtoZeroMessage) and
// the chunked trace ring buffer (TraceRingBuffer). This class is deliberately
// NOT thread safe. The expected design is that each thread owns an instance of
// TraceBufferWriter and that trace events produced on one thread are not passed
// to other threads.
class TRACING_EXPORT TraceBufferWriter
    : public ScatteredStreamWriter::Delegate {
 public:
  // |trace_ring_buffer| is the underlying ring buffer for taking and returning
  // chunks. |writer_id| is an identifier, unique for each instance, which is
  // appended to the header of each chunk. Its purpose is to allow the importer
  // to reconstruct the logical sequence of chunks for a given writer. Think to
  // this as a thread-id (just, in rare cases, some threads can have more than
  // one writer).
  TraceBufferWriter(TraceRingBuffer* trace_ring_buffer, uint32_t writer_id);
  ~TraceBufferWriter() override;

  // Adds a new event and returns a handle to it. The new event is valid (can be
  // populated) until the next call to AddEvent(). The new event is finalized
  // and fully committed to the ring buffer on the next call to AddEvent() or
  // when the returned handle goes out of scope, whichever comes first.
  TraceEventHandle AddEvent();

  // ScatteredStreamWriter::Delegate implementation.
  // Called by the ProtoZeroMessage's ScatteredStreamWriter when the caller
  // tries to append a new field and the write overflows the current chunk.
  ContiguousMemoryRange GetNewBuffer() override;

  // Finalize the pending event (if any) and returns back all chunks to the
  // |trace_ring_buffer_|.
  void Flush();

  const ScatteredStreamWriter& stream_writer() const { return stream_writer_; }

  uint32_t writer_id() const { return writer_id_; }

 private:
  void FinalizeCurrentEvent();
  void FinalizeCurrentChunk(bool is_fragmenting_event);
  ContiguousMemoryRange AcquireNewChunk(bool event_continues_from_prev_chunk);
  uint8_t* WriteEventPreambleForNewChunk(uint8_t* begin);

  TraceRingBuffer* trace_ring_buffer_;

  // Unique id of this writer (see comment in the ctor).
  const uint32_t writer_id_;

  // Monotonic counter (within the scope of writer_id_) of chunks.
  uint32_t chunk_seq_id_;

  // The last chunk acquired from the ring buffer. nullptr before the first call
  // to AddEvent(). Each instance can own more than one chunk at any given time
  // (to deal with messages larger than a chunk). This is being tracked through
  // the singly linked list Chunk::next_in_owner_list().
  TraceRingBuffer::Chunk* chunk_;

  // Used to work out how many bytes for the current |event_| lie in the current
  // |chunk_|.
  uint8_t* event_data_start_in_current_chunk_;

  ScatteredStreamWriter stream_writer_;

  // This field should be a proto::Event. However, ProtoZeroMessage subclasses
  // are stateless by design. This avoids to pull the full tree of autogenerated
  // headers for the stub classes and reduce build time.
  ProtoZeroMessage event_;

  DISALLOW_COPY_AND_ASSIGN(TraceBufferWriter);
};

}  // namespace v2
}  // namespace tracing

#endif  // COMPONENTS_TRACING_CORE_TRACE_BUFFER_WRITER_H_
