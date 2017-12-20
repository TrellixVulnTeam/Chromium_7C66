// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/safe_browsing/permission_reporter.h"

#include <functional>

#include "base/hash.h"
#include "base/memory/ptr_util.h"
#include "base/time/default_clock.h"
#include "chrome/browser/permissions/permission_request.h"
#include "chrome/common/safe_browsing/permission_report.pb.h"
#include "components/variations/active_field_trials.h"
#include "content/public/browser/permission_type.h"
#include "net/url_request/report_sender.h"

using content::PermissionType;

namespace safe_browsing {

namespace {
// URL to upload permission action reports.
const char kPermissionActionReportingUploadUrl[] =
    "https://safebrowsing.googleusercontent.com/safebrowsing/clientreport/"
    "chrome-permissions";

const int kMaximumReportsPerOriginPerPermissionPerMinute = 5;

PermissionReport::PermissionType PermissionTypeForReport(
    PermissionType permission) {
  switch (permission) {
    case PermissionType::MIDI_SYSEX:
      return PermissionReport::MIDI_SYSEX;
    case PermissionType::PUSH_MESSAGING:
      return PermissionReport::PUSH_MESSAGING;
    case PermissionType::NOTIFICATIONS:
      return PermissionReport::NOTIFICATIONS;
    case PermissionType::GEOLOCATION:
      return PermissionReport::GEOLOCATION;
    case PermissionType::PROTECTED_MEDIA_IDENTIFIER:
      return PermissionReport::PROTECTED_MEDIA_IDENTIFIER;
    case PermissionType::MIDI:
      return PermissionReport::MIDI;
    case PermissionType::DURABLE_STORAGE:
      return PermissionReport::DURABLE_STORAGE;
    case PermissionType::AUDIO_CAPTURE:
      return PermissionReport::AUDIO_CAPTURE;
    case PermissionType::VIDEO_CAPTURE:
      return PermissionReport::VIDEO_CAPTURE;
    case PermissionType::BACKGROUND_SYNC:
      return PermissionReport::BACKGROUND_SYNC;
    case PermissionType::FLASH:
      return PermissionReport::FLASH;
    case PermissionType::NUM:
      break;
  }

  NOTREACHED();
  return PermissionReport::UNKNOWN_PERMISSION;
}

PermissionReport::Action PermissionActionForReport(PermissionAction action) {
  switch (action) {
    case GRANTED:
      return PermissionReport::GRANTED;
    case DENIED:
      return PermissionReport::DENIED;
    case DISMISSED:
      return PermissionReport::DISMISSED;
    case IGNORED:
      return PermissionReport::IGNORED;
    case REVOKED:
      return PermissionReport::REVOKED;
    case REENABLED:
    case REQUESTED:
      return PermissionReport::ACTION_UNSPECIFIED;
    case PERMISSION_ACTION_NUM:
      break;
  }

  NOTREACHED();
  return PermissionReport::ACTION_UNSPECIFIED;
}

PermissionReport::SourceUI SourceUIForReport(PermissionSourceUI source_ui) {
  switch (source_ui) {
    case PermissionSourceUI::PROMPT:
      return PermissionReport::PROMPT;
    case PermissionSourceUI::OIB:
      return PermissionReport::OIB;
    case PermissionSourceUI::SITE_SETTINGS:
      return PermissionReport::SITE_SETTINGS;
    case PermissionSourceUI::PAGE_ACTION:
      return PermissionReport::PAGE_ACTION;
    case PermissionSourceUI::SOURCE_UI_NUM:
      break;
  }

  NOTREACHED();
  return PermissionReport::SOURCE_UI_UNSPECIFIED;
}

PermissionReport::GestureType GestureTypeForReport(
    PermissionRequestGestureType gesture_type) {
  switch (gesture_type) {
    case PermissionRequestGestureType::UNKNOWN:
      return PermissionReport::GESTURE_TYPE_UNSPECIFIED;
    case PermissionRequestGestureType::GESTURE:
      return PermissionReport::GESTURE;
    case PermissionRequestGestureType::NO_GESTURE:
      return PermissionReport::NO_GESTURE;
    case PermissionRequestGestureType::NUM:
      break;
  }

  NOTREACHED();
  return PermissionReport::GESTURE_TYPE_UNSPECIFIED;
}

PermissionReport::PersistDecision PersistDecisionForReport(
    PermissionPersistDecision persist_decision) {
  switch (persist_decision) {
    case PermissionPersistDecision::UNSPECIFIED:
      return PermissionReport::PERSIST_DECISION_UNSPECIFIED;
    case PermissionPersistDecision::PERSISTED:
      return PermissionReport::PERSISTED;
    case PermissionPersistDecision::NOT_PERSISTED:
      return PermissionReport::NOT_PERSISTED;
  }

  NOTREACHED();
  return PermissionReport::PERSIST_DECISION_UNSPECIFIED;
}

}  // namespace

bool PermissionAndOrigin::operator==(const PermissionAndOrigin& other) const {
  return (permission == other.permission && origin == other.origin);
}

std::size_t PermissionAndOriginHash::operator()(
    const PermissionAndOrigin& permission_and_origin) const {
  std::size_t permission_hash =
      static_cast<std::size_t>(permission_and_origin.permission);
  std::size_t origin_hash =
      std::hash<std::string>()(permission_and_origin.origin.spec());
  return base::HashInts(permission_hash, origin_hash);
}

PermissionReporter::PermissionReporter(net::URLRequestContext* request_context)
    : PermissionReporter(
          base::MakeUnique<net::ReportSender>(
              request_context,
              net::ReportSender::CookiesPreference::DO_NOT_SEND_COOKIES),
          base::WrapUnique(new base::DefaultClock)) {}

PermissionReporter::PermissionReporter(
    std::unique_ptr<net::ReportSender> report_sender,
    std::unique_ptr<base::Clock> clock)
    : permission_report_sender_(std::move(report_sender)),
      clock_(std::move(clock)) {}

PermissionReporter::~PermissionReporter() {}

void PermissionReporter::SendReport(const PermissionReportInfo& report_info) {
  if (IsReportThresholdExceeded(report_info.permission, report_info.origin))
    return;

  std::string serialized_report;
  BuildReport(report_info, &serialized_report);
  permission_report_sender_->Send(GURL(kPermissionActionReportingUploadUrl),
                                  "application/octet-stream", serialized_report,
                                  base::Closure(),
                                  base::Callback<void(const GURL&, int)>());
}

// static
bool PermissionReporter::BuildReport(const PermissionReportInfo& report_info,
                                     std::string* output) {
  PermissionReport report;
  report.set_origin(report_info.origin.spec());
  report.set_permission(PermissionTypeForReport(report_info.permission));
  report.set_action(PermissionActionForReport(report_info.action));
  report.set_source_ui(SourceUIForReport(report_info.source_ui));
  report.set_gesture(GestureTypeForReport(report_info.gesture_type));
  report.set_persisted(
      PersistDecisionForReport(report_info.persist_decision));
  report.set_num_prior_dismissals(report_info.num_prior_dismissals);
  report.set_num_prior_ignores(report_info.num_prior_ignores);

  // Collect platform data.
#if defined(OS_ANDROID)
  report.set_platform_type(PermissionReport::ANDROID_PLATFORM);
#elif defined(OS_MACOSX) || defined(OS_WIN) || defined(OS_CHROMEOS) || \
    defined(OS_LINUX)
  report.set_platform_type(PermissionReport::DESKTOP_PLATFORM);
#else
#error Unsupported platform.
#endif

  // Collect field trial data.
  std::vector<variations::ActiveGroupId> active_group_ids;
  variations::GetFieldTrialActiveGroupIds(&active_group_ids);
  for (auto active_group_id : active_group_ids) {
    PermissionReport::FieldTrial* field_trial = report.add_field_trials();
    field_trial->set_name_id(active_group_id.name);
    field_trial->set_group_id(active_group_id.group);
  }
  return report.SerializeToString(output);
}

bool PermissionReporter::IsReportThresholdExceeded(
    content::PermissionType permission,
    const GURL& origin) {
  std::queue<base::Time>& log = report_logs_[{permission, origin}];
  base::Time current_time = clock_->Now();
  // Remove entries that are sent more than one minute ago.
  while (!log.empty() &&
         current_time - log.front() > base::TimeDelta::FromMinutes(1)) {
    log.pop();
  }
  if (log.size() < kMaximumReportsPerOriginPerPermissionPerMinute) {
    log.push(current_time);
    return false;
  } else {
    return true;
  }
}

}  // namespace safe_browsing
