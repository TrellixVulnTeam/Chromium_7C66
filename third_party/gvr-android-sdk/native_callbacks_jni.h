// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     com/google/vr/internal/controller/NativeCallbacks

// Local modification includes:
// 1. Remove all implementaiton, only keep definition.
// 2. Use absolute path instead of relative path.
// 3. Removed all helper functions such as: Create.
// 4. Replace all nativeHandle to handle. This is because jni_generator.py
// require jni functions start with "native" prefix. So we add the prefix to
// generate the file. But the real jni functions in the static library
// doesn't have the prefix.
// 5. Added function RegisterNativeCallbacksNatives at the end of this file.

#ifndef com_google_vr_internal_controller_NativeCallbacks_JNI
#define com_google_vr_internal_controller_NativeCallbacks_JNI

#include "base/android/jni_android.h"
// ----------------------------------------------------------------------------
// Native JNI methods
// ----------------------------------------------------------------------------
#include <jni.h>

#include "base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kNativeCallbacksClassPath[] =
    "com/google/vr/internal/controller/NativeCallbacks";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_NativeCallbacks_clazz __attribute__((unused)) = 0;
#define NativeCallbacks_clazz(env)                            \
  base::android::LazyGetClass(env, kNativeCallbacksClassPath, \
                              &g_NativeCallbacks_clazz)

}  // namespace

namespace NativeCallbacks {

// Step 2: method stubs.

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleStateChanged(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerId,
    jint newState);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleControllerRecentered(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jlong timestampNanos,
    jfloat qx,
    jfloat qy,
    jfloat qz,
    jfloat qw);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleTouchEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jlong timestampNanos,
    jint action,
    jfloat x,
    jfloat y);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleOrientationEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jlong timestampNanos,
    jfloat qx,
    jfloat qy,
    jfloat qz,
    jfloat qw);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleButtonEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jlong timestampNanos,
    jint buttonCode,
    jboolean down);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleAccelEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jlong timestampNanos,
    jfloat x,
    jfloat y,
    jfloat z);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleGyroEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jlong timestampNanos,
    jfloat x,
    jfloat y,
    jfloat z);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceInitFailed(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint failureReason);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceFailed(
    JNIEnv* env,
    jobject jcaller,
    jlong userData);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceUnavailable(
    JNIEnv* env,
    jobject jcaller,
    jlong userData);
extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceConnected(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint flags);

extern "C" __attribute__((visibility("default"))) void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceDisconnected(
    JNIEnv* env,
    jobject jcaller,
    jlong userData);

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsNativeCallbacks[] = {
    {"handleStateChanged",
     "("
     "J"
     "I"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleStateChanged)},
    {"handleControllerRecentered",
     "("
     "J"
     "J"
     "F"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleControllerRecentered)},
    {"handleTouchEvent",
     "("
     "J"
     "J"
     "I"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleTouchEvent)},
    {"handleOrientationEvent",
     "("
     "J"
     "J"
     "F"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleOrientationEvent)},
    {"handleButtonEvent",
     "("
     "J"
     "J"
     "I"
     "Z"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleButtonEvent)},
    {"handleAccelEvent",
     "("
     "J"
     "J"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleAccelEvent)},
    {"handleGyroEvent",
     "("
     "J"
     "J"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleGyroEvent)},
    {"handleServiceInitFailed",
     "("
     "J"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceInitFailed)},
    {"handleServiceFailed",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceFailed)},
    {"handleServiceUnavailable",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceUnavailable)},
    {"handleServiceConnected",
     "("
     "J"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceConnected)},
    {"handleServiceDisconnected",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceDisconnected)},
};

static bool RegisterNativesImpl(JNIEnv* env) {
  if (base::android::IsManualJniRegistrationDisabled())
    return true;

  const int kMethodsNativeCallbacksSize = arraysize(kMethodsNativeCallbacks);

  if (env->RegisterNatives(NativeCallbacks_clazz(env), kMethodsNativeCallbacks,
                           kMethodsNativeCallbacksSize) < 0) {
    jni_generator::HandleRegistrationError(env, NativeCallbacks_clazz(env),
                                           __FILE__);
    return false;
  }

  return true;
}

static bool RegisterNativeCallbacksNatives(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace NativeCallbacks

#endif  // com_google_vr_internal_controller_NativeCallbacks_JNI