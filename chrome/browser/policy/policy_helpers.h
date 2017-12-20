// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_POLICY_HELPERS_H_
#define CHROME_BROWSER_POLICY_POLICY_HELPERS_H_

class GURL;

namespace policy {

// Returns true if |url| should never be blacklisted by policy.
bool OverrideBlacklistForURL(const GURL& url, bool* block, int* reason);

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_POLICY_HELPERS_H_
