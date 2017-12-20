// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include <algorithm>
#include <sstream>
#include <string>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/logging.h"
#include "base/strings/string_split.h"
#include "content/browser/download/quarantine.h"
#include "content/browser/download/quarantine_constants_linux.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace content {
namespace {

using std::istringstream;
using std::string;
using std::vector;

class QuarantineLinuxTest : public testing::Test {
 public:
  QuarantineLinuxTest()
      : source_url_("http://www.source.com"),
        referrer_url_("http://www.referrer.com"),
        is_xattr_supported_(false) {}

  const base::FilePath& test_file() const { return test_file_; }

  const GURL& source_url() const { return source_url_; }

  const GURL& referrer_url() const { return referrer_url_; }

  bool is_xattr_supported() const { return is_xattr_supported_; }

 protected:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    ASSERT_TRUE(
        base::CreateTemporaryFileInDir(temp_dir_.GetPath(), &test_file_));
    int result =
        setxattr(test_file_.value().c_str(), "user.test", "test", 4, 0);
    is_xattr_supported_ = (!result) || (errno != ENOTSUP);
    if (!is_xattr_supported_) {
      DVLOG(0) << "Test will be skipped because extended attributes are not "
               << "supported on this OS/file system.";
    }
  }

  void CheckExtendedAttributeValue(const string attr_name,
                                   const string expected_value) const {
    ssize_t len =
        getxattr(test_file().value().c_str(), attr_name.c_str(), NULL, 0);
    if (len <= static_cast<ssize_t>(0)) {
      FAIL() << "Attribute '" << attr_name << "' does not exist";
    }
    char* buffer = new char[len];
    len = getxattr(test_file().value().c_str(), attr_name.c_str(), buffer, len);
    EXPECT_EQ(expected_value.size(), static_cast<size_t>(len));
    string real_value(buffer, len);
    delete[] buffer;
    EXPECT_EQ(expected_value, real_value);
  }

  void GetExtendedAttributeNames(vector<string>* attr_names) const {
    ssize_t len = listxattr(test_file().value().c_str(), NULL, 0);
    if (len <= static_cast<ssize_t>(0))
      return;
    char* buffer = new char[len];
    len = listxattr(test_file().value().c_str(), buffer, len);
    *attr_names =
        base::SplitString(string(buffer, len), std::string(1, '\0'),
                          base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
    delete[] buffer;
  }

  void VerifyAttributesAreSetCorrectly() const {
    vector<string> attr_names;
    GetExtendedAttributeNames(&attr_names);

    // Check if the attributes are set on the file
    vector<string>::const_iterator pos =
        find(attr_names.begin(), attr_names.end(), kSourceURLExtendedAttrName);
    EXPECT_NE(pos, attr_names.end());
    pos = find(attr_names.begin(), attr_names.end(),
               kReferrerURLExtendedAttrName);
    EXPECT_NE(pos, attr_names.end());

    // Check if the attribute values are set correctly
    CheckExtendedAttributeValue(kSourceURLExtendedAttrName,
                                source_url().spec());
    CheckExtendedAttributeValue(kReferrerURLExtendedAttrName,
                                referrer_url().spec());
  }

 private:
  base::ScopedTempDir temp_dir_;
  base::FilePath test_file_;
  GURL source_url_;
  GURL referrer_url_;
  bool is_xattr_supported_;
};

TEST_F(QuarantineLinuxTest, CheckMetadataSetCorrectly) {
  if (!is_xattr_supported())
    return;
  EXPECT_EQ(
      QuarantineFileResult::OK,
      QuarantineFile(test_file(), source_url(), referrer_url(), std::string()));
  VerifyAttributesAreSetCorrectly();
}

TEST_F(QuarantineLinuxTest, SetMetadataMultipleTimes) {
  if (!is_xattr_supported())
    return;
  GURL dummy_url("http://www.dummy.com");
  EXPECT_EQ(QuarantineFileResult::OK,
            QuarantineFile(test_file(), dummy_url, dummy_url, std::string()));
  EXPECT_EQ(
      QuarantineFileResult::OK,
      QuarantineFile(test_file(), source_url(), referrer_url(), std::string()));
  VerifyAttributesAreSetCorrectly();
}

TEST_F(QuarantineLinuxTest, InvalidSourceURLTest) {
  if (!is_xattr_supported())
    return;
  GURL invalid_url;
  vector<string> attr_names;
  EXPECT_EQ(
      QuarantineFileResult::ANNOTATION_FAILED,
      QuarantineFile(test_file(), invalid_url, referrer_url(), std::string()));
  GetExtendedAttributeNames(&attr_names);
  EXPECT_EQ(attr_names.end(), find(attr_names.begin(), attr_names.end(),
                                   kSourceURLExtendedAttrName));
  CheckExtendedAttributeValue(kReferrerURLExtendedAttrName,
                              referrer_url().spec());
}

TEST_F(QuarantineLinuxTest, InvalidReferrerURLTest) {
  if (!is_xattr_supported())
    return;
  GURL invalid_url;
  vector<string> attr_names;
  EXPECT_EQ(
      QuarantineFileResult::OK,
      QuarantineFile(test_file(), source_url(), invalid_url, std::string()));
  GetExtendedAttributeNames(&attr_names);
  EXPECT_EQ(attr_names.end(), find(attr_names.begin(), attr_names.end(),
                                   kReferrerURLExtendedAttrName));
  CheckExtendedAttributeValue(kSourceURLExtendedAttrName, source_url().spec());
}

TEST_F(QuarantineLinuxTest, InvalidURLsTest) {
  if (!is_xattr_supported())
    return;
  GURL invalid_url;
  vector<string> attr_names;
  EXPECT_EQ(
      QuarantineFileResult::ANNOTATION_FAILED,
      QuarantineFile(test_file(), invalid_url, invalid_url, std::string()));
  GetExtendedAttributeNames(&attr_names);
  EXPECT_EQ(attr_names.end(), find(attr_names.begin(), attr_names.end(),
                                   kSourceURLExtendedAttrName));
  EXPECT_EQ(attr_names.end(), find(attr_names.begin(), attr_names.end(),
                                   kReferrerURLExtendedAttrName));
}

}  // namespace
}  // namespace content
