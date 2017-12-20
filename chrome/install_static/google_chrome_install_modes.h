// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Brand-specific types and constants for Google Chrome.

#ifndef CHROME_INSTALL_STATIC_GOOGLE_CHROME_INSTALL_MODES_H_
#define CHROME_INSTALL_STATIC_GOOGLE_CHROME_INSTALL_MODES_H_

namespace install_static {

enum : bool {
  kUseGoogleUpdateIntegration = true,
};

enum InstallConstantIndex {
  STABLE_INDEX,
  CANARY_INDEX,
  NUM_INSTALL_MODES,
};

}  // namespace install_static

#endif  // CHROME_INSTALL_STATIC_GOOGLE_CHROME_INSTALL_MODES_H_
