// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.test.runTests([
  function isAvailable() {
    chrome.hotwordPrivate.getStatus(
        true,
        chrome.test.callbackPass(function(result) {
          chrome.test.sendMessage("available: " + result.available);
    }));
  }
]);
