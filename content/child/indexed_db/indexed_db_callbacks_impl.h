// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CHILD_INDEXED_DB_INDEXED_DB_CALLBACKS_IMPL_H_
#define CONTENT_CHILD_INDEXED_DB_INDEXED_DB_CALLBACKS_IMPL_H_

#include "content/common/indexed_db/indexed_db.mojom.h"
#include "mojo/public/cpp/bindings/associated_binding.h"

namespace blink {
class WebIDBCallbacks;
}

namespace content {

class ThreadSafeSender;

// Implements the child-process end of the pipe used to deliver callbacks. It
// is owned by the IO thread. |callback_runner_| is used to post tasks back to
// the thread which owns the blink::WebIDBCallbacks.
class IndexedDBCallbacksImpl : public indexed_db::mojom::Callbacks {
 public:
  enum : int64_t { kNoTransaction = -1 };

  // This class holds the parts of the internal state of IndexedDBCallbacksImpl
  // that must live on whatever renderer or worker thread the API is used from.
  class InternalState {
   public:
    InternalState(std::unique_ptr<blink::WebIDBCallbacks> callbacks,
                  int64_t transaction_id,
                  scoped_refptr<base::SingleThreadTaskRunner> io_runner,
                  scoped_refptr<ThreadSafeSender> thread_safe_sender);
    ~InternalState();

    void Error(int32_t code, const base::string16& message);
    void SuccessStringList(const std::vector<base::string16>& value);
    void Blocked(int64_t existing_version);
    void UpgradeNeeded(indexed_db::mojom::DatabaseAssociatedPtrInfo database,
                       int64_t old_version,
                       blink::WebIDBDataLoss data_loss,
                       const std::string& data_loss_message,
                       const content::IndexedDBDatabaseMetadata& metadata);
    void SuccessDatabase(indexed_db::mojom::DatabaseAssociatedPtrInfo database,
                         const content::IndexedDBDatabaseMetadata& metadata);
    void SuccessCursor(int32_t cursor_id,
                       const IndexedDBKey& key,
                       const IndexedDBKey& primary_key,
                       indexed_db::mojom::ValuePtr value);
    void SuccessValue(indexed_db::mojom::ReturnValuePtr value);
    void SuccessArray(std::vector<indexed_db::mojom::ReturnValuePtr> values);
    void SuccessKey(const IndexedDBKey& key);
    void SuccessInteger(int64_t value);
    void Success();

   private:
    std::unique_ptr<blink::WebIDBCallbacks> callbacks_;
    int64_t transaction_id_;
    scoped_refptr<base::SingleThreadTaskRunner> io_runner_;
    scoped_refptr<ThreadSafeSender> thread_safe_sender_;

    DISALLOW_COPY_AND_ASSIGN(InternalState);
  };

  IndexedDBCallbacksImpl(std::unique_ptr<blink::WebIDBCallbacks> callbacks,
                         int64_t transaction_id,
                         scoped_refptr<base::SingleThreadTaskRunner> io_runner,
                         scoped_refptr<ThreadSafeSender> thread_safe_sender);
  ~IndexedDBCallbacksImpl() override;

  // indexed_db::mojom::Callbacks implementation:
  void Error(int32_t code, const base::string16& message) override;
  void SuccessStringList(const std::vector<base::string16>& value) override;
  void Blocked(int64_t existing_version) override;
  void UpgradeNeeded(
      indexed_db::mojom::DatabaseAssociatedPtrInfo database_info,
      int64_t old_version,
      blink::WebIDBDataLoss data_loss,
      const std::string& data_loss_message,
      const content::IndexedDBDatabaseMetadata& metadata) override;
  void SuccessDatabase(
      indexed_db::mojom::DatabaseAssociatedPtrInfo database_info,
      const content::IndexedDBDatabaseMetadata& metadata) override;
  void SuccessCursor(int32_t cursor_id,
                     const IndexedDBKey& key,
                     const IndexedDBKey& primary_key,
                     indexed_db::mojom::ValuePtr value) override;
  void SuccessValue(indexed_db::mojom::ReturnValuePtr value) override;
  void SuccessArray(
      std::vector<indexed_db::mojom::ReturnValuePtr> values) override;
  void SuccessKey(const IndexedDBKey& key) override;
  void SuccessInteger(int64_t value) override;
  void Success() override;

 private:
  // |internal_state_| is owned by the thread on which |callback_runner_|
  // executes tasks and must be destroyed there.
  InternalState* internal_state_;
  scoped_refptr<base::SingleThreadTaskRunner> callback_runner_;

  DISALLOW_COPY_AND_ASSIGN(IndexedDBCallbacksImpl);
};

}  // namespace content

#endif  // CONTENT_CHILD_INDEXED_DB_INDEXED_DB_CALLBACKS_IMPL_H_
