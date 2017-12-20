// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/location_bar/autocomplete_text_field_cell.h"

#include <stddef.h>

#include "base/logging.h"
#include "base/mac/foundation_util.h"
#include "base/mac/mac_logging.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/themes/theme_service.h"
#import "chrome/browser/ui/cocoa/location_bar/autocomplete_text_field.h"
#import "chrome/browser/ui/cocoa/location_bar/location_bar_decoration.h"
#import "chrome/browser/ui/cocoa/themed_window.h"
#include "chrome/grit/theme_resources.h"
#import "extensions/common/feature_switch.h"
#import "third_party/mozilla/NSPasteboard+Utils.h"
#import "ui/base/cocoa/appkit_utils.h"
#import "ui/base/cocoa/tracking_area.h"
#import "ui/base/cocoa/nsview_additions.h"
#include "ui/base/cocoa/scoped_cg_context_smooth_fonts.h"
#include "ui/base/material_design/material_design_controller.h"

using extensions::FeatureSwitch;

namespace {

// Matches the clipping radius of |GradientButtonCell|.
const CGFloat kCornerRadius = 3.0;

// How far to inset the left- and right-hand decorations from the field's
// bounds.
const CGFloat kRightDecorationXOffset = 2.0;
const CGFloat kLeftDecorationXOffset = 1.0;

// How much the text frame needs to overlap the rightmost left decoration.
const CGFloat kTextFrameDecorationOverlap = 5.0;

// How long to wait for mouse-up on the location icon before assuming
// that the user wants to drag.
const NSTimeInterval kLocationIconDragTimeout = 0.25;

// Calculate the positions for a set of decorations.  |frame| is the
// overall frame to do layout in, |remaining_frame| will get the
// left-over space.  |all_decorations| is the set of decorations to
// lay out, |decorations| will be set to the decorations which are
// visible and which fit, in the same order as |all_decorations|,
// while |decoration_frames| will be the corresponding frames.
// |x_edge| describes the edge to layout the decorations against
// (|NSMinXEdge| or |NSMaxXEdge|).  |regular_padding| is the padding
// from the edge of |cell_frame| to use when the first visible decoration
// is a regular decoration.
void CalculatePositionsHelper(
    NSRect frame,
    const std::vector<LocationBarDecoration*>& all_decorations,
    NSRectEdge x_edge,
    CGFloat regular_padding,
    std::vector<LocationBarDecoration*>* decorations,
    std::vector<NSRect>* decoration_frames,
    NSRect* remaining_frame) {
  DCHECK(x_edge == NSMinXEdge || x_edge == NSMaxXEdge);
  DCHECK_EQ(decorations->size(), decoration_frames->size());

  // The initial padding depends on whether the first visible decoration is
  // a button or not.
  bool is_first_visible_decoration = true;

  for (size_t i = 0; i < all_decorations.size(); ++i) {
    if (all_decorations[i]->IsVisible()) {
      CGFloat padding = 0;
      if (is_first_visible_decoration) {
        padding = regular_padding;
        is_first_visible_decoration = false;
      }

      NSRect padding_rect, available;

      // Peel off the outside padding.
      NSDivideRect(frame, &padding_rect, &available, padding, x_edge);

      // Find out how large the decoration will be in the remaining
      // space.
      const CGFloat used_width =
          all_decorations[i]->GetWidthForSpace(NSWidth(available));

      if (used_width != LocationBarDecoration::kOmittedWidth) {
        DCHECK_GT(used_width, 0.0);
        NSRect decoration_frame;

        // Peel off the desired width, leaving the remainder in
        // |frame|.
        NSDivideRect(available, &decoration_frame, &frame,
                     used_width, x_edge);

        decorations->push_back(all_decorations[i]);
        decoration_frames->push_back(decoration_frame);
        DCHECK_EQ(decorations->size(), decoration_frames->size());
      }
    }
  }

  DCHECK_EQ(decorations->size(), decoration_frames->size());
  *remaining_frame = frame;
}

// Helper function for calculating placement of decorations w/in the cell.
// |frame| is the cell's boundary rectangle, |remaining_frame| will get any
// space left after decorations are laid out (for text).  |left_decorations| is
// a set of decorations for the left-hand side of the cell, |right_decorations|
// for the right-hand side.
// |decorations| will contain the resulting visible decorations, and
// |decoration_frames| will contain their frames in the same coordinates as
// |frame|.  Decorations will be ordered left to right. As a convenience returns
// the index of the first right-hand decoration.
size_t CalculatePositionsInFrame(
    NSRect frame,
    const std::vector<LocationBarDecoration*>& left_decorations,
    const std::vector<LocationBarDecoration*>& right_decorations,
    std::vector<LocationBarDecoration*>* decorations,
    std::vector<NSRect>* decoration_frames,
    NSRect* remaining_frame) {
  decorations->clear();
  decoration_frames->clear();

  // Layout |left_decorations| against the LHS.
  CalculatePositionsHelper(frame, left_decorations, NSMinXEdge,
                           kLeftDecorationXOffset, decorations,
                           decoration_frames, &frame);
  DCHECK_EQ(decorations->size(), decoration_frames->size());

  // Capture the number of visible left-hand decorations.
  const size_t left_count = decorations->size();

  // Extend the text frame so that it slightly overlaps the rightmost left
  // decoration.
  if (left_count) {
    frame.origin.x -= kTextFrameDecorationOverlap;
    frame.size.width += kTextFrameDecorationOverlap;
  }

  // Layout |right_decorations| against the RHS.
  CalculatePositionsHelper(frame, right_decorations, NSMaxXEdge,
                           kRightDecorationXOffset, decorations,
                           decoration_frames, &frame);
  DCHECK_EQ(decorations->size(), decoration_frames->size());

  // Reverse the right-hand decorations so that overall everything is
  // sorted left to right.
  std::reverse(decorations->begin() + left_count, decorations->end());
  std::reverse(decoration_frames->begin() + left_count,
               decoration_frames->end());

  *remaining_frame = frame;
  return left_count;
}

}  // namespace

