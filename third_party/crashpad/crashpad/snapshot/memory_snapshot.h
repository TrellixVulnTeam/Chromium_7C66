// Copyright 2014 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASHPAD_SNAPSHOT_MEMORY_SNAPSHOT_H_
#define CRASHPAD_SNAPSHOT_MEMORY_SNAPSHOT_H_

#include <stdint.h>
#include <sys/types.h>

namespace crashpad {

//! \brief An abstract interface to a snapshot representing a region of memory
//!     present in a snapshot process.
class MemorySnapshot {
 public:
  //! \brief An interface that MemorySnapshot clients must implement in order to
  //!     receive memory snapshot data.
  //!
  //! This callback-based model frees MemorySnapshot implementations from having
  //! to deal with memory region ownership problems. When a memory snapshot’s
  //! data is read, it will be passed to a delegate method.
  class Delegate {
   public:
    virtual ~Delegate() {}

    //! \brief Called by MemorySnapshot::Read() to provide data requested by a
    //!     call to that method.
    //!
    //! \param[in] data A pointer to the data that was read. The callee does not
    //!     take ownership of this data. This data is only valid for the
    //!     duration of the call to this method. This parameter may be `nullptr`
    //!     if \a size is `0`.
    //! \param[in] size The size of the data that was read.
    //!
    //! \return `true` on success, `false` on failure. MemoryDelegate::Read()
    //!     will use this as its own return value.
    virtual bool MemorySnapshotDelegateRead(void* data, size_t size) = 0;
  };

  virtual ~MemorySnapshot() {}

  //! \brief The base address of the memory snapshot in the snapshot process’
  //!     address space.
  virtual uint64_t Address() const = 0;

  //! \brief The size of the memory snapshot.
  virtual size_t Size() const = 0;

  //! \brief Calls Delegate::MemorySnapshotDelegateRead(), providing it with
  //!     the memory snapshot’s data.
  //!
  //! Implementations do not necessarily read the memory snapshot data prior to
  //! this method being called. Memory snapshot data may be loaded lazily and
  //! may be discarded after being passed to the delegate. This provides clean
  //! memory management without burdening a snapshot implementation with the
  //! requirement that it track all memory region data simultaneously.
  //!
  //! \return `false` on failure, otherwise, the return value of
  //!     Delegate::MemorySnapshotDelegateRead(), which should be `true` on
  //!     success and `false` on failure.
  virtual bool Read(Delegate* delegate) const = 0;
};

}  // namespace crashpad

#endif  // CRASHPAD_SNAPSHOT_MEMORY_SNAPSHOT_H_
