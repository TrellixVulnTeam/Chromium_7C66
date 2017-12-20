// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DRIVER_DATA_TYPE_MANAGER_IMPL_H__
#define COMPONENTS_SYNC_DRIVER_DATA_TYPE_MANAGER_IMPL_H__

#include "components/sync/driver/data_type_manager.h"

#include <map>
#include <queue>
#include <vector>

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "components/sync/base/weak_handle.h"
#include "components/sync/driver/backend_data_type_configurer.h"
#include "components/sync/driver/model_association_manager.h"

namespace syncer {

class DataTypeController;
class DataTypeDebugInfoListener;
class DataTypeEncryptionHandler;
class DataTypeManagerObserver;
struct DataTypeConfigurationStats;

// List of data types grouped by priority and ordered from high priority to
// low priority.
typedef std::queue<ModelTypeSet> TypeSetPriorityList;

class DataTypeManagerImpl : public DataTypeManager,
                            public ModelAssociationManagerDelegate {
 public:
  DataTypeManagerImpl(
      const WeakHandle<DataTypeDebugInfoListener>& debug_info_listener,
      const DataTypeController::TypeMap* controllers,
      const DataTypeEncryptionHandler* encryption_handler,
      BackendDataTypeConfigurer* configurer,
      DataTypeManagerObserver* observer);
  ~DataTypeManagerImpl() override;

  // DataTypeManager interface.
  void Configure(ModelTypeSet desired_types, ConfigureReason reason) override;
  void ReenableType(ModelType type) override;
  void ResetDataTypeErrors() override;

  // Needed only for backend migration.
  void PurgeForMigration(ModelTypeSet undesired_types,
                         ConfigureReason reason) override;

  void Stop() override;
  State state() const override;

  // |ModelAssociationManagerDelegate| implementation.
  void OnAllDataTypesReadyForConfigure() override;
  void OnSingleDataTypeAssociationDone(
      ModelType type,
      const DataTypeAssociationStats& association_stats) override;
  void OnModelAssociationDone(
      const DataTypeManager::ConfigureResult& result) override;
  void OnSingleDataTypeWillStop(ModelType type,
                                const SyncError& error) override;

  // Used by unit tests. TODO(sync) : This would go away if we made
  // this class be able to do Dependency injection. crbug.com/129212.
  ModelAssociationManager* GetModelAssociationManagerForTesting() {
    return &model_association_manager_;
  }

 private:
  // Helper enum for identifying which types within a priority group to
  // associate.
  enum AssociationGroup {
    // Those types that were already downloaded and didn't have an error at
    // configuration time. Corresponds with AssociationTypesInfo's
    // |ready_types|. These types can start associating as soon as the
    // ModelAssociationManager is not busy.
    READY_AT_CONFIG,
    // All other types, including first time sync types and those that have
    // encountered an error. These types must wait until the syncer has done
    // any db changes and/or downloads before associating.
    UNREADY_AT_CONFIG,
  };

  friend class TestDataTypeManager;

  // Abort configuration and stop all data types due to configuration errors.
  void Abort(ConfigureStatus status);

  // Returns the priority types (control + priority user types).
  // Virtual for overriding during tests.
  virtual ModelTypeSet GetPriorityTypes() const;

  // Divide |types| into sets by their priorities and return the sets from
  // high priority to low priority.
  TypeSetPriorityList PrioritizeTypes(const ModelTypeSet& types);

  // Update unready state of types in data_type_status_table_ to match value of
  // DataTypeController::ReadyForStart().
  void UpdateUnreadyTypeErrors(const ModelTypeSet& desired_types);

  // Post a task to reconfigure when no downloading or association are running.
  void ProcessReconfigure();

  void Restart(ConfigureReason reason);
  void DownloadReady(ModelTypeSet types_to_download,
                     ModelTypeSet first_sync_types,
                     ModelTypeSet failed_configuration_types);

  // Notification from the SBH that download failed due to a transient
  // error and it will be retried.
  void OnDownloadRetry();
  void NotifyStart();
  void NotifyDone(const ConfigureResult& result);

  void ConfigureImpl(ModelTypeSet desired_types, ConfigureReason reason);

  // Calls data type controllers of requested types to register with backend.
  void RegisterTypesWithBackend();

  BackendDataTypeConfigurer::DataTypeConfigStateMap BuildDataTypeConfigStateMap(
      const ModelTypeSet& types_being_configured) const;

  // Start download of next set of types in |download_types_queue_| (if
  // any exist, does nothing otherwise).
  // Will kick off association of any new ready types.
  void StartNextDownload(ModelTypeSet high_priority_types_before);

  // Start association of next batch of data types after association of
  // previous batch finishes. |group| controls which set of types within
  // an AssociationTypesInfo to associate. Does nothing if model associator
  // is busy performing association.
  void StartNextAssociation(AssociationGroup group);

  void StopImpl();

  // Returns the currently enabled types.
  ModelTypeSet GetEnabledTypes() const;

  BackendDataTypeConfigurer* configurer_;
  // Map of all data type controllers that are available for sync.
  // This list is determined at startup by various command line flags.
  const DataTypeController::TypeMap* controllers_;
  State state_;
  ModelTypeSet last_requested_types_;

  // A set of types that were enabled at the time initialization with the
  // |model_association_manager_| was last attempted.
  ModelTypeSet last_enabled_types_;

  // Whether an attempt to reconfigure was made while we were busy configuring.
  // The |last_requested_types_| will reflect the newest set of requested types.
  bool needs_reconfigure_;

  // The reason for the last reconfigure attempt. Note: this will be set to a
  // valid value only when |needs_reconfigure_| is set.
  ConfigureReason last_configure_reason_;

  // The last time Restart() was called.
  base::Time last_restart_time_;

  // Sync's datatype debug info listener, which we pass model association
  // statistics to.
  const WeakHandle<DataTypeDebugInfoListener> debug_info_listener_;

  // The manager that handles the model association of the individual types.
  ModelAssociationManager model_association_manager_;

  // DataTypeManager must have only one observer -- the ProfileSyncService that
  // created it and manages its lifetime.
  DataTypeManagerObserver* const observer_;

  // For querying failed data types (having unrecoverable error) when
  // configuring backend.
  DataTypeStatusTable data_type_status_table_;

  // Types waiting to be downloaded.
  TypeSetPriorityList download_types_queue_;

  // Types waiting for association and related time tracking info.
  struct AssociationTypesInfo {
    AssociationTypesInfo();
    AssociationTypesInfo(const AssociationTypesInfo& other);
    ~AssociationTypesInfo();

    // Types to associate.
    ModelTypeSet types;
    // Types that have just been downloaded and are being associated for the
    // first time. This includes types that had previously encountered an error
    // and had to be purged/unapplied from the sync db.
    // This is a subset of |types|.
    ModelTypeSet first_sync_types;
    // Types that were already ready for association at configuration time.
    ModelTypeSet ready_types;
    // Time at which |types| began downloading.
    base::Time download_start_time;
    // Time at which |types| finished downloading.
    base::Time download_ready_time;
    // Time at which the association for |read_types| began.
    base::Time ready_association_request_time;
    // Time at which the association for |types| began (not relevant to
    // |ready_types|.
    base::Time full_association_request_time;
    // The set of types that are higher priority (and were therefore blocking)
    // the association of |types|.
    ModelTypeSet high_priority_types_before;
    // The subset of |types| that were successfully configured.
    ModelTypeSet configured_types;
  };
  std::queue<AssociationTypesInfo> association_types_queue_;

  // The encryption handler lets the DataTypeManager know the state of sync
  // datatype encryption.
  const DataTypeEncryptionHandler* encryption_handler_;

  // Association and time stats of data type configuration.
  std::vector<DataTypeConfigurationStats> configuration_stats_;

  // True iff we are in the process of catching up datatypes.
  bool catch_up_in_progress_;

  // Configuration process is started when ModelAssociationManager notifies
  // DataTypeManager that all types are ready for configure.
  // This flag ensures that this process is started only once.
  bool download_started_;

  base::WeakPtrFactory<DataTypeManagerImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(DataTypeManagerImpl);
};

}  // namespace syncer

#endif  // COMPONENTS_SYNC_DRIVER_DATA_TYPE_MANAGER_IMPL_H__
