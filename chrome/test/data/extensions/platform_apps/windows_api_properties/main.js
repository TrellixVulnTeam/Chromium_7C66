// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var current = chrome.app.window.current();
var bg = null;
var nextTestNumber = 1;

function makeEventTest(eventName, startFunction) {
  var test = function() {
    bg.clearEventCounts();
    var listener = function() {
      current[eventName].removeListener(listener);
      function waitForBackgroundPageToSeeEvent() {
        if (!bg.eventCounts[eventName] > 0) {
          bg.eventCallback = waitForBackgroundPageToSeeEvent;
        }
        else {
          bg.eventCallback = null;
          current.restore();
          chrome.test.succeed();
        }
      }
      waitForBackgroundPageToSeeEvent();
    };
    current[eventName].addListener(listener);
    startFunction();
  };
  // For anonymous functions, setting 'generatedName' controls what shows up in
  // the apitest framework's logging output.
  test.generatedName = "Test" + nextTestNumber++  + "_" + eventName;
  return test;
}


var tests = [

  makeEventTest('onMinimized', function() { current.minimize(); }),
  makeEventTest('onMaximized', function() { current.maximize(); }),
  makeEventTest('onRestored', function() {
    current.minimize();
    current.restore();
  }),
  makeEventTest('onRestored', function() {
    current.maximize();
    current.restore();
  }),
  makeEventTest('onBoundsChanged', function() {
    current.outerBounds.setPosition(5, 5);
  }),
  makeEventTest('onBoundsChanged', function() {
    current.outerBounds.setSize(150, 150);
  }),
  makeEventTest('onBoundsChanged', function() {
    current.innerBounds.setPosition(40, 40);
  }),
  makeEventTest('onBoundsChanged', function() {
    current.innerBounds.setSize(100, 100);
  })

];

chrome.runtime.getBackgroundPage(function(page) {
  bg = page;
  chrome.test.runTests(tests);
});
