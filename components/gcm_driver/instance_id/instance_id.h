// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_GCM_DRIVER_INSTANCE_ID_INSTANCE_ID_H_
#define COMPONENTS_GCM_DRIVER_INSTANCE_ID_INSTANCE_ID_H_

#include <map>
#include <memory>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"

namespace gcm {
class GCMDriver;
}  // namespace gcm

namespace instance_id {

extern const char kGCMScope[];

// Encapsulates Instance ID functionalities that need to be implemented for
// different platforms. One instance is created per application. Life of
// Instance ID is managed by the InstanceIDDriver.
class InstanceID {
 public:
  enum Result {
    // Successful operation.
    SUCCESS,
    // Invalid parameter.
    INVALID_PARAMETER,
    // Instance ID is disabled.
    DISABLED,
    // Previous asynchronous operation is still pending to finish.
    ASYNC_OPERATION_PENDING,
    // Network socket error.
    NETWORK_ERROR,
    // Problem at the server.
    SERVER_ERROR,
    // Other errors.
    UNKNOWN_ERROR
  };

  // Asynchronous callbacks. Must not synchronously delete |this| (using
  // InstanceIDDriver::RemoveInstanceID).
  typedef base::Callback<void(const std::string& app_id,
                              bool update_id)> TokenRefreshCallback;
  typedef base::Callback<void(const std::string& id)> GetIDCallback;
  typedef base::Callback<void(const base::Time& creation_time)>
      GetCreationTimeCallback;
  typedef base::Callback<void(const std::string& token,
                              Result result)> GetTokenCallback;
  typedef base::Callback<void(const std::string&, const std::string&)>
      GetEncryptionInfoCallback;
  typedef base::Callback<void(Result result)> DeleteTokenCallback;
  typedef base::Callback<void(Result result)> DeleteIDCallback;

  static const int kInstanceIDByteLength = 8;

  // Creator. Should only be used by InstanceIDDriver::GetInstanceID.
  // |app_id|: identifies the application that uses the Instance ID.
  // |handler|: provides the GCM functionality needed to support Instance ID.
  //            Must outlive this class. On Android, this can be null instead.
  static std::unique_ptr<InstanceID> CreateInternal(const std::string& app_id,
                                                    gcm::GCMDriver* gcm_driver);

  virtual ~InstanceID();

  // Sets the callback that will be invoked when the token refresh event needs
  // to be triggered.
  void SetTokenRefreshCallback(const TokenRefreshCallback& callback);

  // Returns the Instance ID.
  virtual void GetID(const GetIDCallback& callback) = 0;

  // Returns the time when the InstanceID has been generated.
  virtual void GetCreationTime(const GetCreationTimeCallback& callback) = 0;

  // Retrieves a token that allows the authorized entity to access the service
  // defined as "scope".
  // |authorized_entity|: identifies the entity that is authorized to access
  //                      resources associated with this Instance ID. It can be
  //                      another Instance ID or a project ID.
  // |scope|: identifies authorized actions that the authorized entity can take.
  //          E.g. for sending GCM messages, "GCM" scope should be used.
  // |options|: allows including a small number of string key/value pairs that
  //            will be associated with the token and may be used in processing
  //            the request.
  // |callback|: to be called once the asynchronous operation is done.
  virtual void GetToken(const std::string& authorized_entity,
                        const std::string& scope,
                        const std::map<std::string, std::string>& options,
                        const GetTokenCallback& callback) = 0;

  // Get the public encryption key and authentication secret associated with a
  // GCM-scoped token. If encryption info is not yet associated, it will be
  // created.
  // |authorized_entity|: the authorized entity passed when obtaining the token.
  // |callback|: to be called once the asynchronous operation is done.
  void GetEncryptionInfo(const std::string& authorized_entity,
                         const GetEncryptionInfoCallback& callback);

  // Revokes a granted token.
  // |authorized_entity|: the authorized entity passed when obtaining the token.
  // |scope|: the scope that was passed when obtaining the token.
  // |callback|: to be called once the asynchronous operation is done.
  void DeleteToken(const std::string& authorized_entity,
                   const std::string& scope,
                   const DeleteTokenCallback& callback);

  // Resets the app instance identifier and revokes all tokens associated with
  // it.
  // |callback|: to be called once the asynchronous operation is done.
  void DeleteID(const DeleteIDCallback& callback);

  std::string app_id() const { return app_id_; }

 protected:
  InstanceID(const std::string& app_id, gcm::GCMDriver* gcm_driver);

  // Platform-specific implementations.
  virtual void DeleteTokenImpl(const std::string& authorized_entity,
                               const std::string& scope,
                               const DeleteTokenCallback& callback) = 0;
  virtual void DeleteIDImpl(const DeleteIDCallback& callback) = 0;

  void NotifyTokenRefresh(bool update_id);

  gcm::GCMDriver* gcm_driver() { return gcm_driver_; }

 private:
  void DidDelete(const std::string& authorized_entity,
                 const base::Callback<void(Result result)>& callback,
                 Result result);

  // Owned by GCMProfileServiceFactory, which is a dependency of
  // InstanceIDProfileServiceFactory, which owns this.
  gcm::GCMDriver* gcm_driver_;

  std::string app_id_;
  TokenRefreshCallback token_refresh_callback_;

  base::WeakPtrFactory<InstanceID> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(InstanceID);
};

}  // namespace instance_id

#endif  // COMPONENTS_GCM_DRIVER_INSTANCE_ID_INSTANCE_ID_H_
