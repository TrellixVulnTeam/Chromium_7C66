// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cronet/Cronet.h>
#import <Foundation/Foundation.h>

#include <stdint.h>

#include "base/logging.h"
#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "components/cronet/ios/test/start_cronet.h"
#include "components/cronet/ios/test/test_server.h"
#include "components/grpc_support/test/quic_test_server.h"
#include "net/base/mac/url_conversions.h"
#include "net/base/net_errors.h"
#include "net/cert/mock_cert_verifier.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gtest_mac.h"

#include "url/gurl.h"

@interface TestDelegate : NSObject<NSURLSessionDataDelegate,
                                   NSURLSessionDelegate,
                                   NSURLSessionTaskDelegate>

// Completion semaphore for this TestDelegate. When the request this delegate is
// attached to finishes (either successfully or with an error), this delegate
// signals this semaphore.
@property(assign, nonatomic) dispatch_semaphore_t semaphore;

// Body of response received by the request this delegate is attached to.
@property(retain, nonatomic) NSString* responseBody;

// Error the request this delegate is attached to failed with, if any.
@property(retain, nonatomic) NSError* error;

@end

@implementation TestDelegate
@synthesize semaphore = _semaphore;
@synthesize responseBody = _responseBody;
@synthesize error = _error;

- (id)init {
  if (self = [super init]) {
    _semaphore = dispatch_semaphore_create(0);
  }
  return self;
}

- (void)dealloc {
  dispatch_release(_semaphore);
  [_error release];
  _error = nil;
  [super dealloc];
}

- (void)URLSession:(NSURLSession*)session
    didBecomeInvalidWithError:(NSError*)error {
}

- (void)URLSession:(NSURLSession*)session
                    task:(NSURLSessionTask*)task
    didCompleteWithError:(NSError*)error {
  [self setError:error];
  dispatch_semaphore_signal(_semaphore);
}

- (void)URLSession:(NSURLSession*)session
                   task:(NSURLSessionTask*)task
    didReceiveChallenge:(NSURLAuthenticationChallenge*)challenge
      completionHandler:
          (void (^)(NSURLSessionAuthChallengeDisposition disp,
                    NSURLCredential* credential))completionHandler {
  completionHandler(NSURLSessionAuthChallengeUseCredential, nil);
}

