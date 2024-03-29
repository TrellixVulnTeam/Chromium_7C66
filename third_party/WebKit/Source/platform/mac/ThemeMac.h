/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ThemeMac_h
#define ThemeMac_h

#include "platform/Theme.h"

namespace blink {

class ThemeMac : public Theme {
 public:
  ThemeMac() {}
  ~ThemeMac() override {}

  virtual int baselinePositionAdjustment(ControlPart) const;

  virtual FontDescription controlFont(ControlPart,
                                      const FontDescription&,
                                      float zoomFactor) const;

  virtual LengthSize controlSize(ControlPart,
                                 const FontDescription&,
                                 const LengthSize&,
                                 float zoomFactor) const;
  virtual LengthSize minimumControlSize(ControlPart,
                                        const FontDescription&,
                                        float zoomFactor) const;

  virtual LengthBox controlPadding(ControlPart,
                                   const FontDescription&,
                                   const LengthBox& zoomedBox,
                                   float zoomFactor) const;
  virtual LengthBox controlBorder(ControlPart,
                                  const FontDescription&,
                                  const LengthBox& zoomedBox,
                                  float zoomFactor) const;

  virtual bool controlRequiresPreWhiteSpace(ControlPart part) const {
    return part == PushButtonPart;
  }

  virtual void addVisualOverflow(ControlPart,
                                 ControlStates,
                                 float zoomFactor,
                                 IntRect& borderBox) const;

  // Inflate an IntRect to accout for specific padding around margins.
  enum { TopMargin = 0, RightMargin = 1, BottomMargin = 2, LeftMargin = 3 };
  static PLATFORM_EXPORT IntRect inflateRect(const IntRect&,
                                             const IntSize&,
                                             const int* margins,
                                             float zoomLevel = 1.0f);

  // Inflate an IntRect to account for any bleeding that would happen due to
  // anti-aliasing.
  static PLATFORM_EXPORT IntRect inflateRectForAA(const IntRect&);

  // Inflate an IntRect to account for its focus ring.
  // TODO: Consider using computing the focus ring's bounds with
  // -[NSCell focusRingMaskBoundsForFrame:inView:]).
  static PLATFORM_EXPORT IntRect inflateRectForFocusRing(const IntRect&);

  static PLATFORM_EXPORT LengthSize checkboxSize(const FontDescription&,
                                                 const LengthSize& zoomedSize,
                                                 float zoomFactor);
  static PLATFORM_EXPORT NSButtonCell* checkbox(ControlStates,
                                                const IntRect& zoomedRect,
                                                float zoomFactor);
  static PLATFORM_EXPORT const IntSize* checkboxSizes();
  static PLATFORM_EXPORT const int* checkboxMargins(NSControlSize);
  static PLATFORM_EXPORT NSView* ensuredView(ScrollableArea*);

  static PLATFORM_EXPORT const IntSize* radioSizes();
  static PLATFORM_EXPORT const int* radioMargins(NSControlSize);
  static PLATFORM_EXPORT LengthSize radioSize(const FontDescription&,
                                              const LengthSize& zoomedSize,
                                              float zoomFactor);
  static PLATFORM_EXPORT NSButtonCell* radio(ControlStates,
                                             const IntRect& zoomedRect,
                                             float zoomFactor);

  static PLATFORM_EXPORT const IntSize* buttonSizes();
  static PLATFORM_EXPORT const int* buttonMargins(NSControlSize);
  static PLATFORM_EXPORT NSButtonCell* button(ControlPart,
                                              ControlStates,
                                              const IntRect& zoomedRect,
                                              float zoomFactor);

  static PLATFORM_EXPORT NSControlSize
  controlSizeFromPixelSize(const IntSize* sizes,
                           const IntSize& minZoomedSize,
                           float zoomFactor);
  static PLATFORM_EXPORT const IntSize* stepperSizes();
};

}  // namespace blink

#endif  // ThemeMac_h
