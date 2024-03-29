// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/model/syncable_service.h"

namespace syncer {

SyncableService::~SyncableService() {}

std::unique_ptr<AttachmentStoreForSync>
SyncableService::GetAttachmentStoreForSync() {
  return std::unique_ptr<AttachmentStoreForSync>();
}

void SyncableService::SetAttachmentService(
    std::unique_ptr<AttachmentService> attachment_service) {}

}  // namespace syncer
