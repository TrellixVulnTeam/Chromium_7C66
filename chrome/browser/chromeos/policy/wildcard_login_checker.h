// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_WILDCARD_LOGIN_CHECKER_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_WILDCARD_LOGIN_CHECKER_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "components/policy/core/common/cloud/user_info_fetcher.h"
#include "google_apis/gaia/google_service_auth_error.h"

namespace net {
class URLRequestContextGetter;
}

namespace policy {

class PolicyOAuth2TokenFetcher;

// Performs online verification whether wildcard login is allowed, i.e. whether
// the user is a hosted user. This class performs an asynchronous check and
// reports the result via a callback.
class WildcardLoginChecker : public UserInfoFetcher::Delegate {
 public:
  // Indicates result of the wildcard login check.
  enum Result {
    RESULT_ALLOWED,  // Wildcard check succeeded, login allowed.
    RESULT_BLOCKED,  // Check completed, but user should be blocked.
    RESULT_FAILED,   // Failure due to network errors etc.
  };

  typedef base::Callback<void(Result)> StatusCallback;

  WildcardLoginChecker();
  virtual ~WildcardLoginChecker();

  // Starts checking. The result will be reported via |callback_|.
  void StartWithSigninContext(
      scoped_refptr<net::URLRequestContextGetter> signin_context,
      const StatusCallback& callback);

  // Starts checking with a provided refresh token.
  void StartWithRefreshToken(const std::string& refresh_token,
                             const StatusCallback& callback);

  // Starts checking with a provided access token.
  void StartWithAccessToken(const std::string& access_token,
                            const StatusCallback& callback);

  // UserInfoFetcher::Delegate:
  void OnGetUserInfoSuccess(const base::DictionaryValue* response) override;
  void OnGetUserInfoFailure(const GoogleServiceAuthError& error) override;

 private:
  // Starts the check after successful token minting.
  void OnPolicyTokenFetched(const std::string& access_token,
                            const GoogleServiceAuthError& error);

  // Starts the user info fetcher.
  void StartUserInfoFetcher(const std::string& access_token);

  // Handles the response of the check and calls ReportResult().
  void OnCheckCompleted(Result result);

  StatusCallback callback_;

  std::unique_ptr<PolicyOAuth2TokenFetcher> token_fetcher_;
  std::unique_ptr<UserInfoFetcher> user_info_fetcher_;

  base::Time start_timestamp_;
  base::Time token_available_timestamp_;

  DISALLOW_COPY_AND_ASSIGN(WildcardLoginChecker);
};

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_WILDCARD_LOGIN_CHECKER_H_
