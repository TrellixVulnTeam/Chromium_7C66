// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/mac/scoped_nsobject.h"
#import "ui/base/cocoa/tracking_area.h"
#import "ui/gfx/test/ui_cocoa_test_helper.h"

// A test object that counts the number of times a message is sent to it.
@interface TestTrackingAreaOwner : NSObject {
 @private
  NSUInteger messageCount_;
}
@property(nonatomic, assign) NSUInteger messageCount;
- (void)performMessage;
@end

@implementation TestTrackingAreaOwner
@synthesize messageCount = messageCount_;
- (void)performMessage {
  ++messageCount_;
}
@end

namespace ui {

class CrTrackingAreaTest : public CocoaTest {
 public:
  CrTrackingAreaTest()
      : owner_([[TestTrackingAreaOwner alloc] init]),
        trackingArea_([[CrTrackingArea alloc]
            initWithRect:NSMakeRect(0, 0, 100, 100)
                 options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
                   owner:owner_.get()
                userInfo:nil]) {
  }

  base::scoped_nsobject<TestTrackingAreaOwner> owner_;
  base::scoped_nsobject<CrTrackingArea> trackingArea_;
};

TEST_F(CrTrackingAreaTest, OwnerForwards) {
  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(1U, [owner_ messageCount]);

  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(2U, [owner_ messageCount]);
}

TEST_F(CrTrackingAreaTest, OwnerStopsForwarding) {
  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(1U, [owner_ messageCount]);

  [trackingArea_ clearOwner];

  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(1U, [owner_ messageCount]);
}

TEST_F(CrTrackingAreaTest, OwnerAutomaticallyStopsForwardingOnClose) {
  [test_window() orderFront:nil];
  [trackingArea_ clearOwnerWhenWindowWillClose:test_window()];

  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(1U, [owner_ messageCount]);

  [test_window() close];

  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(1U, [owner_ messageCount]);
}

TEST_F(CrTrackingAreaTest, ScoperInit) {
  {
    ScopedCrTrackingArea scoper([trackingArea_ retain]);
    [[scoper.get() owner] performMessage];
    EXPECT_EQ(1U, [owner_ messageCount]);
  }

  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(1U, [owner_ messageCount]);
}

TEST_F(CrTrackingAreaTest, ScoperReset) {
  {
    ScopedCrTrackingArea scoper;
    EXPECT_FALSE(scoper.get());

    scoper.reset([trackingArea_ retain]);
    [[scoper.get() owner] performMessage];
    EXPECT_EQ(1U, [owner_ messageCount]);

    [[scoper.get() owner] performMessage];
    EXPECT_EQ(2U, [owner_ messageCount]);
  }

  [[trackingArea_ owner] performMessage];
  EXPECT_EQ(2U, [owner_ messageCount]);
}

}  // namespace ui
