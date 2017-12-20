// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/query_parser/snippet.h"

#include <stddef.h>

#include <algorithm>

#include "base/macros.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace query_parser {
namespace {

// A sample document to compute snippets of.
// The \x bits after the first "Google" are UTF-8 of U+2122 TRADE MARK SIGN,
// and are useful for verifying we don't screw up in UTF-8/UTF-16 conversion.
const char* kSampleDocument = "Google\xe2\x84\xa2 Terms of Service "
"Welcome to Google! "
"1. Your relationship with Google "
"1.1 Your use of Google's products, software, services and web sites "
"(referred to collectively as the \"Services\" in this document and excluding "
"any services provided to you by Google under a separate written agreement) "
"is subject to the terms of a legal agreement between you and Google. "
"\"Google\" means Google Inc., whose principal place of business is at 1600 "
"Amphitheatre Parkway, Mountain View, CA 94043, United States. This document "
"explains how the agreement is made up, and sets out some of the terms of "
"that agreement.";
};

// Thai sample taken from http://www.google.co.th/intl/th/privacy.html
// TODO(jungshik) : Add more samples (e.g. Hindi) after porting
// ICU 4.0's character iterator changes to our copy of ICU 3.8 to get
// grapheme clusters in Indic scripts handled more reasonably.
const char* kThaiSample = "Google \xE0\xB9\x80\xE0\xB8\x81\xE0\xB9\x87"
"\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xA7\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xA7"
"\xE0\xB8\xA1 \xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xA1\xE0\xB8\xB9"
"\xE0\xB8\xA5\xE0\xB8\xAA\xE0\xB9\x88\xE0\xB8\xA7\xE0\xB8\x99\xE0\xB8\x9A"
"\xE0\xB8\xB8\xE0\xB8\x84\xE0\xB8\x84\xE0\xB8\xA5 \xE0\xB9\x80\xE0\xB8\xA1"
"\xE0\xB8\xB7\xE0\xB9\x88\xE0\xB8\xAD\xE0\xB8\x84\xE0\xB8\xB8\xE0\xB8\x93"
"\xE0\xB8\xA5\xE0\xB8\x87\xE0\xB8\x97\xE0\xB8\xB0\xE0\xB9\x80\xE0\xB8\x9A"
"\xE0\xB8\xB5\xE0\xB8\xA2\xE0\xB8\x99\xE0\xB9\x80\xE0\xB8\x9E\xE0\xB8\xB7"
"\xE0\xB9\x88\xE0\xB8\xAD\xE0\xB9\x83\xE0\xB8\x8A\xE0\xB9\x89\xE0\xB8\x9A"
"\xE0\xB8\xA3\xE0\xB8\xB4\xE0\xB8\x81\xE0\xB8\xB2\xE0\xB8\xA3\xE0\xB8\x82"
"\xE0\xB8\xAD\xE0\xB8\x87 Google \xE0\xB8\xAB\xE0\xB8\xA3\xE0\xB8\xB7"
"\xE0\xB8\xAD\xE0\xB9\x83\xE0\xB8\xAB\xE0\xB9\x89\xE0\xB8\x82\xE0\xB9\x89"
"\xE0\xB8\xAD\xE0\xB8\xA1\xE0\xB8\xB9\xE0\xB8\xA5\xE0\xB8\x94\xE0\xB8\xB1"
"\xE0\xB8\x87\xE0\xB8\x81\xE0\xB8\xA5\xE0\xB9\x88\xE0\xB8\xB2\xE0\xB8\xA7"
"\xE0\xB9\x82\xE0\xB8\x94\xE0\xB8\xA2\xE0\xB8\xAA\xE0\xB8\xA1\xE0\xB8\xB1"
"\xE0\xB8\x84\xE0\xB8\xA3\xE0\xB9\x83\xE0\xB8\x88 \xE0\xB9\x80\xE0\xB8\xA3"
"\xE0\xB8\xB2\xE0\xB8\xAD\xE0\xB8\xB2\xE0\xB8\x88\xE0\xB8\xA3\xE0\xB8\xA7"
"\xE0\xB8\xA1\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xA1\xE0\xB8\xB9"
"\xE0\xB8\xA5\xE0\xB8\xAA\xE0\xB9\x88\xE0\xB8\xA7\xE0\xB8\x99\xE0\xB8\x9A"
"\xE0\xB8\xB8\xE0\xB8\x84\xE0\xB8\x84\xE0\xB8\xA5\xE0\xB8\x97\xE0\xB8\xB5"
"\xE0\xB9\x88\xE0\xB9\x80\xE0\xB8\x81\xE0\xB9\x87\xE0\xB8\x9A\xE0\xB8\xA3"
"\xE0\xB8\xA7\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xA7\xE0\xB8\xA1\xE0\xB8\x88"
"\xE0\xB8\xB2\xE0\xB8\x81\xE0\xB8\x84\xE0\xB8\xB8\xE0\xB8\x93\xE0\xB9\x80"
"\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xB2\xE0\xB8\x81\xE0\xB8\xB1\xE0\xB8\x9A"
"\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xA1\xE0\xB8\xB9\xE0\xB8\xA5"
"\xE0\xB8\x88\xE0\xB8\xB2\xE0\xB8\x81\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xB4"
"\xE0\xB8\x81\xE0\xB8\xB2\xE0\xB8\xA3\xE0\xB8\xAD\xE0\xB8\xB7\xE0\xB9\x88"
"\xE0\xB8\x99\xE0\xB8\x82\xE0\xB8\xAD\xE0\xB8\x87 Google \xE0\xB8\xAB"
"\xE0\xB8\xA3\xE0\xB8\xB7\xE0\xB8\xAD\xE0\xB8\x9A\xE0\xB8\xB8\xE0\xB8\x84"
"\xE0\xB8\x84\xE0\xB8\xA5\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\xAA"
"\xE0\xB8\xB2\xE0\xB8\xA1 \xE0\xB9\x80\xE0\xB8\x9E\xE0\xB8\xB7\xE0\xB9\x88"
"\xE0\xB8\xAD\xE0\xB9\x83\xE0\xB8\xAB\xE0\xB9\x89\xE0\xB8\x9C\xE0\xB8\xB9"
"\xE0\xB9\x89\xE0\xB9\x83\xE0\xB8\x8A\xE0\xB9\x89\xE0\xB9\x84\xE0\xB8\x94"
"\xE0\xB9\x89\xE0\xB8\xA3\xE0\xB8\xB1\xE0\xB8\x9A\xE0\xB8\x9B\xE0\xB8\xA3"
"\xE0\xB8\xB0\xE0\xB8\xAA\xE0\xB8\x9A\xE0\xB8\x81\xE0\xB8\xB2\xE0\xB8\xA3"
"\xE0\xB8\x93\xE0\xB9\x8C\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\x94"
"\xE0\xB8\xB5\xE0\xB8\x82\xE0\xB8\xB6\xE0\xB9\x89\xE0\xB8\x99 \xE0\xB8\xA3"
"\xE0\xB8\xA7\xE0\xB8\xA1\xE0\xB8\x97\xE0\xB8\xB1\xE0\xB9\x89\xE0\xB8\x87"
"\xE0\xB8\x9B\xE0\xB8\xA3\xE0\xB8\xB1\xE0\xB8\x9A\xE0\xB9\x81\xE0\xB8\x95"
"\xE0\xB9\x88\xE0\xB8\x87\xE0\xB9\x80\xE0\xB8\x99\xE0\xB8\xB7\xE0\xB9\x89"
"\xE0\xB8\xAD\xE0\xB8\xAB\xE0\xB8\xB2\xE0\xB9\x83\xE0\xB8\xAB\xE0\xB9\x89"
"\xE0\xB9\x80\xE0\xB8\xAB\xE0\xB8\xA1\xE0\xB8\xB2\xE0\xB8\xB0\xE0\xB8\xAA"
"\xE0\xB8\xB3\xE0\xB8\xAB\xE0\xB8\xA3\xE0\xB8\xB1\xE0\xB8\x9A\xE0\xB8\x84"
"\xE0\xB8\xB8\xE0\xB8\x93";

