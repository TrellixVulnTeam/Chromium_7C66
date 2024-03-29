// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/client/cursor_client.h"

#include "ui/aura/window.h"
#include "ui/aura/window_property.h"

DECLARE_WINDOW_PROPERTY_TYPE(aura::client::CursorClient*)

namespace aura {
namespace client {

// A property key to store a client that handles window moves.
DEFINE_LOCAL_WINDOW_PROPERTY_KEY(
    CursorClient*, kCursorClientKey, NULL);

void SetCursorClient(Window* window, CursorClient* client) {
  window->SetProperty(kCursorClientKey, client);
}

CursorClient* GetCursorClient(Window* window) {
  return window->GetProperty(kCursorClientKey);
}

}  // namespace client
}  // namespace aura
