// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/base/ash_test_environment_chrome.h"

#include "ash/test/ash_test_views_delegate.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "content/public/browser/browser_thread.h"
#include "ui/views/views_delegate.h"

AshTestEnvironmentChrome::AshTestEnvironmentChrome() {}

AshTestEnvironmentChrome::~AshTestEnvironmentChrome() {}

base::SequencedWorkerPool* AshTestEnvironmentChrome::GetBlockingPool() {
  return content::BrowserThread::GetBlockingPool();
}

std::unique_ptr<views::ViewsDelegate>
AshTestEnvironmentChrome::CreateViewsDelegate() {
  return base::MakeUnique<ash::test::AshTestViewsDelegate>();
}
