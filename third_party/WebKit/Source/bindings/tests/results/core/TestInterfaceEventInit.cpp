// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// clang-format off
#include "TestInterfaceEventInit.h"

namespace blink {

TestInterfaceEventInit::TestInterfaceEventInit() {
}

TestInterfaceEventInit::~TestInterfaceEventInit() {}

TestInterfaceEventInit::TestInterfaceEventInit(const TestInterfaceEventInit&) = default;

TestInterfaceEventInit& TestInterfaceEventInit::operator=(const TestInterfaceEventInit&) = default;

bool TestInterfaceEventInit::hasStringMember() const {
  return !m_stringMember.isNull();
}
String TestInterfaceEventInit::stringMember() const {
  return m_stringMember;
}
void TestInterfaceEventInit::setStringMember(String value) {
  m_stringMember = value;
}

DEFINE_TRACE(TestInterfaceEventInit) {
  EventInit::trace(visitor);
}

}  // namespace blink