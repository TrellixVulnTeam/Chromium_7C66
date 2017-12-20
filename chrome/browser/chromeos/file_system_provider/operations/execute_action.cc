// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/file_system_provider/operations/execute_action.h"

#include <algorithm>
#include <string>

#include "chrome/common/extensions/api/file_system_provider.h"
#include "chrome/common/extensions/api/file_system_provider_internal.h"

namespace chromeos {
namespace file_system_provider {
namespace operations {

ExecuteAction::ExecuteAction(
    extensions::EventRouter* event_router,
    const ProvidedFileSystemInfo& file_system_info,
    const std::vector<base::FilePath>& entry_paths,
    const std::string& action_id,
    const storage::AsyncFileUtil::StatusCallback& callback)
    : Operation(event_router, file_system_info),
      entry_paths_(entry_paths),
      action_id_(action_id),
      callback_(callback) {}

ExecuteAction::~ExecuteAction() {
}

bool ExecuteAction::Execute(int request_id) {
  using extensions::api::file_system_provider::ExecuteActionRequestedOptions;

  ExecuteActionRequestedOptions options;
  options.file_system_id = file_system_info_.file_system_id();
  options.request_id = request_id;
  for (const auto& entry_path : entry_paths_)
    options.entry_paths.push_back(entry_path.AsUTF8Unsafe());
  options.action_id = action_id_;

  return SendEvent(
      request_id,
      extensions::events::FILE_SYSTEM_PROVIDER_ON_EXECUTE_ACTION_REQUESTED,
      extensions::api::file_system_provider::OnExecuteActionRequested::
          kEventName,
      extensions::api::file_system_provider::OnExecuteActionRequested::Create(
          options));
}

void ExecuteAction::OnSuccess(int /* request_id */,
                              std::unique_ptr<RequestValue> result,
                              bool has_more) {
  callback_.Run(base::File::FILE_OK);
}

void ExecuteAction::OnError(int /* request_id */,
                            std::unique_ptr<RequestValue> /* result */,
                            base::File::Error error) {
  callback_.Run(error);
}

}  // namespace operations
}  // namespace file_system_provider
}  // namespace chromeos
