// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NET_LOG_NET_EXPORT_UI_CONSTANTS_H_
#define COMPONENTS_NET_LOG_NET_EXPORT_UI_CONSTANTS_H_

namespace net_log {

// Resource paths.
// Must match the resource file names.
extern const char kNetExportUIJS[];

// Message handlers.
// Must match the constants used in the resource files.
extern const char kGetExportNetLogInfoHandler[];
extern const char kSendNetLogHandler[];
extern const char kStartNetLogHandler[];
extern const char kStopNetLogHandler[];

// Other values.
// Must match the constants used in the resource files.
extern const char kOnExportNetLogInfoChanged[];

}  // namespace net_log

#endif  // COMPONENTS_NET_LOG_NET_EXPORT_UI_CONSTANTS_H_