@implementation AutocompleteTextFieldCell

@synthesize isPopupMode = isPopupMode_;
@synthesize singlePixelLineWidth = singlePixelLineWidth_;

- (CGFloat)topTextFrameOffset {
  return 3.0;
}

- (CGFloat)bottomTextFrameOffset {
  return 3.0;
}

- (CGFloat)cornerRadius {
  return kCornerRadius;
}

- (BOOL)shouldDrawBezel {
  return YES;
}

- (CGFloat)lineHeight {
  return 17;
}

- (void)clearTrackingArea {
  for (auto& decoration : mouseTrackingDecorations_)
    decoration->RemoveTrackingArea();

  mouseTrackingDecorations_.clear();
}

- (void)clearDecorations {
  leftDecorations_.clear();
  rightDecorations_.clear();
  [self clearTrackingArea];
}

- (void)addLeftDecoration:(LocationBarDecoration*)decoration {
  leftDecorations_.push_back(decoration);
}

- (void)addRightDecoration:(LocationBarDecoration*)decoration {
  rightDecorations_.push_back(decoration);
}

- (CGFloat)availableWidthInFrame:(const NSRect)frame {
  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect textFrame;
  CalculatePositionsInFrame(frame, leftDecorations_, rightDecorations_,
                            &decorations, &decorationFrames, &textFrame);

  return NSWidth(textFrame);
}

- (NSRect)frameForDecoration:(const LocationBarDecoration*)aDecoration
                     inFrame:(NSRect)cellFrame {
  // Short-circuit if the decoration is known to be not visible.
  if (aDecoration && !aDecoration->IsVisible())
    return NSZeroRect;

  // Layout the decorations.
  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect textFrame;
  CalculatePositionsInFrame(cellFrame, leftDecorations_, rightDecorations_,
                            &decorations, &decorationFrames, &textFrame);

  // Find our decoration and return the corresponding frame.
  std::vector<LocationBarDecoration*>::const_iterator iter =
      std::find(decorations.begin(), decorations.end(), aDecoration);
  if (iter != decorations.end()) {
    const size_t index = iter - decorations.begin();
    return decorationFrames[index];
  }

  // Decorations which are not visible should have been filtered out
  // at the top, but return |NSZeroRect| rather than a 0-width rect
  // for consistency.
  NOTREACHED();
  return NSZeroRect;
}

