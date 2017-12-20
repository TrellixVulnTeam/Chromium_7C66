// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/test/integration/sync_arc_package_helper.h"

#include <vector>

#include "base/command_line.h"
#include "base/memory/ptr_util.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/browser/chromeos/arc/arc_auth_service.h"
#include "chrome/browser/chromeos/login/users/fake_chrome_user_manager.h"
#include "chrome/browser/chromeos/login/users/scoped_user_manager_enabler.h"
#include "chrome/browser/chromeos/profiles/profile_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sync/test/integration/sync_datatype_helper.h"
#include "chrome/browser/sync/test/integration/sync_test.h"
#include "chrome/browser/ui/app_list/arc/arc_app_list_prefs.h"
#include "chrome/browser/ui/app_list/arc/arc_app_list_prefs_factory.h"
#include "chrome/browser/ui/app_list/arc/arc_package_syncable_service.h"
#include "chromeos/chromeos_switches.h"
#include "components/arc/arc_bridge_service.h"
#include "components/arc/test/fake_app_instance.h"
#include "components/arc/test/fake_arc_bridge_service.h"

namespace arc {

namespace {

std::string GetTestPackageName(size_t id) {
  return "testarcpackage" + base::SizeTToString(id);
}

chromeos::FakeChromeUserManager* GetUserManager() {
  return static_cast<chromeos::FakeChromeUserManager*>(
      user_manager::UserManager::Get());
}

const user_manager::User* CreateUserAndLogin(Profile* profile, size_t id) {
  std::string gaia_id = "1234567890" + base::SizeTToString(id);
  const AccountId account_id(
      AccountId::FromUserEmailGaiaId(profile->GetProfileUserName(), gaia_id));
  const user_manager::User* user = GetUserManager()->AddUser(account_id);
  GetUserManager()->LoginUser(account_id);
  return user;
}

}  // namespace

SyncArcPackageHelper* SyncArcPackageHelper::GetInstance() {
  SyncArcPackageHelper* instance = base::Singleton<SyncArcPackageHelper>::get();
  DCHECK(sync_datatype_helper::test());
  instance->SetupTest(sync_datatype_helper::test());
  return instance;
}

SyncArcPackageHelper::SyncArcPackageHelper()
    : test_(nullptr), setup_completed_(false), user_manager_enabler_(nullptr) {}

SyncArcPackageHelper::~SyncArcPackageHelper() {}

void SyncArcPackageHelper::SetupTest(SyncTest* test) {
  if (setup_completed_) {
    DCHECK_EQ(test, test_);
    return;
  }
  test_ = test;

  user_manager_enabler_ = base::MakeUnique<chromeos::ScopedUserManagerEnabler>(
      new chromeos::FakeChromeUserManager());
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      chromeos::switches::kEnableArc);
  ArcAppListPrefsFactory::SetFactoryForSyncTest();
  size_t id = 0;
  for (auto* profile : test_->GetAllProfiles())
    SetupArcService(profile, id++);
  setup_completed_ = true;
}

void SyncArcPackageHelper::CleanUp() {
  ArcAuthService::Get()->Shutdown();
  user_manager_enabler_.reset();
}

void SyncArcPackageHelper::InstallPackageWithIndex(Profile* profile,
                                                   size_t id) {
  std::string package_name = GetTestPackageName(id);

  mojom::ArcPackageInfo package;
  package.package_name = package_name;
  package.package_version = id;
  package.last_backup_android_id = id;
  package.last_backup_time = id;
  package.sync = true;

  InstallPackage(profile, package);
}

void SyncArcPackageHelper::UninstallPackageWithIndex(Profile* profile,
                                                     size_t id) {
  std::string package_name = GetTestPackageName(id);
  UninstallPackage(profile, package_name);
}

void SyncArcPackageHelper::ClearPackages(Profile* profile) {
  const ArcAppListPrefs* prefs = ArcAppListPrefs::Get(profile);
  DCHECK(prefs);
  const std::vector<std::string> pref_packages = prefs->GetPackagesFromPrefs();
  for (const auto& package : pref_packages) {
    UninstallPackage(profile, package);
  }
}

bool SyncArcPackageHelper::AllProfilesHaveSamePackages() {
  const auto& profiles = test_->GetAllProfiles();
  for (auto* profile : profiles) {
    if (profile != profiles.front() &&
        !ArcPackagesMatch(profiles.front(), profile)) {
      DVLOG(1) << "Packages match failed!";
      return false;
    }
  }
  return true;
}

bool SyncArcPackageHelper::AllProfilesHaveSamePackageDetails() {
  if (!AllProfilesHaveSamePackages()) {
    DVLOG(1) << "Packages match failed, skip packages detail match.";
    return false;
  }

  const auto& profiles = test_->GetAllProfiles();
  for (auto* profile : profiles) {
    if (profile != profiles.front() &&
        !ArcPackageDetailsMatch(profiles.front(), profile)) {
      DVLOG(1) << "Profile1: " << ArcPackageSyncableService::Get(profile);
      DVLOG(1) << "Profile2: "
               << ArcPackageSyncableService::Get(profiles.front());
      return false;
    }
  }
  return true;
}

