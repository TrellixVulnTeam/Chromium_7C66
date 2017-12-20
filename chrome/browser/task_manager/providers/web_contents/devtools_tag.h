// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TASK_MANAGER_PROVIDERS_WEB_CONTENTS_DEVTOOLS_TAG_H_
#define CHROME_BROWSER_TASK_MANAGER_PROVIDERS_WEB_CONTENTS_DEVTOOLS_TAG_H_

#include "base/macros.h"
#include "chrome/browser/task_manager/providers/web_contents/devtools_task.h"
#include "chrome/browser/task_manager/providers/web_contents/web_contents_tag.h"

namespace task_manager {

// Defines a concrete UserData type for WebContents owned by DevToolsWindow.
class DevToolsTag : public WebContentsTag {
 public:
  // task_manager::WebContentsTag:
  DevToolsTask* CreateTask() const override;

 private:
  friend class WebContentsTags;

  explicit DevToolsTag(content::WebContents* web_contents);
  ~DevToolsTag() override;

  DISALLOW_COPY_AND_ASSIGN(DevToolsTag);
};

}  // namespace task_manager

#endif  // CHROME_BROWSER_TASK_MANAGER_PROVIDERS_WEB_CONTENTS_DEVTOOLS_TAG_H_
