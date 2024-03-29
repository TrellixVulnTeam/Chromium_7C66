// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SESSIONS_IOS_CHROME_TAB_RESTORE_SERVICE_FACTORY_H_
#define IOS_CHROME_BROWSER_SESSIONS_IOS_CHROME_TAB_RESTORE_SERVICE_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "components/keyed_service/ios/browser_state_keyed_service_factory.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace sessions {
class TabRestoreService;
}

namespace ios {
class ChromeBrowserState;
}

// Singleton that owns all TabRestoreServices and associates them with
// ChromeBrowserStates.
class IOSChromeTabRestoreServiceFactory
    : public BrowserStateKeyedServiceFactory {
 public:
  static sessions::TabRestoreService* GetForBrowserState(
      ios::ChromeBrowserState* browser_state);

  static IOSChromeTabRestoreServiceFactory* GetInstance();

  // Returns the default factory used to build TabRestoreService. Can be
  // registered with SetTestingFactory to use the TabRestoreService instance
  // during testing.
  static TestingFactoryFunction GetDefaultFactory();

 private:
  friend struct base::DefaultSingletonTraits<IOSChromeTabRestoreServiceFactory>;

  IOSChromeTabRestoreServiceFactory();
  ~IOSChromeTabRestoreServiceFactory() override;

  // BrowserStateKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceFor(
      web::BrowserState* context) const override;
  bool ServiceIsNULLWhileTesting() const override;

  DISALLOW_COPY_AND_ASSIGN(IOSChromeTabRestoreServiceFactory);
};

#endif  // IOS_CHROME_BROWSER_SESSIONS_IOS_CHROME_TAB_RESTORE_SERVICE_FACTORY_H_