// Overriden to account for the decorations.
- (NSRect)textFrameForFrame:(NSRect)cellFrame {
  // Get the frame adjusted for decorations.
  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect textFrame = [super textFrameForFrame:cellFrame];
  CalculatePositionsInFrame(textFrame, leftDecorations_, rightDecorations_,
                            &decorations, &decorationFrames, &textFrame);

  // The text needs to be slightly higher than its default position to match the
  // Material Design spec. It turns out this adjustment is equal to the single
  // pixel line width (so 1 on non-Retina, 0.5 on Retina). Make this adjustment
  // after computing decoration positions because the decorations are already
  // correctly positioned. The spec also calls for positioning the text 1pt to
  // the right of its default position.
  textFrame.origin.x += 1;
  textFrame.size.width -= 1;
  textFrame.origin.y -= singlePixelLineWidth_;

  // NOTE: This function must closely match the logic in
  // |-drawInteriorWithFrame:inView:|.

  return textFrame;
}

- (NSRect)textCursorFrameForFrame:(NSRect)cellFrame {
  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect textFrame;
  size_t left_count =
      CalculatePositionsInFrame(cellFrame, leftDecorations_, rightDecorations_,
                                &decorations, &decorationFrames, &textFrame);

  // Determine the left-most extent for the i-beam cursor.
  CGFloat minX = NSMinX(textFrame);
  for (size_t index = left_count; index--; ) {
    if (decorations[index]->AcceptsMousePress())
      break;

    // If at leftmost decoration, expand to edge of cell.
    if (!index)
      minX = NSMinX(cellFrame);
    else
      minX = NSMinX(decorationFrames[index]);
  }

  // Determine the right-most extent for the i-beam cursor.
  CGFloat maxX = NSMaxX(textFrame);
  for (size_t index = left_count; index < decorations.size(); ++index) {
    if (decorations[index]->AcceptsMousePress())
      break;

    // If at rightmost decoration, expand to edge of cell.
    if (index == decorations.size() - 1)
      maxX = NSMaxX(cellFrame);
    else
      maxX = NSMaxX(decorationFrames[index]);
  }

  // I-beam cursor covers left-most to right-most.
  return NSMakeRect(minX, NSMinY(textFrame), maxX - minX, NSHeight(textFrame));
}

- (void)drawWithFrame:(NSRect)frame inView:(NSView*)controlView {
  BOOL inDarkMode = [[controlView window] inIncognitoModeWithSystemTheme];
  BOOL showingFirstResponder = [self showsFirstResponder];
  // Adjust the inset by 1/2 the line width to get a crisp line (screen pixels
  // lay between cooridnate space lines).
  CGFloat insetSize = 1 - singlePixelLineWidth_ / 2.;
  if (showingFirstResponder && !inDarkMode) {
    insetSize++;
  }

  // Compute the border's bezier path.
  NSRect pathRect = NSInsetRect(frame, insetSize, insetSize);
  NSBezierPath* path =
      [NSBezierPath bezierPathWithRoundedRect:pathRect
                                      xRadius:kCornerRadius
                                      yRadius:kCornerRadius];
  [path setLineWidth:showingFirstResponder ? singlePixelLineWidth_ * 2
                                           : singlePixelLineWidth_];

  // Fill the background.
  [[self backgroundColor] set];
  if (isPopupMode_) {
    NSRectFillUsingOperation(NSInsetRect(frame, 1, 1), NSCompositeSourceOver);
  } else {
    [path fill];
  }

  // Draw the border.
  if (!inDarkMode) {
    const CGFloat kNormalStrokeGray = 168 / 255.;
    [[NSColor colorWithCalibratedWhite:kNormalStrokeGray alpha:1] set];
  } else {
    const CGFloat k30PercentAlpha = 0.3;
    [[NSColor colorWithCalibratedWhite:0 alpha:k30PercentAlpha] set];
  }
  [path stroke];

  // Draw the interior contents. We do this after drawing the border as some
  // of the interior controls draw over it.
  [self drawInteriorWithFrame:frame inView:controlView];

  // Draw the focus ring.
  if (showingFirstResponder) {
    CGFloat alphaComponent = 0.5 / singlePixelLineWidth_;
    if (inDarkMode) {
      // Special focus color for Material Incognito.
      [[NSColor colorWithSRGBRed:123 / 255.
                           green:170 / 255.
                            blue:247 / 255.
                           alpha:1] set];
    } else {
      [[[NSColor keyboardFocusIndicatorColor]
          colorWithAlphaComponent:alphaComponent] set];
    }
    [path stroke];
  }
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView*)controlView {
  ui::ScopedCGContextSmoothFonts fontSmoothing;
  [super drawInteriorWithFrame:cellFrame inView:controlView];

  // NOTE: This method must closely match the logic in |-textFrameForFrame:|.
  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect workingFrame;

  CalculatePositionsInFrame(cellFrame, leftDecorations_, rightDecorations_,
                            &decorations, &decorationFrames, &workingFrame);

  // Draw the decorations. Do this after drawing the interior because the
  // field editor's background rect overlaps the right edge of the security
  // decoration's hover rounded rect.
  for (size_t i = 0; i < decorations.size(); ++i) {
    if (decorations[i]) {
      decorations[i]->DrawWithBackgroundInFrame(decorationFrames[i],
                                                controlView);
    }
  }
}

