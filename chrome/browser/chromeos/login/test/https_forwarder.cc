// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/test/https_forwarder.h"

#include <cstring>

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/path_service.h"
#include "base/values.h"
#include "net/cert/test_root_certs.h"
#include "net/cert/x509_certificate.h"
#include "net/test/python_utils.h"
#include "net/test/spawned_test_server/base_test_server.h"
#include "net/test/spawned_test_server/local_test_server.h"
#include "url/third_party/mozilla/url_parse.h"
#include "url/url_canon.h"

namespace chromeos {

namespace {

// The root certificate used by net/tools/testserver/minica.py.
const char kMinicaRootCert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB0TCCATqgAwIBAgIBATANBgkqhkiG9w0BAQUFADAVMRMwEQYDVQQDEwpUZXN0aW5nIENBMB"
    "4XDTEwMDEwMTA2MDAwMFoXDTMyMTIwMTA2MDAwMFowFTETMBEGA1UEAxMKVGVzdGluZyBDQTCB"
    "nTANBgkqhkiG9w0BAQEFAAOBiwAwgYcCgYEApxmY8pML/nPQMah/Ez0vN47u7tUqd+RND8n/bw"
    "f/Msvz2pmd5O1lgyr8sIB/mHh1BlOdJYoM48LHeWdlMJmpA0qbEVqHbDmoxOTtSs0MZAlZRvs5"
    "7utHoHBNuwGKz0jDocS4lfxAn7SjQKmGsa/EVRmrnspHwwGFx3HGSqXs8H0CAQOjMzAxMBIGA1"
    "UdEwEB/wQIMAYBAf8CAQAwGwYDVR0gAQEABBEwDzANBgsrBgEEAdZ5AgHODzANBgkqhkiG9w0B"
    "AQUFAAOBgQA/STb40A6D+93jMfLGQzXc997IsaJZdoPt7tYa8PqGJBL62EiTj+erd/H5pDZx/2"
    "/bcpOG4m9J56ygwOohbllw2TM+oeEd8syzV6X+1SIPnGI56JRrm3UXcHYx1Rq5loM9WKAiz/Wm"
    "IWmskljsEQ7+542pq0pkHjs8nuXovSkUYA==\n"
    "-----END CERTIFICATE-----";

}  // namespace

// A net::LocalTestServer that handles the actual forwarding to another server.
// Requires that the root certificate used by minica.py be marked as trusted
// before it is used.
class ForwardingServer : public net::LocalTestServer {
 public:
  ForwardingServer(const std::string& ssl_host, const GURL& forward_target);

  // net::LocalTestServer:
  bool SetPythonPath() const override;
  bool GetTestServerPath(base::FilePath* testserver_path) const override;
  bool GenerateAdditionalArguments(
      base::DictionaryValue* arguments) const override;

 private:
  const std::string ssl_host_;
  const GURL forward_target_;

  DISALLOW_COPY_AND_ASSIGN(ForwardingServer);
};

ForwardingServer::ForwardingServer(const std::string& ssl_host,
                                   const GURL& forward_target)
    : net::LocalTestServer(net::LocalTestServer::TYPE_HTTPS,
                           SSLOptions(SSLOptions::CERT_AUTO),
                           base::FilePath()),
      ssl_host_(ssl_host),
      forward_target_(forward_target) {
}

bool ForwardingServer::SetPythonPath() const {
  if (!net::LocalTestServer::SetPythonPath())
    return false;

  base::FilePath net_testserver_path;
  if (!LocalTestServer::GetTestServerPath(&net_testserver_path))
    return false;
  AppendToPythonPath(net_testserver_path.DirName());

  return true;
}

bool ForwardingServer::GetTestServerPath(
    base::FilePath* testserver_path) const {
  base::FilePath source_root_dir;
  if (!PathService::Get(base::DIR_SOURCE_ROOT, &source_root_dir))
    return false;

  *testserver_path = source_root_dir.Append("chrome")
                                    .Append("browser")
                                    .Append("chromeos")
                                    .Append("login")
                                    .Append("test")
                                    .Append("https_forwarder.py");
  return true;
}

bool ForwardingServer::GenerateAdditionalArguments(
    base::DictionaryValue* arguments) const {
  base::FilePath source_root_dir;
  if (!net::LocalTestServer::GenerateAdditionalArguments(arguments) ||
      !PathService::Get(base::DIR_SOURCE_ROOT, &source_root_dir))
    return false;

  arguments->SetString("ssl-host", ssl_host_);
  arguments->SetString("forward-target", forward_target_.spec());

  return true;
}

HTTPSForwarder::HTTPSForwarder() {
}

HTTPSForwarder::~HTTPSForwarder() {
}

GURL HTTPSForwarder::GetURLForSSLHost(const std::string& path) const {
  CHECK(forwarding_server_);
  url::Replacements<char> replacements;
  replacements.SetHost(ssl_host_.c_str(), url::Component(0, ssl_host_.size()));
  return forwarding_server_->GetURL(path).ReplaceComponents(replacements);
}

bool HTTPSForwarder::Initialize(const std::string& ssl_host,
                                const GURL& forward_target) {
  // Mark the root certificate used by minica.py as trusted. This will be used
  // by the Python part of the HTTPSForwarder to generate a certificate for
  // |ssl_host_|.
  net::TestRootCerts* root_certs = net::TestRootCerts::GetInstance();
  if (!root_certs)
    return false;
  net::CertificateList certs =
      net::X509Certificate::CreateCertificateListFromBytes(
          kMinicaRootCert, strlen(kMinicaRootCert),
          net::X509Certificate::FORMAT_AUTO);
  if (certs.size() != 1)
    return false;
  root_certs->Add(certs.front().get());

  ssl_host_ = ssl_host;
  forwarding_server_.reset(new ForwardingServer(ssl_host, forward_target));
  return forwarding_server_->Start();
}

}  // namespace chromeos
