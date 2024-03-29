// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gin/try_catch.h"

#include <sstream>

#include "gin/converter.h"

namespace {

v8::Local<v8::String> GetSourceLine(v8::Isolate* isolate,
                                    v8::Local<v8::Message> message) {
  auto maybe = message->GetSourceLine(isolate->GetCurrentContext());
  v8::Local<v8::String> source_line;
  return maybe.ToLocal(&source_line) ? source_line : v8::String::Empty(isolate);
}

}  // namespace

namespace gin {

TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(isolate), try_catch_(isolate) {
}

TryCatch::~TryCatch() {
}

bool TryCatch::HasCaught() {
  return try_catch_.HasCaught();
}

std::string TryCatch::GetStackTrace() {
  if (!HasCaught()) {
    return "";
  }

  std::stringstream ss;
  v8::Local<v8::Message> message = try_catch_.Message();
  ss << V8ToString(message->Get()) << std::endl
     << V8ToString(GetSourceLine(isolate_, message)) << std::endl;

  v8::Local<v8::StackTrace> trace = message->GetStackTrace();
  if (trace.IsEmpty())
    return ss.str();

  int len = trace->GetFrameCount();
  for (int i = 0; i < len; ++i) {
    v8::Local<v8::StackFrame> frame = trace->GetFrame(i);
    ss << V8ToString(frame->GetScriptName()) << ":"
       << frame->GetLineNumber() << ":"
       << frame->GetColumn() << ": "
       << V8ToString(frame->GetFunctionName())
       << std::endl;
  }
  return ss.str();
}

}  // namespace gin
