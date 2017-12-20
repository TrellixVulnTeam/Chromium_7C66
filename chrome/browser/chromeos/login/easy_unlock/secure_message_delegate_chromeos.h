// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_EASY_UNLOCK_SECURE_MESSAGE_DELEGATE_CHROMEOS_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_EASY_UNLOCK_SECURE_MESSAGE_DELEGATE_CHROMEOS_H_

#include "base/macros.h"
#include "components/proximity_auth/cryptauth/secure_message_delegate.h"

namespace chromeos {

class EasyUnlockClient;

// SecureMessageDelegate implementation for ChromeOS.
class SecureMessageDelegateChromeOS
    : public proximity_auth::SecureMessageDelegate {
 public:
  SecureMessageDelegateChromeOS();
  ~SecureMessageDelegateChromeOS() override;

  // SecureMessageDelegate:
  void GenerateKeyPair(const GenerateKeyPairCallback& callback) override;
  void DeriveKey(const std::string& private_key,
                 const std::string& public_key,
                 const DeriveKeyCallback& callback) override;
  void CreateSecureMessage(
      const std::string& payload,
      const std::string& key,
      const CreateOptions& create_options,
      const CreateSecureMessageCallback& callback) override;
  void UnwrapSecureMessage(
      const std::string& serialized_message,
      const std::string& key,
      const UnwrapOptions& unwrap_options,
      const UnwrapSecureMessageCallback& callback) override;

 private:
  chromeos::EasyUnlockClient* dbus_client_;

  DISALLOW_COPY_AND_ASSIGN(SecureMessageDelegateChromeOS);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_EASY_UNLOCK_SECURE_MESSAGE_DELEGATE_CHROMEOS_H_
