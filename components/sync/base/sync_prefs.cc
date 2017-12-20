// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/base/sync_prefs.h"

#include "base/base64.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "components/sync/base/pref_names.h"

namespace syncer {

SyncPrefObserver::~SyncPrefObserver() {}

SyncPrefs::SyncPrefs(PrefService* pref_service) : pref_service_(pref_service) {
  DCHECK(pref_service);
  RegisterPrefGroups();
  // Watch the preference that indicates sync is managed so we can take
  // appropriate action.
  pref_sync_managed_.Init(
      prefs::kSyncManaged, pref_service_,
      base::Bind(&SyncPrefs::OnSyncManagedPrefChanged, base::Unretained(this)));
}

SyncPrefs::SyncPrefs() : pref_service_(nullptr) {}

SyncPrefs::~SyncPrefs() {
  DCHECK(CalledOnValidThread());
}

// static
void SyncPrefs::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(prefs::kSyncFirstSetupComplete, false);
  registry->RegisterBooleanPref(prefs::kSyncSuppressStart, false);
  registry->RegisterInt64Pref(prefs::kSyncLastSyncedTime, 0);
  registry->RegisterInt64Pref(prefs::kSyncLastPollTime, 0);
  registry->RegisterInt64Pref(prefs::kSyncFirstSyncTime, 0);

  // All datatypes are on by default, but this gets set explicitly
  // when you configure sync (when turning it on), in
  // ProfileSyncService::OnUserChoseDatatypes.
  registry->RegisterBooleanPref(prefs::kSyncKeepEverythingSynced, true);

  ModelTypeSet user_types = UserTypes();

  // Include proxy types as well, as they can be individually selected,
  // although they don't have sync representations.
  user_types.PutAll(ProxyTypes());

  // Treat bookmarks and device info specially.
  RegisterDataTypePreferredPref(registry, BOOKMARKS, true);
  RegisterDataTypePreferredPref(registry, DEVICE_INFO, true);
  user_types.Remove(BOOKMARKS);
  user_types.Remove(DEVICE_INFO);

  // All types are set to off by default, which forces a configuration to
  // explicitly enable them. GetPreferredTypes() will ensure that any new
  // implicit types are enabled when their pref group is, or via
  // KeepEverythingSynced.
  for (ModelTypeSet::Iterator it = user_types.First(); it.Good(); it.Inc()) {
    RegisterDataTypePreferredPref(registry, it.Get(), false);
  }

  registry->RegisterBooleanPref(prefs::kSyncManaged, false);
  registry->RegisterStringPref(prefs::kSyncEncryptionBootstrapToken,
                               std::string());
  registry->RegisterStringPref(prefs::kSyncKeystoreEncryptionBootstrapToken,
                               std::string());
#if defined(OS_CHROMEOS)
  registry->RegisterStringPref(prefs::kSyncSpareBootstrapToken, "");
#endif

  registry->RegisterBooleanPref(prefs::kSyncHasAuthError, false);
  registry->RegisterStringPref(prefs::kSyncSessionsGUID, std::string());
  registry->RegisterBooleanPref(prefs::kSyncPassphrasePrompted, false);
  registry->RegisterIntegerPref(prefs::kSyncMemoryPressureWarningCount, -1);
  registry->RegisterBooleanPref(prefs::kSyncShutdownCleanly, false);
  registry->RegisterDictionaryPref(prefs::kSyncInvalidationVersions);
  registry->RegisterStringPref(prefs::kSyncLastRunVersion, std::string());
  registry->RegisterBooleanPref(
      prefs::kSyncPassphraseEncryptionTransitionInProgress, false);
  registry->RegisterStringPref(prefs::kSyncNigoriStateForPassphraseTransition,
                               std::string());
}

void SyncPrefs::AddSyncPrefObserver(SyncPrefObserver* sync_pref_observer) {
  DCHECK(CalledOnValidThread());
  sync_pref_observers_.AddObserver(sync_pref_observer);
}

void SyncPrefs::RemoveSyncPrefObserver(SyncPrefObserver* sync_pref_observer) {
  DCHECK(CalledOnValidThread());
  sync_pref_observers_.RemoveObserver(sync_pref_observer);
}

