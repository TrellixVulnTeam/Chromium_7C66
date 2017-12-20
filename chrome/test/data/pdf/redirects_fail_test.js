// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Load a plugin with the given paramaters.
function createPluginForUrl(streamUrl, url, headers, progressCallback) {
  var plugin = document.createElement('embed');
    plugin.type = 'application/x-google-chrome-pdf';
    plugin.addEventListener('message', function(message) {
      switch (message.data.type.toString()) {
        case 'loadProgress':
          progressCallback(message.data.progress);
        break;
      }
    }, false);

    plugin.setAttribute('src', url);
    plugin.setAttribute('stream-url', streamUrl);
    plugin.setAttribute('full-frame', '');
    plugin.setAttribute('headers', headers);
    document.body.appendChild(plugin);
}

function parseUrl(url) {
  var a = document.createElement("a");
  a.href = url;
  return a;
};

var tests = [
  // Test that if the plugin is loaded with a URL that redirects it fails.
  function redirectsFail() {
    var url = parseUrl(viewer.originalUrl_);
    var redirectUrl = url.origin + '/server-redirect?' + viewer.originalUrl_;
    createPluginForUrl(redirectUrl, redirectUrl, '', function(progress) {
      if (progress == -1)
        chrome.test.succeed();
      else
        chrome.test.fail();
    });
  },

  // Test that if the plugin is loaded with a URL that doesn't redirect but
  // subsequent requests do redirect, it fails.
  function partialRedirectsFail() {
    var url = parseUrl(viewer.originalUrl_);
    var redirectUrl = url.origin + '/server-redirect?' + viewer.originalUrl_;
    // Set the headers manually so that the first request is made using a URL
    // that doesn't redirect and subsequent requests are made using a URL that
    // does.
    var headers = 'Accept-Ranges: bytes\n' +
                  'Content-Length: 101688487\n' +
                  'Content-Type: application/pdf\n';
    createPluginForUrl(viewer.originalUrl_, redirectUrl, headers,
        function(progress) {
          if (progress == -1)
            chrome.test.succeed();
          else
            chrome.test.fail();
        });
  },

  // Test that if the plugin is loaded with a URL that doesn't redirect, it
  // succeeds.
  function noRedirectsSucceed() {
    createPluginForUrl(viewer.originalUrl_, viewer.originalUrl_, '',
        function(progress) {
          if (progress == 100)
            chrome.test.succeed()
        });
  },
];

var scriptingAPI = new PDFScriptingAPI(window, window);
scriptingAPI.setLoadCallback(function() {
  chrome.test.runTests(tests);
});
