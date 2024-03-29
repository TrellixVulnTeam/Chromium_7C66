// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/autofill/core/browser/validation.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::ASCIIToUTF16;

namespace autofill {
namespace {

struct ExpirationDate {
  const char* year;
  const char* month;
};

struct IntExpirationDate {
  const int year;
  const int month;
};

// From https://www.paypalobjects.com/en_US/vhelp/paypalmanager_help/credit_card_numbers.htm
const char* const kValidNumbers[] = {
  "378282246310005",
  "3714 4963 5398 431",
  "3787-3449-3671-000",
  "5610591081018250",
  "3056 9309 0259 04",
  "3852-0000-0232-37",
  "6011111111111117",
  "6011 0009 9013 9424",
  "3530-1113-3330-0000",
  "3566002020360505",
  "5555 5555 5555 4444",
  "5105-1051-0510-5100",
  "4111111111111111",
  "4012 8888 8888 1881",
  "4222-2222-2222-2",
  "5019717010103742",
  "6331101999990016",
  "6247130048162403",
};
const char* const kInvalidNumbers[] = {
  "4111 1111 112", /* too short */
  "41111111111111111115", /* too long */
  "4111-1111-1111-1110", /* wrong Luhn checksum */
  "3056 9309 0259 04aa", /* non-digit characters */
};
const char kCurrentDate[]="1 May 2013";
const IntExpirationDate kValidCreditCardIntExpirationDate[] = {
  { 2013, 5 },  // Valid month in current year.
  { 2014, 1 },  // Any month in next year.
  { 2014, 12 },  // Edge condition.
};
const IntExpirationDate kInvalidCreditCardIntExpirationDate[] = {
  { 2013, 4 },  // Previous month in current year.
  { 2012, 12 },  // Any month in previous year.
  { 2015, 13 },  // Not a real month.
  { 2015, 0 },  // Zero is legal in the CC class but is not a valid date.
};
const char* const kValidCreditCardSecurityCode[] = {
  "323",  // 3-digit CSC.
  "3234",  // 4-digit CSC.
};
const char* const kInvalidCreditCardSecurityCode[] = {
  "32",  // CSC too short.
  "12345",  // CSC too long.
  "asd",  // non-numeric CSC.
};
const char* const kValidEmailAddress[] = {
  "user@example",
  "user@example.com",
  "user@subdomain.example.com",
  "user+postfix@example.com",
};
const char* const kInvalidEmailAddress[] = {
  "user",
  "foo.com",
  "user@",
  "user@=example.com"
};
const char kAmericanExpressCard[] = "341111111111111";
const char kVisaCard[] = "4111111111111111";
const char kAmericanExpressCVC[] = "1234";
const char kVisaCVC[] = "123";
}  // namespace

TEST(AutofillValidation, IsValidCreditCardNumber) {
  for (const char* valid_number : kValidNumbers) {
    SCOPED_TRACE(valid_number);
    EXPECT_TRUE(IsValidCreditCardNumber(ASCIIToUTF16(valid_number)));
  }
  for (const char* invalid_number : kInvalidNumbers) {
    SCOPED_TRACE(invalid_number);
    EXPECT_FALSE(IsValidCreditCardNumber(ASCIIToUTF16(invalid_number)));
  }
}

TEST(AutofillValidation, IsValidCreditCardIntExpirationDate) {
  base::Time now;
  ASSERT_TRUE(base::Time::FromString(kCurrentDate, &now));

  for (const IntExpirationDate& data : kValidCreditCardIntExpirationDate) {
    SCOPED_TRACE(data.year);
    SCOPED_TRACE(data.month);
    EXPECT_TRUE(IsValidCreditCardExpirationDate(data.year, data.month, now));
  }
  for (const IntExpirationDate& data : kInvalidCreditCardIntExpirationDate) {
    SCOPED_TRACE(data.year);
    SCOPED_TRACE(data.month);
    EXPECT_TRUE(!IsValidCreditCardExpirationDate(data.year, data.month, now));
  }
}
TEST(AutofillValidation, IsValidCreditCardSecurityCode) {
  for (const char* valid_code : kValidCreditCardSecurityCode) {
    SCOPED_TRACE(valid_code);
    EXPECT_TRUE(IsValidCreditCardSecurityCode(ASCIIToUTF16(valid_code)));
  }
  for (const char* invalid_code : kInvalidCreditCardSecurityCode) {
    SCOPED_TRACE(invalid_code);
    EXPECT_FALSE(IsValidCreditCardSecurityCode(ASCIIToUTF16(invalid_code)));
  }
}

TEST(AutofillValidation, IsValidEmailAddress) {
  for (const char* valid_email : kValidEmailAddress) {
    SCOPED_TRACE(valid_email);
    EXPECT_TRUE(IsValidEmailAddress(ASCIIToUTF16(valid_email)));
  }
  for (const char* invalid_email : kInvalidEmailAddress) {
    SCOPED_TRACE(invalid_email);
    EXPECT_FALSE(IsValidEmailAddress(ASCIIToUTF16(invalid_email)));
  }
}

TEST(AutofillValidation, IsValidCreditCardSecurityCodeWithNumber) {
  EXPECT_TRUE(IsValidCreditCardSecurityCode(
      ASCIIToUTF16(kAmericanExpressCVC), ASCIIToUTF16(kAmericanExpressCard)));
  EXPECT_TRUE(IsValidCreditCardSecurityCode(
      ASCIIToUTF16(kVisaCVC), ASCIIToUTF16(kVisaCard)));
  EXPECT_FALSE(IsValidCreditCardSecurityCode(
      ASCIIToUTF16(kVisaCVC), ASCIIToUTF16(kAmericanExpressCard)));
  EXPECT_FALSE(IsValidCreditCardSecurityCode(
      ASCIIToUTF16(kAmericanExpressCVC), ASCIIToUTF16(kVisaCard)));
  EXPECT_TRUE(IsValidCreditCardSecurityCode(
      ASCIIToUTF16(kVisaCVC), ASCIIToUTF16(kInvalidNumbers[0])));
  EXPECT_FALSE(IsValidCreditCardSecurityCode(
      ASCIIToUTF16(kAmericanExpressCVC), ASCIIToUTF16(kInvalidNumbers[0])));
}

}  // namespace autofill
