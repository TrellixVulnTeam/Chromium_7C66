// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NGWritingMode_h
#define NGWritingMode_h

#include "core/CoreExport.h"
#include "platform/text/WritingMode.h"

namespace blink {

enum NGWritingMode {
  HorizontalTopBottom = 0,
  VerticalRightLeft = 1,
  VerticalLeftRight = 2,
  SidewaysRightLeft = 3,
  SidewaysLeftRight = 4
};

CORE_EXPORT NGWritingMode FromPlatformWritingMode(WritingMode);

}  // namespace blink

#endif  // NGWritingMode_h
