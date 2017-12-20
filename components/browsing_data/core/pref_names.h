// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSING_DATA_CORE_PREF_NAMES_H_
#define COMPONENTS_BROWSING_DATA_CORE_PREF_NAMES_H_

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace browsing_data {

namespace prefs {

extern const char kDeleteTimePeriod[];

extern const char kDeleteBrowsingHistory[];
extern const char kDeleteDownloadHistory[];
extern const char kDeleteCache[];
extern const char kDeleteCookies[];
extern const char kDeletePasswords[];
extern const char kDeleteFormData[];
extern const char kDeleteHostedAppsData[];
extern const char kDeleteMediaLicenses[];

extern const char kLastClearBrowsingDataTime[];
extern const char kClearBrowsingDataHistoryNoticeShownTimes[];

// Registers the Clear Browsing Data UI prefs.
void RegisterBrowserUserPrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace prefs

}  // namespace browsing_data

#endif  // COMPONENTS_BROWSING_DATA_CORE_PREF_NAMES_H_
