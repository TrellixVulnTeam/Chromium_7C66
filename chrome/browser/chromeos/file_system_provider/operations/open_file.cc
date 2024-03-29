// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/file_system_provider/operations/open_file.h"

#include <string>

#include "chrome/common/extensions/api/file_system_provider.h"
#include "chrome/common/extensions/api/file_system_provider_internal.h"

namespace chromeos {
namespace file_system_provider {
namespace operations {

OpenFile::OpenFile(
    extensions::EventRouter* event_router,
    const ProvidedFileSystemInfo& file_system_info,
    const base::FilePath& file_path,
    OpenFileMode mode,
    const ProvidedFileSystemInterface::OpenFileCallback& callback)
    : Operation(event_router, file_system_info),
      file_path_(file_path),
      mode_(mode),
      callback_(callback) {
}

OpenFile::~OpenFile() {
}

bool OpenFile::Execute(int request_id) {
  using extensions::api::file_system_provider::OpenFileRequestedOptions;

  if (!file_system_info_.writable() && mode_ == OPEN_FILE_MODE_WRITE) {
    return false;
  }

  OpenFileRequestedOptions options;
  options.file_system_id = file_system_info_.file_system_id();
  options.request_id = request_id;
  options.file_path = file_path_.AsUTF8Unsafe();

  switch (mode_) {
    case OPEN_FILE_MODE_READ:
      options.mode = extensions::api::file_system_provider::OPEN_FILE_MODE_READ;
      break;
    case OPEN_FILE_MODE_WRITE:
      options.mode =
          extensions::api::file_system_provider::OPEN_FILE_MODE_WRITE;
      break;
  }

  return SendEvent(
      request_id,
      extensions::events::FILE_SYSTEM_PROVIDER_ON_OPEN_FILE_REQUESTED,
      extensions::api::file_system_provider::OnOpenFileRequested::kEventName,
      extensions::api::file_system_provider::OnOpenFileRequested::Create(
          options));
}

void OpenFile::OnSuccess(int request_id,
                         std::unique_ptr<RequestValue> result,
                         bool has_more) {
  // File handle is the same as request id of the OpenFile operation.
  callback_.Run(request_id, base::File::FILE_OK);
}

void OpenFile::OnError(int /* request_id */,
                       std::unique_ptr<RequestValue> /* result */,
                       base::File::Error error) {
  callback_.Run(0 /* file_handle */, error);
}

}  // namespace operations
}  // namespace file_system_provider
}  // namespace chromeos
