// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/install_warning.h"

namespace extensions {

InstallWarning::InstallWarning(const std::string& message) : message(message) {
}

InstallWarning::InstallWarning(const std::string& message,
                               const std::string& key)
    : message(message), key(key) {
}

InstallWarning::InstallWarning(const std::string& message,
                               const std::string& key,
                               const std::string& specific)
    : message(message), key(key), specific(specific) {
}

InstallWarning::~InstallWarning() {
}

void PrintTo(const InstallWarning& warning, ::std::ostream* os) {
  // This is just for test error messages, so no need to escape '"'
  // characters inside the message.
  *os << "InstallWarning(\"" << warning.message << "\")";
}

}  // namespace extensions