- (void)URLSession:(NSURLSession*)session
              dataTask:(NSURLSessionDataTask*)dataTask
    didReceiveResponse:(NSURLResponse*)response
     completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))
                           completionHandler {
  completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession*)session
          dataTask:(NSURLSessionDataTask*)dataTask
    didReceiveData:(NSData*)data {
  NSString* stringData =
      [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
  if (_responseBody == nil) {
    _responseBody = stringData;
  } else {
    _responseBody = [_responseBody stringByAppendingString:stringData];
  }
}

- (void)URLSession:(NSURLSession*)session
             dataTask:(NSURLSessionDataTask*)dataTask
    willCacheResponse:(NSCachedURLResponse*)proposedResponse
    completionHandler:
        (void (^)(NSCachedURLResponse* cachedResponse))completionHandler {
  completionHandler(proposedResponse);
}

@end

namespace cronet {
// base::TimeDelta would normally be ideal for this but it does not support
// nanosecond resolution.
static const int64_t ns_in_second = 1000000000LL;
const char kUserAgent[] = "CronetTest/1.0.0.0";

class HttpTest : public ::testing::Test {
 protected:
  HttpTest() {}
  ~HttpTest() override {}

  void SetUp() override {
    grpc_support::StartQuicTestServer();
    TestServer::Start();

    [Cronet setRequestFilterBlock:^(NSURLRequest* request) {
      return YES;
    }];
    StartCronetIfNecessary(grpc_support::GetQuicTestServerPort());
    [Cronet registerHttpProtocolHandler];
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    [Cronet installIntoSessionConfiguration:config];
    delegate_.reset([[TestDelegate alloc] init]);
    NSURLSession* session = [NSURLSession sessionWithConfiguration:config
                                                          delegate:delegate_
                                                     delegateQueue:nil];
    // Take a reference to the session and store it so it doesn't get
    // deallocated until this object does.
    session_.reset([session retain]);
  }

  void TearDown() override {
    grpc_support::ShutdownQuicTestServer();
    TestServer::Shutdown();
  }

  // Launches the supplied |task| and blocks until it completes, with a timeout
  // of 1 second.
  void StartDataTaskAndWaitForCompletion(NSURLSessionDataTask* task) {
    [task resume];
    int64_t deadline_ns = 1 * ns_in_second;
    dispatch_semaphore_wait([delegate_ semaphore],
                            dispatch_time(DISPATCH_TIME_NOW, deadline_ns));
  }

  base::scoped_nsobject<NSURLSession> session_;
  base::scoped_nsobject<TestDelegate> delegate_;
};

TEST_F(HttpTest, NSURLSessionReceivesData) {
  NSURL* url = net::NSURLWithGURL(GURL(grpc_support::kTestServerUrl));
  __block BOOL block_used = NO;
  NSURLSessionDataTask* task = [session_ dataTaskWithURL:url];
  [Cronet setRequestFilterBlock:^(NSURLRequest* request) {
    block_used = YES;
    EXPECT_EQ([request URL], url);
    return YES;
  }];
  StartDataTaskAndWaitForCompletion(task);
  EXPECT_TRUE(block_used);
  EXPECT_EQ(nil, [delegate_ error]);
  EXPECT_STREQ(grpc_support::kHelloBodyValue,
               base::SysNSStringToUTF8([delegate_ responseBody]).c_str());
}

TEST_F(HttpTest, GetGlobalMetricsDeltas) {
  NSData* delta1 = [Cronet getGlobalMetricsDeltas];

  NSURL* url = net::NSURLWithGURL(GURL(grpc_support::kTestServerUrl));
  NSURLSessionDataTask* task = [session_ dataTaskWithURL:url];
  StartDataTaskAndWaitForCompletion(task);
  EXPECT_EQ(nil, [delegate_ error]);
  EXPECT_STREQ(grpc_support::kHelloBodyValue,
               base::SysNSStringToUTF8([delegate_ responseBody]).c_str());

  NSData* delta2 = [Cronet getGlobalMetricsDeltas];
  EXPECT_FALSE([delta2 isEqualToData:delta1]);
}

TEST_F(HttpTest, SdchDisabledByDefault) {
  NSURL* url =
      net::NSURLWithGURL(GURL(TestServer::GetEchoHeaderURL("Accept-Encoding")));
  NSURLSessionDataTask* task = [session_ dataTaskWithURL:url];
  StartDataTaskAndWaitForCompletion(task);
  EXPECT_EQ(nil, [delegate_ error]);
  EXPECT_FALSE([[delegate_ responseBody] containsString:@"sdch"]);
}

TEST_F(HttpTest, NSURLSessionAcceptLanguage) {
  NSURL* url =
      net::NSURLWithGURL(GURL(TestServer::GetEchoHeaderURL("Accept-Language")));
  NSURLSessionDataTask* task = [session_ dataTaskWithURL:url];
  StartDataTaskAndWaitForCompletion(task);
  EXPECT_EQ(nil, [delegate_ error]);
  ASSERT_STREQ("en-US,en", [[delegate_ responseBody] UTF8String]);
}

TEST_F(HttpTest, SetUserAgentIsExact) {
  NSURL* url =
      net::NSURLWithGURL(GURL(TestServer::GetEchoHeaderURL("User-Agent")));
  [Cronet setRequestFilterBlock:nil];
  NSURLSessionDataTask* task = [session_ dataTaskWithURL:url];
  StartDataTaskAndWaitForCompletion(task);
  EXPECT_EQ(nil, [delegate_ error]);
  EXPECT_STREQ(kUserAgent, [[delegate_ responseBody] UTF8String]);
}

TEST_F(HttpTest, FilterOutRequest) {
  NSURL* url =
      net::NSURLWithGURL(GURL(TestServer::GetEchoHeaderURL("User-Agent")));
  __block BOOL block_used = NO;
  NSURLSessionDataTask* task = [session_ dataTaskWithURL:url];
  [Cronet setRequestFilterBlock:^(NSURLRequest* request) {
    block_used = YES;
    EXPECT_EQ([request URL], url);
    return NO;
  }];
  StartDataTaskAndWaitForCompletion(task);
  EXPECT_TRUE(block_used);
  EXPECT_EQ(nil, [delegate_ error]);
  EXPECT_FALSE([[delegate_ responseBody]
      containsString:base::SysUTF8ToNSString(kUserAgent)]);
  EXPECT_TRUE([[delegate_ responseBody] containsString:@"CFNetwork"]);
}

}  // namespace cronet
