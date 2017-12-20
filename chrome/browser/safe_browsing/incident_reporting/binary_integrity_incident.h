// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_BINARY_INTEGRITY_INCIDENT_H_
#define CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_BINARY_INTEGRITY_INCIDENT_H_

#include <stdint.h>

#include <memory>

#include "base/macros.h"
#include "chrome/browser/safe_browsing/incident_reporting/incident.h"

namespace safe_browsing {

class ClientIncidentReport_IncidentData_BinaryIntegrityIncident;

// An incident representing binaries with invalid signatures.
class BinaryIntegrityIncident : public Incident {
 public:
  explicit BinaryIntegrityIncident(
      std::unique_ptr<ClientIncidentReport_IncidentData_BinaryIntegrityIncident>
          binary_integrity);
  ~BinaryIntegrityIncident() override;

  // Incident methods:
  IncidentType GetType() const override;
  std::string GetKey() const override;
  uint32_t ComputeDigest() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(BinaryIntegrityIncident);
};

}  // namespace safe_browsing

#endif  // CHROME_BROWSER_SAFE_BROWSING_INCIDENT_REPORTING_BINARY_INTEGRITY_INCIDENT_H_
