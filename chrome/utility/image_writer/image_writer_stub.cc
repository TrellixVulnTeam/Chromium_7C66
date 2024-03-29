// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/utility/image_writer/image_writer.h"

// This file contains the default version of the platform-specific methods of
// the ImageWriter.  Add new platforms by creating a new version of these
// methods and updating the compliation rules appropriately.
namespace image_writer {

bool ImageWriter::IsValidDevice() {
  NOTIMPLEMENTED();
  return false;
}

void ImageWriter::UnmountVolumes(const base::Closure& continuation) {
  NOTIMPLEMENTED();
  return;
}

bool ImageWriter::OpenDevice() {
  device_file_.Initialize(
      device_path_,
      base::File::FLAG_OPEN | base::File::FLAG_READ | base::File::FLAG_WRITE |
          base::File::FLAG_EXCLUSIVE_READ | base::File::FLAG_EXCLUSIVE_WRITE);
  return device_file_.IsValid();
}

}  // namespace image_writer
