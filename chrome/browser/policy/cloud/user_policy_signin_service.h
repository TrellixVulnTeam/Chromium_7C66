// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_CLOUD_USER_POLICY_SIGNIN_SERVICE_H_
#define CHROME_BROWSER_POLICY_CLOUD_USER_POLICY_SIGNIN_SERVICE_H_

#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "chrome/browser/policy/cloud/user_policy_signin_service_base.h"
#include "google_apis/gaia/oauth2_token_service.h"

class Profile;
class ProfileOAuth2TokenService;

namespace net {
class URLRequestContextGetter;
}

namespace policy {

class CloudPolicyClientRegistrationHelper;

// A specialization of the UserPolicySigninServiceBase for the desktop
// platforms (Windows, Mac and Linux).
class UserPolicySigninService : public UserPolicySigninServiceBase,
                                public OAuth2TokenService::Observer {
 public:
  // Creates a UserPolicySigninService associated with the passed
  // |policy_manager| and |signin_manager|.
  UserPolicySigninService(
      Profile* profile,
      PrefService* local_state,
      DeviceManagementService* device_management_service,
      UserCloudPolicyManager* policy_manager,
      SigninManager* signin_manager,
      scoped_refptr<net::URLRequestContextGetter> system_request_context,
      ProfileOAuth2TokenService* oauth2_token_service);
  ~UserPolicySigninService() override;

  // Registers a CloudPolicyClient for fetching policy for a user. The
  // |oauth2_login_token| and |username| are explicitly passed because
  // the user is not signed in yet (ProfileOAuth2TokenService does not have
  // any tokens yet to prevent services from using it until after we've fetched
  // policy).
  void RegisterForPolicy(const std::string& username,
                         const std::string& oauth2_login_token,
                         const PolicyRegistrationCallback& callback);

  // OAuth2TokenService::Observer implementation:
  void OnRefreshTokenAvailable(const std::string& account_id) override;

  // CloudPolicyService::Observer implementation:
  void OnInitializationCompleted(CloudPolicyService* service) override;

  // KeyedService implementation:
  void Shutdown() override;

 protected:
  // UserPolicySigninServiceBase implementation:
  void InitializeUserCloudPolicyManager(
      const std::string& username,
      std::unique_ptr<CloudPolicyClient> client) override;

  void PrepareForUserCloudPolicyManagerShutdown() override;
  void ShutdownUserCloudPolicyManager() override;

 private:
  // Fetches an OAuth token to allow the cloud policy service to register with
  // the cloud policy server. |oauth_login_token| should contain an OAuth login
  // refresh token that can be downscoped to get an access token for the
  // device_management service.
  void RegisterCloudPolicyService();

  // Callback invoked when policy registration has finished.
  void OnRegistrationComplete();

  // Helper routine which prohibits user signout if the user is registered for
  // cloud policy.
  void ProhibitSignoutIfNeeded();

  // Invoked when a policy registration request is complete.
  void CallPolicyRegistrationCallback(std::unique_ptr<CloudPolicyClient> client,
                                      PolicyRegistrationCallback callback);

  // Parent profile for this service.
  Profile* profile_;

  std::unique_ptr<CloudPolicyClientRegistrationHelper> registration_helper_;

  // Weak pointer to the token service we use to authenticate during
  // CloudPolicyClient registration.
  ProfileOAuth2TokenService* oauth2_token_service_;

  DISALLOW_COPY_AND_ASSIGN(UserPolicySigninService);
};

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_CLOUD_USER_POLICY_SIGNIN_SERVICE_H_