// Comparator for sorting by the first element in a pair.
bool ComparePair1st(const Snippet::MatchPosition& a,
                    const Snippet::MatchPosition& b) {
  return a.first < b.first;
}

// For testing, we'll compute the match positions manually instead of using
// sqlite's FTS matching.  BuildSnippet returns the snippet for matching
// |query| against |document|.  Matches are surrounded by "**".
base::string16 BuildSnippet(const std::string& document,
                            const std::string& query) {
  // This function assumes that |document| does not contain
  // any character for which lowercasing changes its length. Further,
  // it's assumed that lowercasing only the ASCII-portion works for
  // |document|. We need to add more test cases and change this function
  // to be more generic depending on how we deal with 'folding for match'
  // in history.
  const std::string document_folded = base::ToLowerASCII(document);

  // Manually construct match_positions of the document.
  Snippet::MatchPositions match_positions;
  match_positions.clear();
  for (const std::string& word : base::SplitString(
           query, " ", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL)) {
    // Insert all instances of this word into match_pairs.
    size_t ofs = 0;
    while ((ofs = document_folded.find(word, ofs)) != std::string::npos) {
      match_positions.push_back(std::make_pair(ofs, ofs + word.size()));
      ofs += word.size();
    }
  }
  // Sort match_positions in order of increasing offset.
  std::sort(match_positions.begin(), match_positions.end(), ComparePair1st);

  // Compute the snippet.
  Snippet snippet;
  snippet.ComputeSnippet(match_positions, document);

  // Now "highlight" all matches in the snippet with **.
  base::string16 star_snippet;
  Snippet::MatchPositions::const_iterator match;
  size_t pos = 0;
  for (match = snippet.matches().begin();
       match != snippet.matches().end(); ++match) {
    star_snippet += snippet.text().substr(pos, match->first - pos);
    star_snippet += base::UTF8ToUTF16("**");
    star_snippet += snippet.text().substr(match->first,
                                          match->second - match->first);
    star_snippet += base::UTF8ToUTF16("**");
    pos = match->second;
  }
  star_snippet += snippet.text().substr(pos);

  return star_snippet;
}