- (BOOL)canDropAtLocationInWindow:(NSPoint)location
                           ofView:(AutocompleteTextField*)controlView {
  NSRect cellFrame = [controlView bounds];
  const NSPoint locationInView =
      [controlView convertPoint:location fromView:nil];

  // If we have decorations, the drop can't occur at their horizontal padding.
  if (!leftDecorations_.empty() && locationInView.x < kLeftDecorationXOffset)
    return false;

  if (!rightDecorations_.empty() &&
      locationInView.x > NSWidth(cellFrame) - kRightDecorationXOffset) {
    return false;
  }

  LocationBarDecoration* decoration =
      [self decorationForLocationInWindow:location
                                   inRect:cellFrame
                                   ofView:controlView];
  return !decoration;
}

- (LocationBarDecoration*)decorationForEvent:(NSEvent*)theEvent
                                      inRect:(NSRect)cellFrame
                                      ofView:(AutocompleteTextField*)controlView
{
  return [self decorationForLocationInWindow:[theEvent locationInWindow]
                                      inRect:cellFrame
                                      ofView:controlView];
}

- (LocationBarDecoration*)decorationForLocationInWindow:(NSPoint)location
                                                 inRect:(NSRect)cellFrame
                                                 ofView:(AutocompleteTextField*)
                                                            controlView {
  const BOOL flipped = [controlView isFlipped];
  const NSPoint locationInView =
      [controlView convertPoint:location fromView:nil];

  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect textFrame;
  CalculatePositionsInFrame(cellFrame, leftDecorations_, rightDecorations_,
                            &decorations, &decorationFrames, &textFrame);

  for (size_t i = 0; i < decorations.size(); ++i) {
    if (NSMouseInRect(locationInView, decorationFrames[i], flipped))
      return decorations[i];
  }

  return NULL;
}

- (NSMenu*)decorationMenuForEvent:(NSEvent*)theEvent
                           inRect:(NSRect)cellFrame
                           ofView:(AutocompleteTextField*)controlView {
  LocationBarDecoration* decoration =
      [self decorationForEvent:theEvent inRect:cellFrame ofView:controlView];
  if (decoration)
    return decoration->GetMenu();
  return nil;
}

