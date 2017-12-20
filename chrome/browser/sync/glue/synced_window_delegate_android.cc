// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/glue/synced_window_delegate_android.h"

#include "chrome/browser/android/tab_android.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sync/glue/synced_tab_delegate_android.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "content/public/browser/web_contents.h"

using sync_sessions::SyncedTabDelegate;

namespace browser_sync {

// SyncedWindowDelegateAndroid implementations

SyncedWindowDelegateAndroid::SyncedWindowDelegateAndroid(
    TabModel* tab_model,
    bool is_tabbed_activity)
    : tab_model_(tab_model), is_tabbed_activity_(is_tabbed_activity) {}

SyncedWindowDelegateAndroid::~SyncedWindowDelegateAndroid() {}

bool SyncedWindowDelegateAndroid::HasWindow() const {
  return !tab_model_->IsOffTheRecord();
}

SessionID::id_type SyncedWindowDelegateAndroid::GetSessionId() const {
  return tab_model_->GetSessionId();
}

int SyncedWindowDelegateAndroid::GetTabCount() const {
  return tab_model_->GetTabCount();
}

int SyncedWindowDelegateAndroid::GetActiveIndex() const {
  return tab_model_->GetActiveIndex();
}

bool SyncedWindowDelegateAndroid::IsApp() const {
  return false;
}

bool SyncedWindowDelegateAndroid::IsTypeTabbed() const {
  return is_tabbed_activity_;
}

bool SyncedWindowDelegateAndroid::IsTypePopup() const {
  return false;
}

bool SyncedWindowDelegateAndroid::IsTabPinned(
    const SyncedTabDelegate* tab) const {
  return false;
}

SyncedTabDelegate* SyncedWindowDelegateAndroid::GetTabAt(int index) const {
  // After a restart, it is possible for the Tab to be null during startup.
  TabAndroid* tab = tab_model_->GetTabAt(index);
  return tab ? tab->GetSyncedTabDelegate() : nullptr;
}

SessionID::id_type SyncedWindowDelegateAndroid::GetTabIdAt(int index) const {
  SyncedTabDelegate* tab = GetTabAt(index);
  return tab ? tab->GetSessionId() : -1;
}

bool SyncedWindowDelegateAndroid::IsSessionRestoreInProgress() const {
  return tab_model_->IsSessionRestoreInProgress();
}

bool SyncedWindowDelegateAndroid::ShouldSync() const {
  return true;
}

}  // namespace browser_sync
