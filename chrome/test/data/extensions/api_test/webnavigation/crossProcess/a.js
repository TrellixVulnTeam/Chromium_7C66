// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

onload = function() {
  setTimeout(function() {
    location.href =
        "http://127.0.0.1:" +
        location.search.substr(1) +
        "/extensions/api_test/webnavigation/crossProcess/empty.html";
  }, 0);
};
