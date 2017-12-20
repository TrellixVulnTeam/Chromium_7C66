// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/data_usage/core/data_use.h"

namespace data_usage {

namespace {

bool AreNonByteCountFieldsEqual(const DataUse& a, const DataUse& b) {
  return a.url == b.url && a.request_start == b.request_start &&
         a.first_party_for_cookies == b.first_party_for_cookies &&
         a.tab_id == b.tab_id && a.connection_type == b.connection_type &&
         a.mcc_mnc == b.mcc_mnc;
}

bool AreByteCountFieldsEqual(const DataUse& a, const DataUse& b) {
  return a.tx_bytes == b.tx_bytes && a.rx_bytes == b.rx_bytes;
}

}  // namespace

DataUse::DataUse(const GURL& url,
                 const base::TimeTicks& request_start,
                 const GURL& first_party_for_cookies,
                 int32_t tab_id,
                 net::NetworkChangeNotifier::ConnectionType connection_type,
                 const std::string& mcc_mnc,
                 int64_t tx_bytes,
                 int64_t rx_bytes)
    : url(url),
      request_start(request_start),
      first_party_for_cookies(first_party_for_cookies),
      tab_id(tab_id),
      connection_type(connection_type),
      mcc_mnc(mcc_mnc),
      tx_bytes(tx_bytes),
      rx_bytes(rx_bytes) {}

DataUse::DataUse(const DataUse& other) = default;

DataUse::~DataUse() {}

bool DataUse::operator==(const DataUse& other) const {
  return AreNonByteCountFieldsEqual(*this, other) &&
         AreByteCountFieldsEqual(*this, other);
}

bool DataUse::CanCombineWith(const DataUse& other) const {
  return AreNonByteCountFieldsEqual(*this, other);
}

}  // namespace data_usage
