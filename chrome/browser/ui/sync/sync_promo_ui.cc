// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/sync/sync_promo_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/signin_promo_util.h"

bool SyncPromoUI::ShouldShowSyncPromo(Profile* profile) {
  // Don't show sync promo if the sign in promo should not be shown.
  if (!signin::ShouldShowPromo(profile)) {
    return false;
  }

  // Don't show if sync is not allowed to start.
  if (!profile->IsSyncAllowed())
    return false;

  return true;
}