void SyncPrefs::ClearPreferences() {
  DCHECK(CalledOnValidThread());
  pref_service_->ClearPref(prefs::kSyncLastSyncedTime);
  pref_service_->ClearPref(prefs::kSyncLastPollTime);
  pref_service_->ClearPref(prefs::kSyncFirstSetupComplete);
  pref_service_->ClearPref(prefs::kSyncEncryptionBootstrapToken);
  pref_service_->ClearPref(prefs::kSyncKeystoreEncryptionBootstrapToken);
  pref_service_->ClearPref(prefs::kSyncPassphrasePrompted);
  pref_service_->ClearPref(prefs::kSyncMemoryPressureWarningCount);
  pref_service_->ClearPref(prefs::kSyncShutdownCleanly);
  pref_service_->ClearPref(prefs::kSyncInvalidationVersions);
  pref_service_->ClearPref(prefs::kSyncLastRunVersion);
  pref_service_->ClearPref(
      prefs::kSyncPassphraseEncryptionTransitionInProgress);
  pref_service_->ClearPref(prefs::kSyncNigoriStateForPassphraseTransition);

  // TODO(nick): The current behavior does not clear
  // e.g. prefs::kSyncBookmarks.  Is that really what we want?
}

bool SyncPrefs::IsFirstSetupComplete() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetBoolean(prefs::kSyncFirstSetupComplete);
}

void SyncPrefs::SetFirstSetupComplete() {
  DCHECK(CalledOnValidThread());
  pref_service_->SetBoolean(prefs::kSyncFirstSetupComplete, true);
}

bool SyncPrefs::SyncHasAuthError() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetBoolean(prefs::kSyncHasAuthError);
}

void SyncPrefs::SetSyncAuthError(bool error) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetBoolean(prefs::kSyncHasAuthError, error);
}

bool SyncPrefs::IsSyncRequested() const {
  DCHECK(CalledOnValidThread());
  // IsSyncRequested is the inverse of the old SuppressStart pref.
  // Since renaming a pref value is hard, here we still use the old one.
  return !pref_service_->GetBoolean(prefs::kSyncSuppressStart);
}

void SyncPrefs::SetSyncRequested(bool is_requested) {
  DCHECK(CalledOnValidThread());
  // See IsSyncRequested for why we use this pref and !is_requested.
  pref_service_->SetBoolean(prefs::kSyncSuppressStart, !is_requested);
}

base::Time SyncPrefs::GetLastSyncedTime() const {
  DCHECK(CalledOnValidThread());
  return base::Time::FromInternalValue(
      pref_service_->GetInt64(prefs::kSyncLastSyncedTime));
}

void SyncPrefs::SetLastSyncedTime(base::Time time) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetInt64(prefs::kSyncLastSyncedTime, time.ToInternalValue());
}

base::Time SyncPrefs::GetLastPollTime() const {
  DCHECK(CalledOnValidThread());
  return base::Time::FromInternalValue(
      pref_service_->GetInt64(prefs::kSyncLastSyncedTime));
}

void SyncPrefs::SetLastPollTime(base::Time time) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetInt64(prefs::kSyncLastPollTime, time.ToInternalValue());
}

bool SyncPrefs::HasKeepEverythingSynced() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetBoolean(prefs::kSyncKeepEverythingSynced);
}

void SyncPrefs::SetKeepEverythingSynced(bool keep_everything_synced) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetBoolean(prefs::kSyncKeepEverythingSynced,
                            keep_everything_synced);
}

ModelTypeSet SyncPrefs::GetPreferredDataTypes(
    ModelTypeSet registered_types) const {
  DCHECK(CalledOnValidThread());

  if (pref_service_->GetBoolean(prefs::kSyncKeepEverythingSynced)) {
    return registered_types;
  }

  ModelTypeSet preferred_types;
  for (ModelTypeSet::Iterator it = registered_types.First(); it.Good();
       it.Inc()) {
    if (GetDataTypePreferred(it.Get())) {
      preferred_types.Put(it.Get());
    }
  }
  return ResolvePrefGroups(registered_types, preferred_types);
}

void SyncPrefs::SetPreferredDataTypes(ModelTypeSet registered_types,
                                      ModelTypeSet preferred_types) {
  DCHECK(CalledOnValidThread());
  preferred_types = ResolvePrefGroups(registered_types, preferred_types);
  DCHECK(registered_types.HasAll(preferred_types));
  for (ModelTypeSet::Iterator i = registered_types.First(); i.Good(); i.Inc()) {
    SetDataTypePreferred(i.Get(), preferred_types.Has(i.Get()));
  }
}

