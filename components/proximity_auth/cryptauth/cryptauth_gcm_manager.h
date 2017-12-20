// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_CRYPTAUTH_GCM_MANAGER_H
#define COMPONENTS_PROXIMITY_AUTH_CRYPTAUTH_CRYPTAUTH_GCM_MANAGER_H

#include <string>

class PrefRegistrySimple;

namespace proximity_auth {

// Interface for the manager controlling GCM registrations and handling GCM push
// messages for CryptAuth. CryptAuth sends GCM messages to request the local
// device to re-enroll to get the freshest device state, and to notify the
// local device to resync the remote device list when this list changes.
class CryptAuthGCMManager {
 public:
  class Observer {
   public:
    virtual ~Observer();

    // Called when a gcm registration attempt finishes with the |success| of the
    // attempt.
    virtual void OnGCMRegistrationResult(bool success);

    // Called when a GCM message is received to re-enroll the device with
    // CryptAuth.
    virtual void OnReenrollMessage();

    // Called when a GCM message is received to sync down new devices from
    // CryptAuth.
    virtual void OnResyncMessage();
  };

  virtual ~CryptAuthGCMManager() {}

  // Registers the prefs used by the manager to the given |pref_service|.
  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Starts listening to incoming GCM messages. If GCM registration is completed
  // after this function is called, then messages will also be handled properly.
  virtual void StartListening() = 0;

  // Begins registration with GCM. The Observer::OnGCMRegistrationResult()
  // observer function will be called when registration completes.
  virtual void RegisterWithGCM() = 0;

  // Returns the GCM registration id received from the last successful
  // registration. If registration has not been performed, then an empty string
  // will be returned.
  virtual std::string GetRegistrationId() = 0;

  // Adds an observer.
  virtual void AddObserver(Observer* observer) = 0;

  // Removes an observer.
  virtual void RemoveObserver(Observer* observer) = 0;
};

}  // namespace proximity_auth

#endif  // COMPONENTS_PROXIMITY_CRYPTAUTH_CRYPTAUTH_GCM_MANAGER_H
