// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/country_data.h"

#include <utility>

#include "base/memory/singleton.h"
#include "grit/components_strings.h"
#include "third_party/icu/source/common/unicode/locid.h"

namespace autofill {
namespace {

struct StaticCountryData {
  char country_code[3];
  CountryData country_data;
};

// Maps country codes to localized label string identifiers. Keep this sorted
// by country code.
// This list is comprized of countries appearing in both
// //third_party/icu/source/data/region/en.txt and
// //third_party/libaddressinput/src/cpp/src/region_data_constants.cc.
const StaticCountryData kCountryData[] = {
    // clang-format off
  { "AC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "AD", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PARISH,
            ADDRESS_REQUIRES_STATE } },
  { "AE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_EMIRATE,
            ADDRESS_REQUIRES_STATE } },
  { "AF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_ADDRESS_LINE_1_ONLY } },
  { "AI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AQ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AS", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "AT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "AU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "AW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "AX", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "AZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BB", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PARISH,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BD", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "BF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BJ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "BM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BQ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "BS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BV", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "BZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "CC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CD", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "CI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "CO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CV", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CX", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "CZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "DE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "DJ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "DK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "DM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "DO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "DZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "EC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "EE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "EG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "EH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "ER", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "ES", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "ET", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "FI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "FJ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "FK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "FM", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "FO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "FR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GB", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_COUNTY,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GD", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_ADDRESS_LINE_1_ONLY } },
  { "GL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GP", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GQ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "GT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GU", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "GW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "GY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "HK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_AREA,
            ADDRESS_REQUIRES_STATE } },
  { "HM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "HN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "HR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "HT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "HU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "ID", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "IE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_COUNTY,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "IL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "IM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "IN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "IO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "IQ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "IR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "IS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "IT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "JE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "JM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PARISH,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "JO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "JP", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PREFECTURE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "KE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "KP", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "KW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "KY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIRES_STATE } },
  { "KZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LB", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "LK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "LV", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "LY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MD", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "ME", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "MG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MH", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "MK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "ML", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_ADDRESS_LINE_1_ONLY } },
  { "MP", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "MQ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "MR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MV", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "MX", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "MY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "MZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "NE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_DEPARTMENT,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "NO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "NP", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_DISTRICT,
            ADDRESS_REQUIRES_STATE } },
  { "NU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "NZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "OM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "PA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "PE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "PF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "PG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "PH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "PK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "PL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "PM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "PN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "PR", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "PS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "PT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "PW", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "PY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "QA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "RE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "RO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "RS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "RU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "RW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SB", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "SG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_ZIP } },
  { "SH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "SI", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SJ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "SK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_ZIP } },
  { "SN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "SR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "ST", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SV", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "SX", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "SZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "TC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "TD", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TH", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TJ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TL", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TO", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TR", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "TT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TV", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_ISLAND,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "TW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_COUNTY,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "TZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "UA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "UG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "UM", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "US", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "UY", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "UZ", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "VA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "VC", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "VE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_STATE } },
  { "VG", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_ADDRESS_LINE_1_ONLY } },
  { "VI", { IDS_AUTOFILL_FIELD_LABEL_ZIP_CODE,
            IDS_AUTOFILL_FIELD_LABEL_STATE,
            ADDRESS_REQUIRES_CITY_STATE_ZIP } },
  { "VN", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "VU", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "WF", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "WS", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
  { "XK", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "YE", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "YT", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "ZA", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY_ZIP } },
  { "ZM", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIRES_CITY } },
  { "ZW", { IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
            IDS_AUTOFILL_FIELD_LABEL_PROVINCE,
            ADDRESS_REQUIREMENTS_UNKNOWN } },
    // clang-format on
};

// GetCountryCodes and GetCountryData compute the data for CountryDataMap
// based on |kCountryData|.
std::vector<std::string> GetCountryCodes() {
  std::vector<std::string> country_codes;
  country_codes.reserve(arraysize(kCountryData));
  for (const auto& static_data : kCountryData) {
    country_codes.push_back(static_data.country_code);
  }
  return country_codes;
}

std::map<std::string, CountryData> GetCountryData() {
  std::map<std::string, CountryData> country_data;
  // Add all the countries we have explicit data for.
  for (const auto& static_data : kCountryData) {
    country_data.insert(
        std::make_pair(static_data.country_code, static_data.country_data));
  }

  // Add any other countries that ICU knows about, falling back to default data
  // values.
  for (const char* const* country_pointer = icu::Locale::getISOCountries();
       *country_pointer; ++country_pointer) {
    std::string country_code = *country_pointer;
    if (!country_data.count(country_code)) {
      CountryData data = {IDS_AUTOFILL_FIELD_LABEL_POSTAL_CODE,
                          IDS_AUTOFILL_FIELD_LABEL_PROVINCE};
      country_data.insert(
          std::make_pair(std::move(country_code), std::move(data)));
    }
  }
  return country_data;
}

}  // namespace

// static
CountryDataMap* CountryDataMap::GetInstance() {
  return base::Singleton<CountryDataMap>::get();
}

CountryDataMap::CountryDataMap()
    : country_data_(GetCountryData()), country_codes_(GetCountryCodes()) {}

CountryDataMap::~CountryDataMap() = default;

}  // namespace autofill
