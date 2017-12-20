// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TEST_RUNNER_ACCESSIBILITY_CONTROLLER_H_
#define COMPONENTS_TEST_RUNNER_ACCESSIBILITY_CONTROLLER_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "components/test_runner/web_ax_object_proxy.h"
#include "third_party/WebKit/public/web/WebAXObject.h"
#include "v8/include/v8.h"

namespace blink {
class WebLocalFrame;
class WebString;
class WebView;
}

namespace test_runner {

class WebViewTestProxyBase;

class AccessibilityController {
 public:
  explicit AccessibilityController(
      WebViewTestProxyBase* web_view_test_proxy_base);
  ~AccessibilityController();

  void Reset();
  void Install(blink::WebLocalFrame* frame);
  bool ShouldLogAccessibilityEvents();
  void NotificationReceived(const blink::WebAXObject& target,
                            const std::string& notification_name);

 private:
  friend class AccessibilityControllerBindings;

  // Bound methods and properties
  void LogAccessibilityEvents();
  void SetNotificationListener(v8::Local<v8::Function> callback);
  void UnsetNotificationListener();
  v8::Local<v8::Object> FocusedElement();
  v8::Local<v8::Object> RootElement();
  v8::Local<v8::Object> AccessibleElementById(const std::string& id);

  v8::Local<v8::Object> FindAccessibleElementByIdRecursive(
      const blink::WebAXObject&, const blink::WebString& id);

  // If true, will log all accessibility notifications.
  bool log_accessibility_events_;

  WebAXObjectProxyList elements_;

  v8::Persistent<v8::Function> notification_callback_;

  blink::WebView* web_view();
  WebViewTestProxyBase* web_view_test_proxy_base_;

  base::WeakPtrFactory<AccessibilityController> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AccessibilityController);
};

}  // namespace test_runner

#endif  // COMPONENTS_TEST_RUNNER_ACCESSIBILITY_CONTROLLER_H_
