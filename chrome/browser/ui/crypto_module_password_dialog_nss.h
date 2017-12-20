// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_CRYPTO_MODULE_PASSWORD_DIALOG_NSS_H_
#define CHROME_BROWSER_UI_CRYPTO_MODULE_PASSWORD_DIALOG_NSS_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "chrome/browser/ui/crypto_module_password_dialog.h"
#include "net/base/host_port_pair.h"
#include "ui/gfx/native_widget_types.h"

namespace net {
class CryptoModule;
typedef std::vector<scoped_refptr<CryptoModule> > CryptoModuleList;
class X509Certificate;
}

namespace chrome {

// Asynchronously unlock |modules|, if necessary. |callback| is called when
// done (regardless if any modules were successfully unlocked or not).  Should
// only be called on UI thread.
void UnlockSlotsIfNecessary(const net::CryptoModuleList& modules,
                            CryptoModulePasswordReason reason,
                            const net::HostPortPair& server,
                            gfx::NativeWindow parent,
                            const base::Closure& callback);

// Asynchronously unlock the |cert|'s module, if necessary. |callback| is
// called when done (regardless if module was successfully unlocked or not).
// Should only be called on UI thread.
void UnlockCertSlotIfNecessary(net::X509Certificate* cert,
                               CryptoModulePasswordReason reason,
                               const net::HostPortPair& server,
                               gfx::NativeWindow parent,
                               const base::Closure& callback);

}  // namespace chrome

#endif  // CHROME_BROWSER_UI_CRYPTO_MODULE_PASSWORD_DIALOG_NSS_H_
