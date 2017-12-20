// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/net_log/net_export_ui_constants.h"

namespace net_log {

// Resource paths.
const char kNetExportUIJS[] = "net_export.js";

// Message handlers.
const char kGetExportNetLogInfoHandler[] = "getExportNetLogInfo";
const char kSendNetLogHandler[] = "sendNetLog";
const char kStartNetLogHandler[] = "startNetLog";
const char kStopNetLogHandler[] = "stopNetLog";

// Other values.
const char kOnExportNetLogInfoChanged[] =
    "NetExportView.getInstance().onExportNetLogInfoChanged";

}  // namespace net_log
