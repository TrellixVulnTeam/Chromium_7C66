// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/weborigin/KURL.h"
#include "platform/weborigin/KnownPorts.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(KnownPortsTest, IsDefaultPortForProtocol) {
  struct TestCase {
    const unsigned short port;
    const char* protocol;
    const bool isKnown;
  } inputs[] = {
      // Known ones.
      {80, "http", true},
      {443, "https", true},
      {80, "ws", true},
      {443, "wss", true},
      {21, "ftp", true},
      {990, "ftps", true},

      // Unknown ones.
      {5, "foo", false},
      {80, "http:", false},
      {443, "http", false},
      {21, "ftps", false},
      {990, "ftp", false},

      // With upper cases.
      {80, "HTTP", false},
      {443, "Https", false},
  };

  for (const TestCase& test : inputs) {
    bool result = isDefaultPortForProtocol(test.port, test.protocol);
    EXPECT_EQ(test.isKnown, result);
  }
}

TEST(KnownPortsTest, DefaultPortForProtocol) {
  struct TestCase {
    const unsigned short port;
    const char* protocol;
  } inputs[] = {
      // Known ones.
      {80, "http"},
      {443, "https"},
      {80, "ws"},
      {443, "wss"},
      {21, "ftp"},
      {990, "ftps"},

      // Unknown ones.
      {0, "foo"},
      {0, "http:"},
      {0, "HTTP"},
      {0, "Https"},
  };

  for (const TestCase& test : inputs)
    EXPECT_EQ(test.port, defaultPortForProtocol(test.protocol));
}

TEST(KnownPortsTest, IsPortAllowedForScheme) {
  struct TestCase {
    const char* url;
    const bool isAllowed;
  } inputs[] = {
      // Allowed ones.
      {"http://example.com", true},
      {"file://example.com", true},
      {"file://example.com:87", true},
      {"ftp://example.com:21", true},
      {"http://example.com:80", true},
      {"http://example.com:8889", true},

      // Disallowed ones.
      {"ftp://example.com:87", false},
      {"ws://example.com:21", false},
  };

  for (const TestCase& test : inputs)
    EXPECT_EQ(test.isAllowed,
              isPortAllowedForScheme(KURL(ParsedURLString, test.url)));
}

}  // namespace blink
