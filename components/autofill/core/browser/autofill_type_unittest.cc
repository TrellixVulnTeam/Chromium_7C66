// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/autofill_type.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace autofill {
namespace {

TEST(AutofillTypeTest, ServerFieldTypes) {
  // No server data.
  AutofillType none(NO_SERVER_DATA);
  EXPECT_EQ(NO_SERVER_DATA, none.GetStorableType());
  EXPECT_EQ(NO_GROUP, none.group());

  // Unknown type.
  AutofillType unknown(UNKNOWN_TYPE);
  EXPECT_EQ(UNKNOWN_TYPE, unknown.GetStorableType());
  EXPECT_EQ(NO_GROUP, unknown.group());

  // Type with group but no subgroup.
  AutofillType first(NAME_FIRST);
  EXPECT_EQ(NAME_FIRST, first.GetStorableType());
  EXPECT_EQ(NAME, first.group());

  // Type with group and subgroup.
  AutofillType phone(PHONE_HOME_NUMBER);
  EXPECT_EQ(PHONE_HOME_NUMBER, phone.GetStorableType());
  EXPECT_EQ(PHONE_HOME, phone.group());

  // Billing type.
  AutofillType billing_address(ADDRESS_BILLING_LINE1);
  EXPECT_EQ(ADDRESS_HOME_LINE1, billing_address.GetStorableType());
  EXPECT_EQ(ADDRESS_BILLING, billing_address.group());

  // Last value, to check any offset errors.
  AutofillType last(NAME_BILLING_SUFFIX);
  EXPECT_EQ(NAME_SUFFIX, last.GetStorableType());
  EXPECT_EQ(NAME_BILLING, last.group());

  // Boundary (error) condition.
  AutofillType boundary(MAX_VALID_FIELD_TYPE);
  EXPECT_EQ(UNKNOWN_TYPE, boundary.GetStorableType());
  EXPECT_EQ(NO_GROUP, boundary.group());

  // Beyond the boundary (error) condition.
  AutofillType beyond(static_cast<ServerFieldType>(MAX_VALID_FIELD_TYPE + 10));
  EXPECT_EQ(UNKNOWN_TYPE, beyond.GetStorableType());
  EXPECT_EQ(NO_GROUP, beyond.group());

  // In-between value.  Missing from enum but within range.  Error condition.
  AutofillType between(static_cast<ServerFieldType>(16));
  EXPECT_EQ(UNKNOWN_TYPE, between.GetStorableType());
  EXPECT_EQ(NO_GROUP, between.group());
}

TEST(AutofillTypeTest, HtmlFieldTypes) {
  // Unknown type.
  AutofillType unknown(HTML_TYPE_UNSPECIFIED, HTML_MODE_NONE);
  EXPECT_EQ(UNKNOWN_TYPE, unknown.GetStorableType());
  EXPECT_EQ(NO_GROUP, unknown.group());

  // Type with group but no subgroup.
  AutofillType first(HTML_TYPE_GIVEN_NAME, HTML_MODE_NONE);
  EXPECT_EQ(NAME_FIRST, first.GetStorableType());
  EXPECT_EQ(NAME, first.group());

  // Type with group and subgroup.
  AutofillType phone(HTML_TYPE_TEL, HTML_MODE_NONE);
  EXPECT_EQ(PHONE_HOME_WHOLE_NUMBER, phone.GetStorableType());
  EXPECT_EQ(PHONE_HOME, phone.group());

  // Last value, to check any offset errors.
  AutofillType last(HTML_TYPE_CREDIT_CARD_EXP_4_DIGIT_YEAR, HTML_MODE_NONE);
  EXPECT_EQ(CREDIT_CARD_EXP_4_DIGIT_YEAR, last.GetStorableType());
  EXPECT_EQ(CREDIT_CARD, last.group());

  // Shipping mode.
  AutofillType shipping_first(HTML_TYPE_GIVEN_NAME, HTML_MODE_SHIPPING);
  EXPECT_EQ(NAME_FIRST, shipping_first.GetStorableType());
  EXPECT_EQ(NAME, shipping_first.group());

  // Billing mode.
  AutofillType billing_first(HTML_TYPE_GIVEN_NAME, HTML_MODE_BILLING);
  EXPECT_EQ(NAME_FIRST, billing_first.GetStorableType());
  EXPECT_EQ(NAME_BILLING, billing_first.group());
}

}  // namespace
}  // namespace autofill