void SyncArcPackageHelper::SetupArcService(Profile* profile, size_t id) {
  DCHECK(profile);
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      chromeos::switches::kEnableArc);
  const user_manager::User* user = CreateUserAndLogin(profile, id);
  // Have the user-to-profile mapping ready to avoid using the real profile
  // manager (which is null).
  chromeos::ProfileHelper::Get()->SetUserToProfileMappingForTesting(user,
                                                                    profile);

  ArcAuthService* auth_service = ArcAuthService::Get();
  DCHECK(auth_service);
  ArcAuthService::DisableUIForTesting();
  auth_service->OnPrimaryUserProfilePrepared(profile);
  auth_service->EnableArc();

  ArcAppListPrefs* arc_app_list_prefs = ArcAppListPrefs::Get(profile);
  DCHECK(arc_app_list_prefs);

  base::RunLoop run_loop;
  arc_app_list_prefs->SetDefaltAppsReadyCallback(run_loop.QuitClosure());
  run_loop.Run();

  instance_map_[profile] =
      base::MakeUnique<FakeAppInstance>(arc_app_list_prefs);
  DCHECK(instance_map_[profile].get());
  arc_app_list_prefs->app_instance_holder()->SetInstance(
      instance_map_[profile].get());
  // OnPackageListRefreshed will be called when AppInstance is ready.
  // For fakeAppInstance we use SendRefreshPackageList to make sure that
  // OnPackageListRefreshed will be called.
  instance_map_[profile]->SendRefreshPackageList(
      std::vector<arc::mojom::ArcPackageInfo>());
}

void SyncArcPackageHelper::InstallPackage(
    Profile* profile,
    const mojom::ArcPackageInfo& package) {
  ArcAppListPrefs* arc_app_list_prefs = ArcAppListPrefs::Get(profile);
  DCHECK(arc_app_list_prefs);
  FakeAppInstance* fake_app_instance = static_cast<FakeAppInstance*>(
      arc_app_list_prefs->app_instance_holder()->GetInstanceForMethod(
          "InstallPackage"));

  DCHECK(fake_app_instance);
  // After this function, new package should be added to local sync service
  // and install event should be sent to sync server.
  fake_app_instance->InstallPackage(package.Clone());
}

void SyncArcPackageHelper::UninstallPackage(Profile* profile,
                                            const std::string& package_name) {
  ArcAppListPrefs* arc_app_list_prefs = ArcAppListPrefs::Get(profile);
  DCHECK(arc_app_list_prefs);
  FakeAppInstance* fake_app_instance = static_cast<FakeAppInstance*>(
      arc_app_list_prefs->app_instance_holder()->GetInstanceForMethod(
          "UninstallPackage"));
  DCHECK(fake_app_instance);
  // After this function, package should be removed from local sync service
  // and uninstall event should be sent to sync server.
  fake_app_instance->UninstallPackage(package_name);
}

// Packages from local pref are used for these test functions. Packages in local
// pref should be indentical to syncservice after syncservice is launched.
// Packagd update behavior is not synced by design.
bool SyncArcPackageHelper::ArcPackagesMatch(Profile* profile1,
                                            Profile* profile2) {
  const ArcAppListPrefs* prefs1 = ArcAppListPrefs::Get(profile1);
  const ArcAppListPrefs* prefs2 = ArcAppListPrefs::Get(profile2);
  DCHECK(prefs1);
  DCHECK(prefs2);
  const std::vector<std::string> pref1_packages =
      prefs1->GetPackagesFromPrefs();
  const std::vector<std::string> pref2_packages =
      prefs2->GetPackagesFromPrefs();
  if (pref1_packages.size() != pref2_packages.size())
    return false;
  for (const auto& package : pref1_packages) {
    std::unique_ptr<ArcAppListPrefs::PackageInfo> package_info =
        prefs2->GetPackage(package);
    if (!package_info.get())
      return false;
  }
  return true;
}

bool SyncArcPackageHelper::ArcPackageDetailsMatch(Profile* profile1,
                                                  Profile* profile2) {
  const ArcAppListPrefs* prefs1 = ArcAppListPrefs::Get(profile1);
  const ArcAppListPrefs* prefs2 = ArcAppListPrefs::Get(profile2);
  DCHECK(prefs1);
  DCHECK(prefs2);
  const std::vector<std::string> pref1_packages =
      prefs1->GetPackagesFromPrefs();
  for (const auto& package : pref1_packages) {
    std::unique_ptr<ArcAppListPrefs::PackageInfo> package1_info =
        prefs1->GetPackage(package);
    std::unique_ptr<ArcAppListPrefs::PackageInfo> package2_info =
        prefs2->GetPackage(package);
    if (!package2_info.get())
      return false;
    if (package1_info->package_version != package2_info->package_version)
      return false;
    if (package1_info->last_backup_android_id !=
        package2_info->last_backup_android_id)
      return false;
    if (package1_info->last_backup_time != package2_info->last_backup_time)
      return false;
  }
  return true;
}

}  // namespace arc
