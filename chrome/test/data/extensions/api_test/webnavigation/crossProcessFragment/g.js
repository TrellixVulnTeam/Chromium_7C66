// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var target = location.href + "#foo";

function updateFragment() {
  location.href = target;
}

onload = function() {
  var frame = document.getElementById("frame");
  frame.src = "http://127.0.0.1:" + location.search.substr(1) + "/test4";
};