- (BOOL)mouseDown:(NSEvent*)theEvent
           inRect:(NSRect)cellFrame
           ofView:(AutocompleteTextField*)controlView {
  // TODO(groby): Factor this into three pieces - find target for event, handle
  // delayed focus (for any and all events), execute mouseDown for target.

  // Check if this mouseDown was the reason the control became firstResponder.
  // If not, discard focus event.
  base::scoped_nsobject<NSEvent> focusEvent(focusEvent_.release());
  if (![theEvent isEqual:focusEvent])
    focusEvent.reset();

  LocationBarDecoration* decoration =
      [self decorationForEvent:theEvent inRect:cellFrame ofView:controlView];
  if (!decoration || !decoration->AcceptsMousePress())
    return NO;

  decoration->OnMouseDown();

  NSRect decorationRect =
      [self frameForDecoration:decoration inFrame:cellFrame];

  // If the decoration is draggable, then initiate a drag if the user
  // drags or holds the mouse down for awhile.
  if (decoration->IsDraggable()) {
    NSDate* timeout =
        [NSDate dateWithTimeIntervalSinceNow:kLocationIconDragTimeout];
    NSEvent* event = [NSApp nextEventMatchingMask:(NSLeftMouseDraggedMask |
                                                   NSLeftMouseUpMask)
                                        untilDate:timeout
                                           inMode:NSEventTrackingRunLoopMode
                                          dequeue:YES];
    if (!event || [event type] == NSLeftMouseDragged) {
      NSPasteboard* pboard = decoration->GetDragPasteboard();
      DCHECK(pboard);

      NSImage* image = decoration->GetDragImage();
      DCHECK(image);

      NSRect dragImageRect = decoration->GetDragImageFrame(decorationRect);

      // Center under mouse horizontally, with cursor below so the image
      // can be seen.
      const NSPoint mousePoint =
          [controlView convertPoint:[theEvent locationInWindow] fromView:nil];
      dragImageRect.origin =
          NSMakePoint(mousePoint.x - NSWidth(dragImageRect) / 2.0,
                      mousePoint.y - NSHeight(dragImageRect));

      // -[NSView dragImage:at:*] wants the images lower-left point,
      // regardless of -isFlipped.  Converting the rect to window base
      // coordinates doesn't require any special-casing.  Note that
      // -[NSView dragFile:fromRect:*] takes a rect rather than a
      // point, likely for this exact reason.
      const NSPoint dragPoint =
          [controlView convertRect:dragImageRect toView:nil].origin;
      [[controlView window] dragImage:image
                                   at:dragPoint
                               offset:NSZeroSize
                                event:theEvent
                           pasteboard:pboard
                               source:self
                            slideBack:YES];

      return YES;
    }

    // On mouse-up fall through to mouse-pressed case.
    DCHECK_EQ([event type], NSLeftMouseUp);
  }

  const NSPoint mouseLocation = [theEvent locationInWindow];
  const NSPoint point = [controlView convertPoint:mouseLocation fromView:nil];
  return decoration->OnMousePressed(
      decorationRect, NSMakePoint(point.x - decorationRect.origin.x,
                                  point.y - decorationRect.origin.y));
}

- (void)mouseUp:(NSEvent*)theEvent
         inRect:(NSRect)cellFrame
         ofView:(AutocompleteTextField*)controlView {
  LocationBarDecoration* decoration =
      [self decorationForEvent:theEvent inRect:cellFrame ofView:controlView];
  if (decoration)
    decoration->OnMouseUp();
}

