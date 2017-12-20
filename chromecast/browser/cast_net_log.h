// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BROWSER_CAST_NET_LOG_H_
#define CHROMECAST_BROWSER_CAST_NET_LOG_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "net/log/write_to_file_net_log_observer.h"

namespace chromecast {

class CastNetLog : public net::NetLog {
 public:
  CastNetLog();
  ~CastNetLog() override;

 private:
  std::unique_ptr<net::WriteToFileNetLogObserver> write_to_file_observer_;

  DISALLOW_COPY_AND_ASSIGN(CastNetLog);
};

}  // namespace chromecast

#endif  // CHROMECAST_BROWSER_CAST_NET_LOG_H_