bool SyncPrefs::IsManaged() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetBoolean(prefs::kSyncManaged);
}

std::string SyncPrefs::GetEncryptionBootstrapToken() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetString(prefs::kSyncEncryptionBootstrapToken);
}

void SyncPrefs::SetEncryptionBootstrapToken(const std::string& token) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetString(prefs::kSyncEncryptionBootstrapToken, token);
}

std::string SyncPrefs::GetKeystoreEncryptionBootstrapToken() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetString(prefs::kSyncKeystoreEncryptionBootstrapToken);
}

void SyncPrefs::SetKeystoreEncryptionBootstrapToken(const std::string& token) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetString(prefs::kSyncKeystoreEncryptionBootstrapToken, token);
}

std::string SyncPrefs::GetSyncSessionsGUID() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetString(prefs::kSyncSessionsGUID);
}

void SyncPrefs::SetSyncSessionsGUID(const std::string& guid) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetString(prefs::kSyncSessionsGUID, guid);
}

// static
const char* SyncPrefs::GetPrefNameForDataType(ModelType data_type) {
  switch (data_type) {
    case BOOKMARKS:
      return prefs::kSyncBookmarks;
    case PASSWORDS:
      return prefs::kSyncPasswords;
    case PREFERENCES:
      return prefs::kSyncPreferences;
    case AUTOFILL:
      return prefs::kSyncAutofill;
    case AUTOFILL_PROFILE:
      return prefs::kSyncAutofillProfile;
    case AUTOFILL_WALLET_DATA:
      return prefs::kSyncAutofillWallet;
    case AUTOFILL_WALLET_METADATA:
      return prefs::kSyncAutofillWalletMetadata;
    case THEMES:
      return prefs::kSyncThemes;
    case TYPED_URLS:
      return prefs::kSyncTypedUrls;
    case EXTENSION_SETTINGS:
      return prefs::kSyncExtensionSettings;
    case EXTENSIONS:
      return prefs::kSyncExtensions;
    case APP_LIST:
      return prefs::kSyncAppList;
    case APP_SETTINGS:
      return prefs::kSyncAppSettings;
    case APPS:
      return prefs::kSyncApps;
    case SEARCH_ENGINES:
      return prefs::kSyncSearchEngines;
    case SESSIONS:
      return prefs::kSyncSessions;
    case APP_NOTIFICATIONS:
      return prefs::kSyncAppNotifications;
    case HISTORY_DELETE_DIRECTIVES:
      return prefs::kSyncHistoryDeleteDirectives;
    case SYNCED_NOTIFICATIONS:
      return prefs::kSyncSyncedNotifications;
    case SYNCED_NOTIFICATION_APP_INFO:
      return prefs::kSyncSyncedNotificationAppInfo;
    case DICTIONARY:
      return prefs::kSyncDictionary;
    case FAVICON_IMAGES:
      return prefs::kSyncFaviconImages;
    case FAVICON_TRACKING:
      return prefs::kSyncFaviconTracking;
    case SUPERVISED_USER_SETTINGS:
      return prefs::kSyncSupervisedUserSettings;
    case PROXY_TABS:
      return prefs::kSyncTabs;
    case PRIORITY_PREFERENCES:
      return prefs::kSyncPriorityPreferences;
    case SUPERVISED_USERS:
      return prefs::kSyncSupervisedUsers;
    case ARTICLES:
      return prefs::kSyncArticles;
    case SUPERVISED_USER_SHARED_SETTINGS:
      return prefs::kSyncSupervisedUserSharedSettings;
    case SUPERVISED_USER_WHITELISTS:
      return prefs::kSyncSupervisedUserWhitelists;
    case DEVICE_INFO:
      return prefs::kSyncDeviceInfo;
    case WIFI_CREDENTIALS:
      return prefs::kSyncWifiCredentials;
    case ARC_PACKAGE:
      return prefs::kSyncArcPackage;
    case PRINTERS:
      return prefs::kSyncPrinters;
    case READING_LIST:
      return prefs::kSyncReadingList;
    default:
      break;
  }
  NOTREACHED() << "Type is " << data_type;
  return nullptr;
}

#if defined(OS_CHROMEOS)
std::string SyncPrefs::GetSpareBootstrapToken() const {
  DCHECK(CalledOnValidThread());
  return pref_service_->GetString(prefs::kSyncSpareBootstrapToken);
}