// Returns the file path for file |name| if saved at NSURL |base|.
static NSString* PathWithBaseURLAndName(NSURL* base, NSString* name) {
  NSString* filteredName =
      [name stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
  return [[NSURL URLWithString:filteredName relativeToURL:base] path];
}

// Returns if there is already a file |name| at dir NSURL |base|.
static BOOL FileAlreadyExists(NSURL* base, NSString* name) {
  NSString* path = PathWithBaseURLAndName(base, name);
  DCHECK([path hasSuffix:name]);
  return [[NSFileManager defaultManager] fileExistsAtPath:path];
}

// Takes a destination URL, a suggested file name, & an extension (eg .webloc).
// Returns the complete file name with extension you should use.
// The name returned will not contain /, : or ?, will not start with a dot,
// will not be longer than kMaxNameLength + length of extension, and will not
// be a file name that already exists in that directory. If necessary it will
// try appending a space and a number to the name (but before the extension)
// trying numbers up to and including kMaxIndex.
// If the function gives up it returns nil.
static NSString* UnusedLegalNameForNewDropFile(NSURL* saveLocation,
                                               NSString *fileName,
                                               NSString *extension) {
  int number = 1;
  const int kMaxIndex = 20;
  const unsigned kMaxNameLength = 64; // Arbitrary.

  NSString* filteredName = [fileName stringByReplacingOccurrencesOfString:@"/"
                                                               withString:@"-"];
  filteredName = [filteredName stringByReplacingOccurrencesOfString:@":"
                                                         withString:@"-"];
  filteredName = [filteredName stringByReplacingOccurrencesOfString:@"?"
                                                         withString:@"-"];
  filteredName =
      [filteredName stringByReplacingOccurrencesOfString:@"."
                                              withString:@"-"
                                                 options:0
                                                   range:NSMakeRange(0,1)];

  if ([filteredName length] > kMaxNameLength)
    filteredName = [filteredName substringToIndex:kMaxNameLength];

  NSString* candidateName = [filteredName stringByAppendingString:extension];

  while (FileAlreadyExists(saveLocation, candidateName)) {
    if (number > kMaxIndex)
      return nil;
    else
      candidateName = [filteredName stringByAppendingFormat:@" %d%@",
                       number++, extension];
  }

  return candidateName;
}

- (NSArray*)namesOfPromisedFilesDroppedAtDestination:(NSURL*)dropDestination {
  NSPasteboard* pboard = [NSPasteboard pasteboardWithName:NSDragPboard];
  NSFileManager* fileManager = [NSFileManager defaultManager];

  if (![pboard containsURLDataConvertingTextToURL:YES])
    return NULL;

  NSArray *urls = NULL;
  NSArray* titles = NULL;
  [pboard getURLs:&urls
                andTitles:&titles
      convertingFilenames:YES
      convertingTextToURL:YES];

  NSString* urlStr = [urls objectAtIndex:0];
  NSString* nameStr = [titles objectAtIndex:0];

  NSString* nameWithExtensionStr =
      UnusedLegalNameForNewDropFile(dropDestination, nameStr, @".webloc");
  if (!nameWithExtensionStr)
    return NULL;

  NSDictionary* urlDict = [NSDictionary dictionaryWithObject:urlStr
                                                      forKey:@"URL"];
  NSURL* outputURL =
      [NSURL fileURLWithPath:PathWithBaseURLAndName(dropDestination,
                                                    nameWithExtensionStr)];
  [urlDict writeToURL:outputURL
           atomically:NO];

  if (![fileManager fileExistsAtPath:[outputURL path]])
    return NULL;

  NSDictionary* attr = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithBool:YES], NSFileExtensionHidden,
      [NSNumber numberWithUnsignedLong:'ilht'], NSFileHFSTypeCode,
      [NSNumber numberWithUnsignedLong:'MACS'], NSFileHFSCreatorCode,
      nil];
  [fileManager setAttributes:attr
                ofItemAtPath:[outputURL path]
                       error:nil];

  return [NSArray arrayWithObject:nameWithExtensionStr];
}

- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal {
  return NSDragOperationCopy;
}

- (void)updateMouseTrackingAndToolTipsInRect:(NSRect)cellFrame
                                      ofView:
                                          (AutocompleteTextField*)controlView {
  std::vector<LocationBarDecoration*> decorations;
  std::vector<NSRect> decorationFrames;
  NSRect textFrame;
  CalculatePositionsInFrame(cellFrame, leftDecorations_, rightDecorations_,
                            &decorations, &decorationFrames, &textFrame);
  [self clearTrackingArea];

  for (size_t i = 0; i < decorations.size(); ++i) {
    CrTrackingArea* trackingArea =
        decorations[i]->SetupTrackingArea(decorationFrames[i], controlView);
    if (trackingArea)
      mouseTrackingDecorations_.push_back(decorations[i]);

    NSString* tooltip = decorations[i]->GetToolTip();
    if ([tooltip length] > 0)
      [controlView addToolTip:tooltip forRect:decorationFrames[i]];
  }
}

- (BOOL)hideFocusState {
  return hideFocusState_;
}

- (void)setHideFocusState:(BOOL)hideFocusState
                   ofView:(AutocompleteTextField*)controlView {
  if (hideFocusState_ == hideFocusState)
    return;
  hideFocusState_ = hideFocusState;
  [controlView setNeedsDisplay:YES];
  NSTextView* fieldEditor =
      base::mac::ObjCCastStrict<NSTextView>([controlView currentEditor]);
  [fieldEditor updateInsertionPointStateAndRestartTimer:YES];
}

- (BOOL)showsFirstResponder {
  return [super showsFirstResponder] && !hideFocusState_;
}

- (void)handleFocusEvent:(NSEvent*)event
                  ofView:(AutocompleteTextField*)controlView {
  if ([controlView observer]) {
    const bool controlDown = ([event modifierFlags] & NSControlKeyMask) != 0;
    [controlView observer]->OnSetFocus(controlDown);
  }
}

@end

@implementation AutocompleteTextFieldCell (TestingAPI)

- (const std::vector<LocationBarDecoration*>&)mouseTrackingDecorations {
  return mouseTrackingDecorations_;
}

@end
