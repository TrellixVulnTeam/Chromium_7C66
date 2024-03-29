// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/settings/native_certificates_handler.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "chrome/browser/ui/webui/settings_utils.h"
#include "content/public/browser/user_metrics.h"
#include "content/public/browser/web_ui.h"

namespace settings {

NativeCertificatesHandler::NativeCertificatesHandler() {}

NativeCertificatesHandler::~NativeCertificatesHandler() {}

void NativeCertificatesHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "showManageSSLCertificates",
      base::Bind(&NativeCertificatesHandler::HandleShowManageSSLCertificates,
                 base::Unretained(this)));
}

void NativeCertificatesHandler::HandleShowManageSSLCertificates(
    const base::ListValue* args) {
  content::RecordAction(
      base::UserMetricsAction("Options_ManageSSLCertificates"));
  settings_utils::ShowManageSSLCertificates(web_ui()->GetWebContents());
}

}  // namespace settings