void SyncPrefs::SetSpareBootstrapToken(const std::string& token) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetString(prefs::kSyncSpareBootstrapToken, token);
}
#endif

void SyncPrefs::OnSyncManagedPrefChanged() {
  DCHECK(CalledOnValidThread());
  for (auto& observer : sync_pref_observers_)
    observer.OnSyncManagedPrefChange(*pref_sync_managed_);
}

void SyncPrefs::SetManagedForTest(bool is_managed) {
  DCHECK(CalledOnValidThread());
  pref_service_->SetBoolean(prefs::kSyncManaged, is_managed);
}

void SyncPrefs::RegisterPrefGroups() {
  pref_groups_[APPS].Put(APP_NOTIFICATIONS);
  pref_groups_[APPS].Put(APP_SETTINGS);
  pref_groups_[APPS].Put(APP_LIST);
  pref_groups_[APPS].Put(ARC_PACKAGE);
  pref_groups_[APPS].Put(READING_LIST);

  pref_groups_[AUTOFILL].Put(AUTOFILL_PROFILE);
  pref_groups_[AUTOFILL].Put(AUTOFILL_WALLET_DATA);
  pref_groups_[AUTOFILL].Put(AUTOFILL_WALLET_METADATA);

  pref_groups_[EXTENSIONS].Put(EXTENSION_SETTINGS);

  pref_groups_[PREFERENCES].Put(DICTIONARY);
  pref_groups_[PREFERENCES].Put(PRIORITY_PREFERENCES);
  pref_groups_[PREFERENCES].Put(SEARCH_ENGINES);

  pref_groups_[TYPED_URLS].Put(HISTORY_DELETE_DIRECTIVES);
  pref_groups_[TYPED_URLS].Put(SESSIONS);
  pref_groups_[TYPED_URLS].Put(FAVICON_IMAGES);
  pref_groups_[TYPED_URLS].Put(FAVICON_TRACKING);

  pref_groups_[PROXY_TABS].Put(SESSIONS);
  pref_groups_[PROXY_TABS].Put(FAVICON_IMAGES);
  pref_groups_[PROXY_TABS].Put(FAVICON_TRACKING);

  // TODO(zea): put favicons in the bookmarks group as well once it handles
  // those favicons.
}

// static
void SyncPrefs::RegisterDataTypePreferredPref(
    user_prefs::PrefRegistrySyncable* registry,
    ModelType type,
    bool is_preferred) {
  const char* pref_name = GetPrefNameForDataType(type);
  if (!pref_name) {
    NOTREACHED();
    return;
  }
  registry->RegisterBooleanPref(pref_name, is_preferred);
}

bool SyncPrefs::GetDataTypePreferred(ModelType type) const {
  DCHECK(CalledOnValidThread());
  const char* pref_name = GetPrefNameForDataType(type);
  if (!pref_name) {
    NOTREACHED();
    return false;
  }

  // Device info is always enabled.
  if (pref_name == prefs::kSyncDeviceInfo)
    return true;

  if (type == PROXY_TABS &&
      pref_service_->GetUserPrefValue(pref_name) == nullptr &&
      pref_service_->IsUserModifiablePreference(pref_name)) {
    // If there is no tab sync preference yet (i.e. newly enabled type),
    // default to the session sync preference value.
    pref_name = GetPrefNameForDataType(SESSIONS);
  }

  return pref_service_->GetBoolean(pref_name);
}

void SyncPrefs::SetDataTypePreferred(ModelType type, bool is_preferred) {
  DCHECK(CalledOnValidThread());
  const char* pref_name = GetPrefNameForDataType(type);
  if (!pref_name) {
    NOTREACHED();
    return;
  }

  // Device info is always preferred.
  if (type == DEVICE_INFO)
    return;

  pref_service_->SetBoolean(pref_name, is_preferred);
}

ModelTypeSet SyncPrefs::ResolvePrefGroups(ModelTypeSet registered_types,
                                          ModelTypeSet types) const {
  ModelTypeSet types_with_groups = types;
  for (PrefGroupsMap::const_iterator i = pref_groups_.begin();
       i != pref_groups_.end(); ++i) {
    if (types.Has(i->first))
      types_with_groups.PutAll(i->second);
  }
  types_with_groups.RetainAll(registered_types);
  return types_with_groups;
}

