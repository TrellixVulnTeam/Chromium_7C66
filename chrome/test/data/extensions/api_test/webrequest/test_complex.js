// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function getURLHttpXHR() {
  return getServerURL('extensions/api_test/webrequest/xhr/a.html');
}
function getURLHttpXHRJavaScript() {
  return getServerURL('extensions/api_test/webrequest/xhr/a.js');
}
function getURLHttpXHRData() {
  return getServerURL('extensions/api_test/webrequest/xhr/data.json');
}

runTests([
  // Navigates to a page with subresources.
  function complexLoad() {
    expect(
      [  // events
        { label: "a.html-onBeforeRequest",
          event: "onBeforeRequest",
          details: {
            type: "main_frame",
            url: getURL("complexLoad/a.html"),
            frameUrl: getURL("complexLoad/a.html")
          }
        },
        { label: "b.html-onBeforeRequest",
          event: "onBeforeRequest",
          details: {
            type: "sub_frame",
            url: getURL("complexLoad/b.html"),
            frameUrl: getURL("complexLoad/b.html"),
            frameId: 1,
            parentFrameId: 0,
          }
        },
        { label: "b.jpg-onBeforeRequest",
          event: "onBeforeRequest",
          details: {
            type: "image",
            url: getURL("complexLoad/b.jpg"),
            frameUrl: getURL("complexLoad/b.html"),
            frameId: 1,
            parentFrameId: 0,
          }
        },
        { label: "a.html-onResponseStarted",
          event: "onResponseStarted",
          details: {
            type: "main_frame",
            url: getURL("complexLoad/a.html"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "b.html-onResponseStarted",
          event: "onResponseStarted",
          details: {
            type: "sub_frame",
            url: getURL("complexLoad/b.html"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            frameId: 1,
            parentFrameId: 0,
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "b.jpg-onResponseStarted",
          event: "onResponseStarted",
          details: {
            type: "image",
            url: getURL("complexLoad/b.jpg"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            frameId: 1,
            parentFrameId: 0,
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "a.html-onCompleted",
          event: "onCompleted",
          details: {
            type: "main_frame",
            url: getURL("complexLoad/a.html"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "b.html-onCompleted",
          event: "onCompleted",
          details: {
            type: "sub_frame",
            url: getURL("complexLoad/b.html"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            frameId: 1,
            parentFrameId: 0,
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "b.jpg-onCompleted",
          event: "onCompleted",
          details: {
            type: "image",
            url: getURL("complexLoad/b.jpg"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            frameId: 1,
            parentFrameId: 0,
            // Request to chrome-extension:// url has no IP.
          }
        },
      ],
      [  // event order
        ["a.html-onBeforeRequest", "a.html-onResponseStarted",
         "b.html-onBeforeRequest", "b.html-onResponseStarted",
         "b.jpg-onBeforeRequest", "b.jpg-onResponseStarted" ],
        ["a.html-onResponseStarted", "a.html-onCompleted"],
        ["b.html-onResponseStarted", "b.html-onCompleted"],
        ["b.jpg-onResponseStarted", "b.jpg-onCompleted"] ]
      );
    navigateAndWait(getURL("complexLoad/a.html"));
  },

  // Loads several resources, but should only see the complexLoad main_frame
  // and image due to the filter.
  function complexLoadFiltered() {
    expect(
      [  // events
        { label: "a-onBeforeRequest",
          event: "onBeforeRequest",
          details: {
            type: "main_frame",
            url: getURL("complexLoad/a.html"),
            frameUrl: getURL("complexLoad/a.html")
          }
        },
        { label: "b-onBeforeRequest",
          event: "onBeforeRequest",
          details: {
            type: "image",
            url: getURL("complexLoad/b.jpg"),
            // As we do not listed to sub-frames we do not know the frameUrl.
            frameUrl: "unknown frame URL",
            frameId: 1,
            parentFrameId: 0,
          }
        },
        { label: "a-onResponseStarted",
          event: "onResponseStarted",
          details: {
            type: "main_frame",
            url: getURL("complexLoad/a.html"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "b-onResponseStarted",
          event: "onResponseStarted",
          details: {
            type: "image",
            url: getURL("complexLoad/b.jpg"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            frameId: 1,
            parentFrameId: 0,
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "a-onCompleted",
          event: "onCompleted",
          details: {
            type: "main_frame",
            url: getURL("complexLoad/a.html"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            // Request to chrome-extension:// url has no IP.
          }
        },
        { label: "b-onCompleted",
          event: "onCompleted",
          details: {
            type: "image",
            url: getURL("complexLoad/b.jpg"),
            fromCache: false,
            statusCode: 200,
            statusLine: "HTTP/1.1 200 OK",
            frameId: 1,
            parentFrameId: 0,
            // Request to chrome-extension:// url has no IP.
          }
        },
      ],
      [  // event order
        ["a-onBeforeRequest", "a-onResponseStarted",
         "b-onBeforeRequest", "b-onResponseStarted"],
        ["a-onResponseStarted", "a-onCompleted"],
        ["b-onResponseStarted", "b-onCompleted"] ],
      {  // filters
        urls: [getURL("complexLoad/*")],
        types: ["main_frame", "image"],
        tabId: tabId
      });
    chrome.tabs.create({ url: getURL("simpleLoad/a.html") },
        function(newTab) {
      chrome.tabs.remove(newTab.id);
      navigateAndWait(getURL("complexLoad/a.html"));
    });
  },

  // Navigates to a page to generates an XHR.
  function xhrLoad() {
    expect(
      [  // events
        { label: "onBeforeRequest-1",
          event: "onBeforeRequest",
          details: {
            type: "main_frame",
            url: getURLHttpXHR(),
            frameUrl: getURLHttpXHR()
          }
        },
        { label: "onBeforeSendHeaders-1",
          event: "onBeforeSendHeaders",
          details: {
            type: "main_frame",
            url: getURLHttpXHR(),
          }
        },
        { label: "onSendHeaders-1",
          event: "onSendHeaders",
          details: {
            type: "main_frame",
            url: getURLHttpXHR(),
          }
        },
        { label: "onHeadersReceived-1",
          event: "onHeadersReceived",
          details: {
            type: "main_frame",
            url: getURLHttpXHR(),
            statusLine: "HTTP/1.1 200 OK",
            statusCode: 200,
          }
        },
        { label: "onResponseStarted-1",
          event: "onResponseStarted",
          details: {
            type: "main_frame",
            url: getURLHttpXHR(),
            statusCode: 200,
            ip: "127.0.0.1",
            fromCache: false,
            statusLine: "HTTP/1.1 200 OK",
          }
        },
        { label: "onCompleted-1",
          event: "onCompleted",
          details: {
            type: "main_frame",
            url: getURLHttpXHR(),
            statusCode: 200,
            ip: "127.0.0.1",
            fromCache: false,
            statusLine: "HTTP/1.1 200 OK",
          }
        },
        { label: "a.js-onBeforeRequest",
          event: "onBeforeRequest",
          details: {
            type: "script",
            url: getURLHttpXHRJavaScript(),
            frameUrl: getURLHttpXHR()
          }
        },
        { label: "a.js-onBeforeSendHeaders",
          event: "onBeforeSendHeaders",
          details: {
            type: "script",
            url: getURLHttpXHRJavaScript(),
          }
        },
        { label: "a.js-onSendHeaders",
          event: "onSendHeaders",
          details: {
            type: "script",
            url: getURLHttpXHRJavaScript(),
          }
        },
        { label: "a.js-onHeadersReceived",
          event: "onHeadersReceived",
          details: {
            type: "script",
            url: getURLHttpXHRJavaScript(),
            statusLine: "HTTP/1.1 200 OK",
            statusCode: 200,
          }
        },
        { label: "a.js-onResponseStarted",
          event: "onResponseStarted",
          details: {
            type: "script",
            url: getURLHttpXHRJavaScript(),
            statusCode: 200,
            ip: "127.0.0.1",
            fromCache: false,
            statusLine: "HTTP/1.1 200 OK",
          }
        },
        { label: "a.js-onCompleted",
          event: "onCompleted",
          details: {
            type: "script",
            url: getURLHttpXHRJavaScript(),
            statusCode: 200,
            ip: "127.0.0.1",
            fromCache: false,
            statusLine: "HTTP/1.1 200 OK",
          }
        },
        { label: "onBeforeRequest-2",
          event: "onBeforeRequest",
          details: {
            type: "xmlhttprequest",
            url: getURLHttpXHRData(),
            frameUrl: getURLHttpXHR()
          }
        },
        { label: "onBeforeSendHeaders-2",
          event: "onBeforeSendHeaders",
          details: {
            type: "xmlhttprequest",
            url: getURLHttpXHRData(),
          }
        },
        { label: "onSendHeaders-2",
          event: "onSendHeaders",
          details: {
            type: "xmlhttprequest",
            url: getURLHttpXHRData(),
          }
        },
        { label: "onHeadersReceived-2",
          event: "onHeadersReceived",
          details: {
            type: "xmlhttprequest",
            url: getURLHttpXHRData(),
            statusLine: "HTTP/1.1 200 OK",
            statusCode: 200,
          }
        },
        { label: "onResponseStarted-2",
          event: "onResponseStarted",
          details: {
            type: "xmlhttprequest",
            url: getURLHttpXHRData(),
            statusCode: 200,
            ip: "127.0.0.1",
            fromCache: false,
            statusLine: "HTTP/1.1 200 OK",
          }
        },
        { label: "onCompleted-2",
          event: "onCompleted",
          details: {
            type: "xmlhttprequest",
            url: getURLHttpXHRData(),
            statusCode: 200,
            ip: "127.0.0.1",
            fromCache: false,
            statusLine: "HTTP/1.1 200 OK",
          }
        }
      ],
      [  // event order
        ["onBeforeRequest-1", "onBeforeSendHeaders-1", "onSendHeaders-1",
         "onHeadersReceived-1", "onResponseStarted-1", "onCompleted-1",
         "onBeforeRequest-2", "onBeforeSendHeaders-2", "onSendHeaders-2",
         "onHeadersReceived-2", "onResponseStarted-2", "onCompleted-2"] ]);
    navigateAndWait(getURLHttpXHR());
  },
]);
