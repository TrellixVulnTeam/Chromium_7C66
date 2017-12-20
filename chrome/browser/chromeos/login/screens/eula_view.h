// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_EULA_VIEW_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_EULA_VIEW_H_

#include <string>

namespace chromeos {

class EulaModel;

// Interface between eula screen and its representation, either WebUI
// or Views one. Note, do not forget to call OnViewDestroyed in the
// dtor.
class EulaView {
 public:
  virtual ~EulaView() {}

  virtual void PrepareToShow() = 0;
  virtual void Show() = 0;
  virtual void Hide() = 0;
  virtual void Bind(EulaModel& model) = 0;
  virtual void Unbind() = 0;
  virtual void OnPasswordFetched(const std::string& tpm_password) = 0;
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_EULA_VIEW_H_