TEST(Snippets, SimpleQuery) {
  ASSERT_EQ(" ... eferred to collectively as the \"Services\" in this "
            "**document** and excluding any services provided to you by "
            "Goo ...  ... way, Mountain View, CA 94043, United States. This "
            "**document** explains how the agreement is made up, and sets "
            "o ... ",
            base::UTF16ToUTF8(BuildSnippet(kSampleDocument, "document")));
}

// Test that two words that are near each other don't produce two elided bits.
TEST(Snippets, NearbyWords) {
  ASSERT_EQ(" ... lace of business is at 1600 Amphitheatre Parkway, "
            "**Mountain** **View**, CA 94043, United States. This "
            "document explains  ... ",
            base::UTF16ToUTF8(BuildSnippet(kSampleDocument, "mountain view")));
}

// The above tests already test that we get byte offsets correct, but here's
// one that gets the "TM" in its snippet.
TEST(Snippets, UTF8) {
  ASSERT_EQ(" ... ogle\xe2\x84\xa2 Terms of Service Welcome to Google! "
            "1. Your **relationship** with Google 1.1 Your use of Google's "
            "products, so ... ",
            base::UTF16ToUTF8(BuildSnippet(kSampleDocument, "relationship")));
}

TEST(Snippets, ThaiUTF8) {
  // There are 3 instances of '\u0E43\u0E2B\u0E49'
  // (\xE0\xB9\x83\xE0\xB8\xAB\xE0\xB9\x89) in kThaiSample.
  // The 1st is more than |kSniipetContext| graphemes away from the
  // 2nd while the 2nd and 3rd are within that window. However, with
  // the 2nd match added, the snippet goes over the size limit so that
  // the snippet ends right before the 3rd match.
  ASSERT_EQ(" ...  "
            "\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xA1\xE0\xB8\xB9"
            "\xE0\xB8\xA5\xE0\xB8\xAA\xE0\xB9\x88\xE0\xB8\xA7\xE0\xB8\x99"
            "\xE0\xB8\x9A\xE0\xB8\xB8\xE0\xB8\x84\xE0\xB8\x84\xE0\xB8\xA5 "
            "\xE0\xB9\x80\xE0\xB8\xA1\xE0\xB8\xB7\xE0\xB9\x88\xE0\xB8\xAD"
            "\xE0\xB8\x84\xE0\xB8\xB8\xE0\xB8\x93\xE0\xB8\xA5\xE0\xB8\x87"
            "\xE0\xB8\x97\xE0\xB8\xB0\xE0\xB9\x80\xE0\xB8\x9A\xE0\xB8\xB5"
            "\xE0\xB8\xA2\xE0\xB8\x99\xE0\xB9\x80\xE0\xB8\x9E\xE0\xB8\xB7"
            "\xE0\xB9\x88\xE0\xB8\xAD\xE0\xB9\x83\xE0\xB8\x8A\xE0\xB9\x89"
            "\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xB4\xE0\xB8\x81\xE0\xB8\xB2"
            "\xE0\xB8\xA3\xE0\xB8\x82\xE0\xB8\xAD\xE0\xB8\x87 Google "
            "\xE0\xB8\xAB\xE0\xB8\xA3\xE0\xB8\xB7\xE0\xB8\xAD**\xE0\xB9\x83"
            "\xE0\xB8\xAB\xE0\xB9\x89**\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD"
            "\xE0\xB8\xA1\xE0\xB8\xB9\xE0\xB8\xA5\xE0\xB8\x94\xE0\xB8\xB1"
            "\xE0\xB8\x87\xE0\xB8\x81\xE0\xB8\xA5\xE0\xB9\x88\xE0\xB8\xB2"
            "\xE0\xB8\xA7\xE0\xB9\x82\xE0\xB8\x94\xE0\xB8\xA2\xE0\xB8\xAA"
            "\xE0\xB8\xA1\xE0\xB8\xB1\xE0\xB8\x84\xE0\xB8\xA3\xE0\xB9\x83"
            "\xE0\xB8\x88 \xE0\xB9\x80\xE0\xB8\xA3\xE0\xB8\xB2\xE0\xB8\xAD"
            "\xE0\xB8\xB2\xE0\xB8\x88\xE0\xB8\xA3\xE0\xB8\xA7\xE0\xB8\xA1"
            "\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xA1\xE0\xB8\xB9"
            "\xE0\xB8\xA5\xE0\xB8\xAA\xE0\xB9\x88\xE0\xB8\xA7\xE0\xB8\x99"
            "\xE0\xB8\x9A\xE0\xB8\xB8\xE0\xB8\x84\xE0\xB8\x84\xE0\xB8\xA5"
            "\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB9\x80\xE0\xB8\x81"
            "\xE0\xB9\x87\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xA7\xE0\xB8\x9A"
            "\xE0\xB8\xA3\xE0\xB8\xA7\xE0\xB8\xA1 ...  ... \xE0\xB8\x88"
            "\xE0\xB8\xB2\xE0\xB8\x81\xE0\xB8\x84\xE0\xB8\xB8\xE0\xB8\x93"
            "\xE0\xB9\x80\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xB2\xE0\xB8\x81"
            "\xE0\xB8\xB1\xE0\xB8\x9A\xE0\xB8\x82\xE0\xB9\x89\xE0\xB8\xAD"
            "\xE0\xB8\xA1\xE0\xB8\xB9\xE0\xB8\xA5\xE0\xB8\x88\xE0\xB8\xB2"
            "\xE0\xB8\x81\xE0\xB8\x9A\xE0\xB8\xA3\xE0\xB8\xB4\xE0\xB8\x81"
            "\xE0\xB8\xB2\xE0\xB8\xA3\xE0\xB8\xAD\xE0\xB8\xB7\xE0\xB9\x88"
            "\xE0\xB8\x99\xE0\xB8\x82\xE0\xB8\xAD\xE0\xB8\x87 Google "
            "\xE0\xB8\xAB\xE0\xB8\xA3\xE0\xB8\xB7\xE0\xB8\xAD\xE0\xB8\x9A"
            "\xE0\xB8\xB8\xE0\xB8\x84\xE0\xB8\x84\xE0\xB8\xA5\xE0\xB8\x97"
            "\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\xAA\xE0\xB8\xB2\xE0\xB8\xA1 "
            "\xE0\xB9\x80\xE0\xB8\x9E\xE0\xB8\xB7\xE0\xB9\x88\xE0\xB8\xAD**"
            "\xE0\xB9\x83\xE0\xB8\xAB\xE0\xB9\x89**\xE0\xB8\x9C\xE0\xB8\xB9"
            "\xE0\xB9\x89\xE0\xB9\x83\xE0\xB8\x8A\xE0\xB9\x89\xE0\xB9\x84"
            "\xE0\xB8\x94\xE0\xB9\x89\xE0\xB8\xA3\xE0\xB8\xB1\xE0\xB8\x9A"
            "\xE0\xB8\x9B\xE0\xB8\xA3\xE0\xB8\xB0\xE0\xB8\xAA\xE0\xB8\x9A"
            "\xE0\xB8\x81\xE0\xB8\xB2\xE0\xB8\xA3\xE0\xB8\x93\xE0\xB9\x8C"
            "\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\x94\xE0\xB8\xB5"
            "\xE0\xB8\x82\xE0\xB8\xB6\xE0\xB9\x89\xE0\xB8\x99 \xE0\xB8\xA3"
            "\xE0\xB8\xA7\xE0\xB8\xA1\xE0\xB8\x97\xE0\xB8\xB1\xE0\xB9\x89"
            "\xE0\xB8\x87\xE0\xB8\x9B\xE0\xB8\xA3\xE0\xB8\xB1\xE0\xB8\x9A"
            "\xE0\xB9\x81\xE0\xB8\x95\xE0\xB9\x88\xE0\xB8\x87\xE0\xB9\x80"
            "\xE0\xB8\x99\xE0\xB8\xB7\xE0\xB9\x89\xE0\xB8\xAD\xE0\xB8\xAB"
            "\xE0\xB8\xB2",
            base::UTF16ToUTF8(BuildSnippet(kThaiSample,
                                     "\xE0\xB9\x83\xE0\xB8\xAB\xE0\xB9\x89")));
}