base::Time SyncPrefs::GetFirstSyncTime() const {
  return base::Time::FromInternalValue(
      pref_service_->GetInt64(prefs::kSyncFirstSyncTime));
}

void SyncPrefs::SetFirstSyncTime(base::Time time) {
  pref_service_->SetInt64(prefs::kSyncFirstSyncTime, time.ToInternalValue());
}

void SyncPrefs::ClearFirstSyncTime() {
  pref_service_->ClearPref(prefs::kSyncFirstSyncTime);
}

bool SyncPrefs::IsPassphrasePrompted() const {
  return pref_service_->GetBoolean(prefs::kSyncPassphrasePrompted);
}

void SyncPrefs::SetPassphrasePrompted(bool value) {
  pref_service_->SetBoolean(prefs::kSyncPassphrasePrompted, value);
}

int SyncPrefs::GetMemoryPressureWarningCount() const {
  return pref_service_->GetInteger(prefs::kSyncMemoryPressureWarningCount);
}

void SyncPrefs::SetMemoryPressureWarningCount(int value) {
  pref_service_->SetInteger(prefs::kSyncMemoryPressureWarningCount, value);
}

bool SyncPrefs::DidSyncShutdownCleanly() const {
  return pref_service_->GetBoolean(prefs::kSyncShutdownCleanly);
}

void SyncPrefs::SetCleanShutdown(bool value) {
  pref_service_->SetBoolean(prefs::kSyncShutdownCleanly, value);
}

void SyncPrefs::GetInvalidationVersions(
    std::map<ModelType, int64_t>* invalidation_versions) const {
  const base::DictionaryValue* invalidation_dictionary =
      pref_service_->GetDictionary(prefs::kSyncInvalidationVersions);
  ModelTypeSet protocol_types = ProtocolTypes();
  for (auto iter = protocol_types.First(); iter.Good(); iter.Inc()) {
    std::string key = ModelTypeToString(iter.Get());
    std::string version_str;
    if (!invalidation_dictionary->GetString(key, &version_str))
      continue;
    int64_t version = 0;
    if (!base::StringToInt64(version_str, &version))
      continue;
    (*invalidation_versions)[iter.Get()] = version;
  }
}

void SyncPrefs::UpdateInvalidationVersions(
    const std::map<ModelType, int64_t>& invalidation_versions) {
  std::unique_ptr<base::DictionaryValue> invalidation_dictionary(
      new base::DictionaryValue());
  for (const auto& map_iter : invalidation_versions) {
    std::string version_str = base::Int64ToString(map_iter.second);
    invalidation_dictionary->SetString(ModelTypeToString(map_iter.first),
                                       version_str);
  }
  pref_service_->Set(prefs::kSyncInvalidationVersions,
                     *invalidation_dictionary);
}

std::string SyncPrefs::GetLastRunVersion() const {
  return pref_service_->GetString(prefs::kSyncLastRunVersion);
}

void SyncPrefs::SetLastRunVersion(const std::string& current_version) {
  pref_service_->SetString(prefs::kSyncLastRunVersion, current_version);
}

void SyncPrefs::SetPassphraseEncryptionTransitionInProgress(bool value) {
  pref_service_->SetBoolean(
      prefs::kSyncPassphraseEncryptionTransitionInProgress, value);
}

bool SyncPrefs::GetPassphraseEncryptionTransitionInProgress() const {
  return pref_service_->GetBoolean(
      prefs::kSyncPassphraseEncryptionTransitionInProgress);
}

void SyncPrefs::SetNigoriSpecificsForPassphraseTransition(
    const sync_pb::NigoriSpecifics& nigori_specifics) {
  std::string encoded;
  base::Base64Encode(nigori_specifics.SerializeAsString(), &encoded);
  pref_service_->SetString(prefs::kSyncNigoriStateForPassphraseTransition,
                           encoded);
}

void SyncPrefs::GetNigoriSpecificsForPassphraseTransition(
    sync_pb::NigoriSpecifics* nigori_specifics) const {
  const std::string encoded =
      pref_service_->GetString(prefs::kSyncNigoriStateForPassphraseTransition);
  std::string decoded;
  if (base::Base64Decode(encoded, &decoded)) {
    nigori_specifics->ParseFromString(decoded);
  }
}

}  // namespace syncer
