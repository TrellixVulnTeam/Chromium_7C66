// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/feedback/system_logs/log_sources/chrome_internal_log_source.h"

#include "base/json/json_string_value_serializer.h"
#include "base/sys_info.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/sync/profile_sync_service_factory.h"
#include "chrome/common/channel_info.h"
#include "chrome/common/pref_names.h"
#include "components/browser_sync/profile_sync_service.h"
#include "components/prefs/pref_service.h"
#include "components/sync/driver/about_sync_util.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_set.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/metrics/chromeos_metrics_provider.h"
#include "chromeos/system/statistics_provider.h"
#endif

#if defined(OS_WIN)
#include "base/win/win_util.h"
#endif

namespace system_logs {

namespace {

constexpr char kSyncDataKey[] = "about_sync_data";
constexpr char kExtensionsListKey[] = "extensions";
constexpr char kDataReductionProxyKey[] = "data_reduction_proxy";
constexpr char kChromeVersionTag[] = "CHROME VERSION";
#if defined(OS_CHROMEOS)
constexpr char kChromeEnrollmentTag[] = "ENTERPRISE_ENROLLED";
constexpr char kHWIDKey[] = "HWID";
constexpr char kSettingsKey[] = "settings";
constexpr char kLocalStateSettingsResponseKey[] = "Local State: settings";
#else
constexpr char kOsVersionTag[] = "OS VERSION";
#endif
#if defined(OS_WIN)
constexpr char kUsbKeyboardDetected[] = "usb_keyboard_detected";
#endif

#if defined(OS_CHROMEOS)
std::string GetEnrollmentStatusString() {
  switch (ChromeOSMetricsProvider::GetEnrollmentStatus()) {
    case ChromeOSMetricsProvider::NON_MANAGED:
      return "Not managed";
    case ChromeOSMetricsProvider::MANAGED:
      return "Managed";
    case ChromeOSMetricsProvider::UNUSED:
    case ChromeOSMetricsProvider::ERROR_GETTING_ENROLLMENT_STATUS:
    case ChromeOSMetricsProvider::ENROLLMENT_STATUS_MAX:
      return "Error retrieving status";
  }
  // For compilers that don't recognize all cases handled above.
  NOTREACHED();
  return std::string();
}

void GetHWID(SystemLogsResponse* response) {
  DCHECK(response);

  chromeos::system::StatisticsProvider* stats =
      chromeos::system::StatisticsProvider::GetInstance();
  DCHECK(stats);

  std::string hwid;
  if (!stats->GetMachineStatistic(chromeos::system::kHardwareClassKey, &hwid)) {
    VLOG(1) << "Couldn't get machine statistic 'hardware_class'.";
    return;
  }

  (*response)[kHWIDKey] = hwid;
}
#endif

}  // namespace

ChromeInternalLogSource::ChromeInternalLogSource()
    : SystemLogsSource("ChromeInternal") {
}

ChromeInternalLogSource::~ChromeInternalLogSource() {
}

void ChromeInternalLogSource::Fetch(const SysLogsSourceCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(!callback.is_null());

  std::unique_ptr<SystemLogsResponse> response(new SystemLogsResponse());

  (*response)[kChromeVersionTag] = chrome::GetVersionString();

#if defined(OS_CHROMEOS)
  (*response)[kChromeEnrollmentTag] = GetEnrollmentStatusString();
#else
  // On ChromeOS, this will be pulled in from the LSB_RELEASE.
  std::string os_version = base::SysInfo::OperatingSystemName() + ": " +
                           base::SysInfo::OperatingSystemVersion();
  (*response)[kOsVersionTag] =  os_version;
#endif

  PopulateSyncLogs(response.get());
  PopulateExtensionInfoLogs(response.get());
  PopulateDataReductionProxyLogs(response.get());
#if defined(OS_WIN)
  PopulateUsbKeyboardDetected(response.get());
#endif

  if (ProfileManager::GetLastUsedProfile()->IsChild())
    (*response)["account_type"] = "child";

#if defined(OS_CHROMEOS)
  PopulateLocalStateSettings(response.get());

  // Get the HWID on the blocking pool and invoke the callback later when done.
  SystemLogsResponse* response_ptr = response.release();
  content::BrowserThread::PostBlockingPoolTaskAndReply(
      FROM_HERE, base::Bind(&GetHWID, response_ptr),
      base::Bind(callback, base::Owned(response_ptr)));
#else
  // On other platforms, we're done. Invoke the callback.
  callback.Run(response.get());
#endif  // defined(OS_CHROMEOS)
}

void ChromeInternalLogSource::PopulateSyncLogs(SystemLogsResponse* response) {
  // We are only interested in sync logs for the primary user profile.
  Profile* profile = ProfileManager::GetPrimaryUserProfile();
  if (!profile ||
      !ProfileSyncServiceFactory::GetInstance()->HasProfileSyncService(profile))
    return;

  browser_sync::ProfileSyncService* service =
      ProfileSyncServiceFactory::GetInstance()->GetForProfile(profile);
  std::unique_ptr<base::DictionaryValue> sync_logs(
      syncer::sync_ui_util::ConstructAboutInformation(
          service, service->signin(), chrome::GetChannel()));

  // Remove identity section.
  base::ListValue* details = NULL;
  sync_logs->GetList(syncer::sync_ui_util::kDetailsKey, &details);
  if (!details)
    return;
  for (base::ListValue::iterator it = details->begin();
      it != details->end(); ++it) {
    base::DictionaryValue* dict = NULL;
    if ((*it)->GetAsDictionary(&dict)) {
      std::string title;
      dict->GetString("title", &title);
      if (title == syncer::sync_ui_util::kIdentityTitle) {
        details->Erase(it, NULL);
        break;
      }
    }
  }

  // Add sync logs to logs.
  std::string sync_logs_string;
  JSONStringValueSerializer serializer(&sync_logs_string);
  serializer.Serialize(*sync_logs.get());

  (*response)[kSyncDataKey] = sync_logs_string;
}

void ChromeInternalLogSource::PopulateExtensionInfoLogs(
    SystemLogsResponse* response) {
  Profile* primary_profile =
      g_browser_process->profile_manager()->GetPrimaryUserProfile();
  if (!primary_profile)
    return;

  extensions::ExtensionRegistry* extension_registry =
      extensions::ExtensionRegistry::Get(primary_profile);
  std::string extensions_list;
  for (const scoped_refptr<const extensions::Extension>& extension :
       extension_registry->enabled_extensions()) {
    if (extensions_list.empty()) {
      extensions_list = extension->name();
    } else {
      extensions_list += ",\n" + extension->name();
    }
  }
  if (!extensions_list.empty())
    extensions_list += "\n";

  if (!extensions_list.empty())
    (*response)[kExtensionsListKey] = extensions_list;
}

void ChromeInternalLogSource::PopulateDataReductionProxyLogs(
    SystemLogsResponse* response) {
  PrefService* prefs = ProfileManager::GetActiveUserProfile()->GetPrefs();
  bool is_data_reduction_proxy_enabled =
      prefs->HasPrefPath(prefs::kDataSaverEnabled) &&
      prefs->GetBoolean(prefs::kDataSaverEnabled);
  (*response)[kDataReductionProxyKey] = is_data_reduction_proxy_enabled ?
      "enabled" : "disabled";
}

#if defined(OS_CHROMEOS)
void ChromeInternalLogSource::PopulateLocalStateSettings(
    SystemLogsResponse* response) {
  // Extract the "settings" entry in the local state and serialize back to
  // a string.
  std::unique_ptr<base::DictionaryValue> local_state =
      g_browser_process->local_state()->GetPreferenceValuesOmitDefaults();
  const base::DictionaryValue* local_state_settings = nullptr;
  if (!local_state->GetDictionary(kSettingsKey, &local_state_settings)) {
    VLOG(1) << "Failed to extract the settings entry from Local State.";
    return;
  }
  std::string serialized_settings;
  JSONStringValueSerializer serializer(&serialized_settings);
  if (!serializer.Serialize(*local_state_settings))
    return;

  (*response)[kLocalStateSettingsResponseKey] = serialized_settings;
}
#endif  // defined(OS_CHROMEOS)

#if defined(OS_WIN)
void ChromeInternalLogSource::PopulateUsbKeyboardDetected(
    SystemLogsResponse* response) {
  std::string reason;
  bool result = base::win::IsKeyboardPresentOnSlate(&reason);
  (*response)[kUsbKeyboardDetected] = result ? "Keyboard Detected:\n" :
                                               "No Keyboard:\n";
  (*response)[kUsbKeyboardDetected] += reason;
}
#endif

}  // namespace system_logs
