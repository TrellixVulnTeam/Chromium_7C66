// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_CHROME_MOCK_RENDER_THREAD_H_
#define CHROME_RENDERER_CHROME_MOCK_RENDER_THREAD_H_

#include <string>

#include "base/macros.h"
#include "content/public/test/mock_render_thread.h"
#include "extensions/features/features.h"

struct ExtensionMsg_ExternalConnectionInfo;

// Extends content::MockRenderThread to know about extension messages.
class ChromeMockRenderThread : public content::MockRenderThread {
 public:
  ChromeMockRenderThread();
  ~ChromeMockRenderThread() override;

  // content::RenderThread overrides.
  scoped_refptr<base::SingleThreadTaskRunner> GetIOTaskRunner() override;

  //////////////////////////////////////////////////////////////////////////
  // The following functions are called by the test itself.

  void set_io_task_runner(
      const scoped_refptr<base::SingleThreadTaskRunner>& task_runner);

 protected:
  // Overrides base class implementation to add custom handling for
  // print and extensions.
  bool OnMessageReceived(const IPC::Message& msg) override;

 private:
#if BUILDFLAG(ENABLE_EXTENSIONS)
  // The callee expects to be returned a valid channel_id.
  void OnOpenChannelToExtension(int routing_id,
                                const ExtensionMsg_ExternalConnectionInfo& info,
                                const std::string& channel_name,
                                bool include_tls_channel_id,
                                int request_id);
#endif

  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(ChromeMockRenderThread);
};

#endif  // CHROME_RENDERER_CHROME_MOCK_RENDER_THREAD_H_
