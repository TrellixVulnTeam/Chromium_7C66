// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_BROWSER_SIDE_NAVIGATION_POLICY_H_
#define CONTENT_PUBLIC_COMMON_BROWSER_SIDE_NAVIGATION_POLICY_H_

#include "content/common/content_export.h"

// A centralized file for base helper methods and policy decisions about browser
// side navigation (aka PlzNavigate).
// Note: For now this houses a single non-member method but we might change to
// having an actual class if it should make more sense at some point.

namespace content {

CONTENT_EXPORT bool IsBrowserSideNavigationEnabled();

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_BROWSER_SIDE_NAVIGATION_POLICY_H_
