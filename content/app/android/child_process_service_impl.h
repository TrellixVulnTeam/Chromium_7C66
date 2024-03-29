// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_APP_ANDROID_CHILD_PROCESS_SERVICE_IMPL_H_
#define CONTENT_APP_ANDROID_CHILD_PROCESS_SERVICE_IMPL_H_

#include <jni.h>

namespace content {
bool RegisterChildProcessServiceImpl(JNIEnv* env);
}  // namespace content

#endif  // CONTENT_APP_ANDROID_CHILD_PROCESS_SERVICE_IMPL_H_
