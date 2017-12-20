// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/os_crypt/ie7_password_win.h"

#include <windows.h>

#include <vector>

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(IE7PasswordTest, GetUserPassword) {
  // This is the unencrypted values of my keys under Storage2.
  // The passwords have been manually changed to abcdef... but the size remains
  // the same.
  const unsigned char kData1[] =
      "\x0c\x00\x00\x00\x38\x00\x00\x00\x2c\x00\x00\x00"
      "\x57\x49\x43\x4b\x18\x00\x00\x00\x02\x00\x00\x00"
      "\x67\x00\x72\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x4e\xfa\x67\x76\x22\x94\xc8\x01"
      "\x08\x00\x00\x00\x12\x00\x00\x00\x4e\xfa\x67\x76"
      "\x22\x94\xc8\x01\x0c\x00\x00\x00\x61\x00\x62\x00"
      "\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00\x68\x00"
      "\x00\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00"
      "\x66\x00\x67\x00\x68\x00\x69\x00\x6a\x00\x6b\x00"
      "\x6c\x00\x00\x00";

  const unsigned char kData2[] =
      "\x0c\x00\x00\x00\x38\x00\x00\x00\x24\x00\x00\x00"
      "\x57\x49\x43\x4b\x18\x00\x00\x00\x02\x00\x00\x00"
      "\x67\x00\x72\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xa8\xea\xf4\xe5\x9f\x9a\xc8\x01"
      "\x09\x00\x00\x00\x14\x00\x00\x00\xa8\xea\xf4\xe5"
      "\x9f\x9a\xc8\x01\x07\x00\x00\x00\x61\x00\x62\x00"
      "\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00\x68\x00"
      "\x69\x00\x00\x00\x61\x00\x62\x00\x63\x00\x64\x00"
      "\x65\x00\x66\x00\x67\x00\x00\x00";

  std::vector<unsigned char> decrypted_data1;
  decrypted_data1.resize(arraysize(kData1));
  memcpy(&decrypted_data1.front(), kData1, sizeof(kData1));

  std::vector<unsigned char> decrypted_data2;
  decrypted_data2.resize(arraysize(kData2));
  memcpy(&decrypted_data2.front(), kData2, sizeof(kData2));

  std::vector<ie7_password::DecryptedCredentials> credentials;
  ASSERT_TRUE(ie7_password::GetUserPassFromData(decrypted_data1, &credentials));
  ASSERT_EQ(1u, credentials.size());
  EXPECT_EQ(L"abcdefgh", credentials[0].username);
  EXPECT_EQ(L"abcdefghijkl", credentials[0].password);

  credentials.clear();
  ASSERT_TRUE(ie7_password::GetUserPassFromData(decrypted_data2, &credentials));
  ASSERT_EQ(1u, credentials.size());
  EXPECT_EQ(L"abcdefghi", credentials[0].username);
  EXPECT_EQ(L"abcdefg", credentials[0].password);
}

TEST(IE7PasswordTest, GetThreeUserPasswords) {
  // Unencrypted binary data holding 3 sets of credentials
  const unsigned char kData[] =
      "\x0c\x00\x00\x00\x78\x00\x00\x00\x48\x00\x00\x00\x57\x49\x43\x4b\x18"
      "\x00\x00\x00\x06\x00\x00\x00\x5c\x00\x55\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xc3\xf2\x4b\xda\x1d\xc4\xce\x01\x04\x00\x00"
      "\x00\x0a\x00\x00\x00\xc3\xf2\x4b\xda\x1d\xc4\xce\x01\x06\x00\x00\x00"
      "\x18\x00\x00\x00\xbe\x5e\xe9\xe1\x1d\xc4\xce\x01\x04\x00\x00\x00\x22"
      "\x00\x00\x00\xbe\x5e\xe9\xe1\x1d\xc4\xce\x01\x06\x00\x00\x00\x30\x00"
      "\x00\x00\x07\x50\x1f\xe6\x1d\xc4\xce\x01\x04\x00\x00\x00\x3a\x00\x00"
      "\x00\x07\x50\x1f\xe6\x1d\xc4\xce\x01\x06\x00\x00\x00\x71\x00\x77\x00"
      "\x65\x00\x72\x00\x00\x00\x71\x00\x77\x00\x65\x00\x72\x00\x74\x00\x79"
      "\x00\x00\x00\x61\x00\x73\x00\x64\x00\x66\x00\x00\x00\x61\x00\x73\x00"
      "\x64\x00\x66\x00\x67\x00\x68\x00\x00\x00\x7a\x00\x78\x00\x63\x00\x76"
      "\x00\x00\x00\x7a\x00\x78\x00\x63\x00\x76\x00\x62\x00\x6e\x00\x00\x00";

  std::vector<unsigned char> decrypted_data;
  decrypted_data.resize(arraysize(kData));
  memcpy(&decrypted_data.front(), kData, sizeof(kData));

  std::vector<ie7_password::DecryptedCredentials> credentials;
  ASSERT_TRUE(ie7_password::GetUserPassFromData(decrypted_data, &credentials));
  ASSERT_EQ(3u, credentials.size());
  EXPECT_EQ(L"qwer", credentials[0].username);
  EXPECT_EQ(L"qwerty", credentials[0].password);
  EXPECT_EQ(L"asdf", credentials[1].username);
  EXPECT_EQ(L"asdfgh", credentials[1].password);
  EXPECT_EQ(L"zxcv", credentials[2].username);
  EXPECT_EQ(L"zxcvbn", credentials[2].password);
}
