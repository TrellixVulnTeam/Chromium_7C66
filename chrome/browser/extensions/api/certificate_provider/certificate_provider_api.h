// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_CERTIFICATE_PROVIDER_CERTIFICATE_PROVIDER_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_CERTIFICATE_PROVIDER_CERTIFICATE_PROVIDER_API_H_

#include "extensions/browser/extension_function.h"

namespace chromeos {
namespace certificate_provider {
struct CertificateInfo;
}
}

namespace extensions {

namespace api {
namespace certificate_provider {
struct CertificateInfo;
}
}

class CertificateProviderInternalReportCertificatesFunction
    : public UIThreadExtensionFunction {
 private:
  ~CertificateProviderInternalReportCertificatesFunction() override;
  ResponseAction Run() override;

  bool ParseCertificateInfo(
      const api::certificate_provider::CertificateInfo& info,
      chromeos::certificate_provider::CertificateInfo* out_info);

  DECLARE_EXTENSION_FUNCTION("certificateProviderInternal.reportCertificates",
                             CERTIFICATEPROVIDERINTERNAL_REPORTCERTIFICATES);
};

class CertificateProviderInternalReportSignatureFunction
    : public UIThreadExtensionFunction {
 private:
  ~CertificateProviderInternalReportSignatureFunction() override;
  ResponseAction Run() override;

  DECLARE_EXTENSION_FUNCTION("certificateProviderInternal.reportSignature",
                             CERTIFICATEPROVIDERINTERNAL_REPORTSIGNATURE);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_CERTIFICATE_PROVIDER_CERTIFICATE_PROVIDER_API_H_
