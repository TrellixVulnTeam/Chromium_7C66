// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/webapk/webapk_web_manifest_checker.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/common/manifest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/WebKit/public/platform/WebDisplayMode.h"
#include "url/gurl.h"

namespace {

base::NullableString16 ToNullableUTF16(const std::string& str) {
  return base::NullableString16(base::UTF8ToUTF16(str), false);
}

content::Manifest GetValidManifest() {
  content::Manifest manifest;
  manifest.name = ToNullableUTF16("foo");
  manifest.short_name = ToNullableUTF16("bar");
  manifest.start_url = GURL("http://example.com");
  manifest.display = blink::WebDisplayModeStandalone;

  content::Manifest::Icon icon;
  icon.type = base::ASCIIToUTF16("image/png");
  icon.sizes.push_back(gfx::Size(144, 144));
  manifest.icons.push_back(icon);

  return manifest;
}

}  // anonymous namespace

TEST(WebApkWebManifestCheckerTest, Compatible) {
  content::Manifest manifest = GetValidManifest();
  EXPECT_TRUE(AreWebManifestUrlsWebApkCompatible(manifest));
}

TEST(WebApkWebManifestCheckerTest, CompatibleURLHasNoPassword) {
  const GURL kUrlWithPassword("http://answer:42@life/universe/and/everything");

  content::Manifest manifest = GetValidManifest();
  manifest.start_url = kUrlWithPassword;
  EXPECT_FALSE(AreWebManifestUrlsWebApkCompatible(manifest));

  manifest = GetValidManifest();
  manifest.scope = kUrlWithPassword;
  EXPECT_FALSE(AreWebManifestUrlsWebApkCompatible(manifest));

  manifest = GetValidManifest();
  manifest.icons[0].src = kUrlWithPassword;
  EXPECT_FALSE(AreWebManifestUrlsWebApkCompatible(manifest));
}
