// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PERMISSIONS_PERMISSION_UTIL_H_
#define CHROME_BROWSER_PERMISSIONS_PERMISSION_UTIL_H_

#include <string>

#include "base/macros.h"
#include "chrome/browser/permissions/permission_request.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "url/gurl.h"

class Profile;

namespace content {
enum class PermissionType;
}  // namespace content

enum class PermissionSourceUI;

// This enum backs a UMA histogram, so it must be treated as append-only.
enum PermissionAction {
  GRANTED = 0,
  DENIED = 1,
  DISMISSED = 2,
  IGNORED = 3,
  REVOKED = 4,
  REENABLED = 5,
  REQUESTED = 6,

  // Always keep this at the end.
  PERMISSION_ACTION_NUM,
};

struct PermissionTypeHash {
  std::size_t operator()(const content::PermissionType& type) const;
};

// A utility class for permissions.
class PermissionUtil {
 public:
  // Returns the permission string for the given PermissionType.
  static std::string GetPermissionString(content::PermissionType permission);

  // Returns the request type corresponding to a permission type.
  static PermissionRequestType GetRequestType(content::PermissionType type);

  // Returns the gesture type corresponding to whether a permission request is
  // made with or without a user gesture.
  static PermissionRequestGestureType GetGestureType(bool user_gesture);

  // Limited conversion of ContentSettingsType to PermissionType. Intended for
  // recording Permission UMA metrics from areas of the codebase which have not
  // yet been converted to PermissionType. Returns true if the conversion was
  // performed.
  // TODO(tsergeant): Remove this function once callsites operate on
  // PermissionType directly.
  static bool GetPermissionType(ContentSettingsType type,
                                content::PermissionType* out);

  static bool ShouldShowPersistenceToggle();

  // A scoped class that will check the current resolved content setting on
  // construction and report a revocation metric accordingly if the revocation
  // condition is met (from ALLOW to something else).
  class ScopedRevocationReporter {
   public:
    ScopedRevocationReporter(Profile* profile,
                             const GURL& primary_url,
                             const GURL& secondary_url,
                             ContentSettingsType content_type,
                             PermissionSourceUI source_ui);

    ScopedRevocationReporter(Profile* profile,
                             const ContentSettingsPattern& primary_pattern,
                             const ContentSettingsPattern& secondary_pattern,
                             ContentSettingsType content_type,
                             PermissionSourceUI source_ui);

    ~ScopedRevocationReporter();

   private:
    Profile* profile_;
    const GURL primary_url_;
    const GURL secondary_url_;
    ContentSettingsType content_type_;
    PermissionSourceUI source_ui_;
    bool is_initially_allowed_;
  };

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(PermissionUtil);
};

#endif  // CHROME_BROWSER_PERMISSIONS_PERMISSION_UTIL_H_
