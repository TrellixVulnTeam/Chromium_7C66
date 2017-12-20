// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file implements the tamper detection logic, which detects whether
// there are middleboxes and whether they are tampering with the response
// which may break correct communication and data transfer between the Chromium
// client and the Data Reduction Proxy.
//
// At a high level, the tamper detection process works in two steps:
// 1. The Data Reduction Proxy selects a fraction of responses to analyze,
//    generates a series of fingerprints for each, and appends them to the
//    Chrome-Proxy response headers;
// 2. The client re-generate the fingerprints using the same method as the
//    proxy, compares them to the fingerprints in the response, and generates
//    UMA. A response is considered to have been tampered with if the
//    fingerprints do not match.
//
// Four fingerprints are generated by the Data Reduction Proxy:
// 1. Fingerprint of the Chrome-Proxy header, which is designed to check
//    whether the Chrome-Proxy header has been modified or not;
// 2. Fingerprint of the Via header, which is designed to check whether there
//    are middleboxes between the Chromium client and the Data Reduction Proxy;
// 3. Fingerprint of a list of headers, which is designed to check whether the
//    values of a list of headers (list is defined by the Data Reduction Proxy)
//    have been modified or deleted;
// 4. Fingerprint of the Content-Length header, which is designed to check
//    whether the response body has been modified or not (the code assumes that
//    different Content-Length values indicate different response bodies).
//
// On the client side, the fingerprint of the Chrome-Proxy header will be
// checked first. If the fingerprint indicates that the Chrome-Proxy header has
// not been modified, then the other fingerprints will be considered to be
// reliable and will be checked next; if not, then it's possible that the other
// fingerprints have been tampered with and thus they will not be checked.
// If middlebox removes all the fingerprints then such tampering will not be
// detected.
//
// Detected tampering information will be reported to UMA. In general, for each
// fingerprint, the client reports the number of responses that have been
// tampered with for different carriers. For the fingerprint of the
// Content-Length header, which indicates whether the response body has been
// modified or not, the reports of tampering are separated by MIME type of the
// response body.

#ifndef COMPONENTS_DATA_REDUCTION_PROXY_CORE_BROWSER_DATA_REDUCTION_PROXY_TAMPER_DETECTION_H_
#define COMPONENTS_DATA_REDUCTION_PROXY_CORE_BROWSER_DATA_REDUCTION_PROXY_TAMPER_DETECTION_H_

#include <map>
#include <string>
#include <vector>

#include <stdint.h>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "net/proxy/proxy_service.h"

namespace net {
class HttpResponseHeaders;
}

namespace data_reduction_proxy {

// Detects if the response sent by the Data Reduction Proxy has been modified
// by intermediaries on the Web.
class DataReductionProxyTamperDetection {
 public:
  // Checks if the response contains tamper detection fingerprints added by the
  // Data Reduction Proxy, and determines if the response had been tampered
  // with if so. |content_length| is the accurate response body length, it will
  // be used to detect whether response body had been tampered with. Results
  // are reported to UMA. Traffic carried by HTTP proxy and HTTPS proxy are
  // reported separately, specified by |scheme_is_https|. Returns true if the
  // response had been tampered with.
  static bool DetectAndReport(const net::HttpResponseHeaders* headers,
                              bool scheme_is_https,
                              int64_t content_length);

  // Tamper detection checks |response_headers|. Histogram events are reported
  // by |carrier_id|; |scheme_is_https| determines which histogram to report
  // (HTTP and HTTPS are reported separately).
  DataReductionProxyTamperDetection(
      const net::HttpResponseHeaders* response_headers,
      bool scheme_is_https,
      unsigned carrier_id);

  virtual ~DataReductionProxyTamperDetection();

 private:
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           TestFingerprintCommon);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           ChromeProxy);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           Via);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           OtherHeaders);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           ContentLength);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           HeaderRemoving);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           ValuesToSortedString);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           GetHeaderValues);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           HistogramCount);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           DetectAndReport);
  FRIEND_TEST_ALL_PREFIXES(DataReductionProxyTamperDetectionTest,
                           CompressionRatio);

  // Reports UMA for the numbers of responses with valid fingerprints, separated
  // by MIME type.
  void ReportUMAForTamperDetectionCount(int64_t original_content_length) const;

  // Returns the result of validating Chrome-Proxy header.
  bool ValidateChromeProxyHeader(base::StringPiece fingerprint) const;

  // Reports UMA for tampering of the Chrome-Proxy header.
  void ReportUMAForChromeProxyHeaderValidation() const;

  // Returns the result of validating the Via header.
  // |has_chrome_proxy_via_header| indicates that the Data Reduction Proxy's
  // Via header occurs or not.
  bool ValidateViaHeader(base::StringPiece fingerprint,
                         bool* has_chrome_proxy_via_header) const;

  // Reports UMA for tampering of the Via header.
  void ReportUMAForViaHeaderValidation(bool has_chrome_proxy_via_header) const;

  // Returns the result of validating a list of headers.
  bool ValidateOtherHeaders(const std::string& fingerprint) const;

  // Reports UMA for tampering of values of the list of headers.
  void ReportUMAForOtherHeadersValidation() const;

  // Returns the result of validating the contents. It validates this by
  // comparing the content length sent by the Data Reduction Proxy (from the
  // fingerprint), to the actual |content_length| received by the Chromium
  // client. The content length sent by the Data Reduction Proxy is retuned as
  // |original_content_length| for future use, |original_content_length| cannot
  // be NULL.
  bool ValidateContentLength(base::StringPiece fingerprint,
                             int64_t received_content_length,
                             int64_t* original_content_length) const;

  // Reports UMA for tampering of the contents and the compression ratio. The
  // compression ratio is calculated from |content_length|, which is the
  // content length received by the Chromium client, and
  // |original_content_length|, which is the content length sent by the Data
  // Reduction Proxy.
  void ReportUMAForContentLength(int64_t content_length,
                                 int64_t original_content_length) const;

  // Returns a string representation of |values| in sorted order separated by
  // commas, including a trailing comma at the end of the string.
  static std::string ValuesToSortedString(std::vector<std::string>* values);

  // Returns raw MD5 hash value for a given string |input|. It is different to
  // base::MD5String which is base16 encoded.
  static void GetMD5(base::StringPiece input, std::string* output);

  // Returns all the values of |header_name| of the response |headers| as a
  // vector. This function is used for values that need to be sorted later.
  static std::vector<std::string> GetHeaderValues(
      const net::HttpResponseHeaders* headers,
      base::StringPiece header_name);

  // Pointer to response headers.
  const net::HttpResponseHeaders* response_headers_;

  // If true, the connection to the Data Reduction Proxy is over HTTPS;
  const bool scheme_is_https_;

  // Carrier ID: the numeric name of the current registered operator.
  const unsigned carrier_id_;

  DISALLOW_COPY_AND_ASSIGN(DataReductionProxyTamperDetection);
};

}  // namespace data_reduction_proxy
#endif  // COMPONENTS_DATA_REDUCTION_PROXY_CORE_BROWSER_DATA_REDUCTION_PROXY_TAMPER_DETECTION_H_
