// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TEST_RUNNER_WEB_WIDGET_TEST_CLIENT_H_
#define COMPONENTS_TEST_RUNNER_WEB_WIDGET_TEST_CLIENT_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "third_party/WebKit/public/web/WebWidgetClient.h"

namespace blink {
class WebWidget;
}  // namespace blink

namespace test_runner {

class TestRunner;
class TestRunnerForSpecificView;
class WebTestDelegate;
class WebViewTestProxyBase;
class WebWidgetTestProxyBase;

// WebWidgetTestClient implements WebWidgetClient interface, providing behavior
// expected by tests.  WebWidgetTestClient ends up used by WebViewTestProxy
// which coordinates forwarding WebWidgetClient calls either to
// WebWidgetTestClient or to the product code (i.e. currently to
// RenderViewImpl).
class WebWidgetTestClient : public blink::WebWidgetClient {
 public:
  // Caller has to ensure that all arguments (i.e. |test_runner| and |delegate|)
  // live longer than |this|.
  WebWidgetTestClient(WebWidgetTestProxyBase* web_widget_test_proxy_base);

  virtual ~WebWidgetTestClient();

  // WebWidgetClient overrides needed by WebWidgetTestProxy.
  blink::WebScreenInfo screenInfo() override;
  void scheduleAnimation() override;
  bool requestPointerLock() override;
  void requestPointerUnlock() override;
  bool isPointerLocked() override;
  void setToolTipText(const blink::WebString& text,
                      blink::WebTextDirection direction) override;
  void startDragging(blink::WebReferrerPolicy policy,
                     const blink::WebDragData& data,
                     blink::WebDragOperationsMask mask,
                     const blink::WebImage& image,
                     const blink::WebPoint& point) override;

 private:
  void AnimateNow();

  WebTestDelegate* delegate();
  TestRunnerForSpecificView* view_test_runner();
  TestRunner* test_runner();

  // Borrowed pointer to WebWidgetTestProxyBase.
  WebWidgetTestProxyBase* web_widget_test_proxy_base_;

  bool animation_scheduled_;

  base::WeakPtrFactory<WebWidgetTestClient> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebWidgetTestClient);
};

}  // namespace test_runner

#endif  // COMPONENTS_TEST_RUNNER_WEB_WIDGET_TEST_CLIENT_H_