TEST(Snippets, ExtractMatchPositions) {
  struct TestData {
    const std::string offsets_string;
    const size_t expected_match_count;
    const size_t expected_matches[10];
  } data[] = {
    { "0 0 1 2 0 0 4 1 0 0 1 5",            1,     { 1, 6 } },
    { "0 0 1 4 0 0 2 1",                    1,     { 1, 5 } },
    { "0 0 4 1 0 0 2 1",                    2,     { 2, 3, 4, 5 } },
    { "0 0 0 1",                            1,     { 0, 1 } },
    { "0 0 0 1 0 0 0 2",                    1,     { 0, 2 } },
    { "0 0 1 1 0 0 1 2",                    1,     { 1, 3 } },
    { "0 0 1 2 0 0 4 3 0 0 3 1",            1,     { 1, 7 } },
    { "0 0 1 4 0 0 2 5",                    1,     { 1, 7 } },
    { "0 0 1 2 0 0 1 1",                    1,     { 1, 3 } },
    { "0 0 1 1 0 0 5 2 0 0 10 1 0 0 3 10",  2,     { 1, 2, 3, 13 } },
  };
  for (size_t i = 0; i < arraysize(data); ++i) {
    Snippet::MatchPositions matches;
    Snippet::ExtractMatchPositions(data[i].offsets_string, "0", &matches);
    EXPECT_EQ(data[i].expected_match_count, matches.size());
    for (size_t j = 0; j < data[i].expected_match_count; ++j) {
      EXPECT_EQ(data[i].expected_matches[2 * j], matches[j].first);
      EXPECT_EQ(data[i].expected_matches[2 * j + 1], matches[j].second);
    }
  }
}

}  // namespace query_parser
