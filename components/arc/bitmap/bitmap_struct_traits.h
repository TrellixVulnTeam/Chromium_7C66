// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENT_ARC_BITMAP_BITMAP_STRUCT_TRAITS_H_
#define COMPONENT_ARC_BITMAP_BITMAP_STRUCT_TRAITS_H_

#include "components/arc/common/bitmap.mojom.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace mojo {

template <>
struct StructTraits<arc::mojom::ArcBitmapDataView, SkBitmap> {
  static const mojo::CArray<uint8_t> pixel_data(const SkBitmap& r) {
    const SkImageInfo& info = r.info();
    DCHECK_EQ(info.colorType(), kRGBA_8888_SkColorType);

    return mojo::CArray<uint8_t>(
        r.getSize(), r.getSize(), static_cast<uint8_t*>(r.getPixels()));
  }
  static uint32_t width(const SkBitmap& r) { return r.width(); }
  static uint32_t height(const SkBitmap& r) { return r.height(); }

  static bool Read(arc::mojom::ArcBitmapDataView data, SkBitmap* out);
};

}

#endif  // COMPONENT_ARC_BITMAP_BITMAP_STRUCT_TRAITS_H_
