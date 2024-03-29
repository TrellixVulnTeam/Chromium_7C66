// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/bidirectional_stream_request_info.h"

namespace net {

BidirectionalStreamRequestInfo::BidirectionalStreamRequestInfo()
    : priority(LOW), end_stream_on_headers(false) {}

BidirectionalStreamRequestInfo::~BidirectionalStreamRequestInfo() {}

}  // namespace net
