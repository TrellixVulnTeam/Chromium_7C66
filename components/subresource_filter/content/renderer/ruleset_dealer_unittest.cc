// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/subresource_filter/content/renderer/ruleset_dealer.h"

#include <vector>

#include "base/files/file.h"
#include "components/subresource_filter/content/renderer/ruleset_dealer.h"
#include "components/subresource_filter/core/common/memory_mapped_ruleset.h"
#include "components/subresource_filter/core/common/test_ruleset_creator.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace subresource_filter {

namespace {

const char kTestRulesetData1[] = "foo";
const char kTestRulesetData2[] = "bar";

std::vector<uint8_t> ReadRulesetContents(const MemoryMappedRuleset* ruleset) {
  return std::vector<uint8_t>(ruleset->data(),
                              ruleset->data() + ruleset->length());
}

}  // namespace

class SubresourceFilterRulesetDealerTest : public ::testing::Test {
 public:
  SubresourceFilterRulesetDealerTest() {}

 protected:
  void SetUp() override {
    ResetRulesetDealer();
    ASSERT_NO_FATAL_FAILURE(
        test_ruleset_creator_.CreateRulesetToDisallowURLsWithPathSuffix(
            kTestRulesetData1, &test_ruleset_pair_1_));
    ASSERT_NO_FATAL_FAILURE(
        test_ruleset_creator_.CreateRulesetToDisallowURLsWithPathSuffix(
            kTestRulesetData2, &test_ruleset_pair_2_));
  }

  testing::TestRuleset& test_indexed_ruleset_1() {
    return test_ruleset_pair_1_.indexed;
  }

  testing::TestRuleset& test_indexed_ruleset_2() {
    return test_ruleset_pair_2_.indexed;
  }

  RulesetDealer* ruleset_dealer() { return ruleset_dealer_.get(); }

  void ResetRulesetDealer() { ruleset_dealer_.reset(new RulesetDealer); }

  bool has_cached_ruleset() const {
    return ruleset_dealer_->has_cached_ruleset();
  }

 private:
  testing::TestRulesetCreator test_ruleset_creator_;
  testing::TestRulesetPair test_ruleset_pair_1_;
  testing::TestRulesetPair test_ruleset_pair_2_;
  std::unique_ptr<RulesetDealer> ruleset_dealer_;

  DISALLOW_COPY_AND_ASSIGN(SubresourceFilterRulesetDealerTest);
};

TEST_F(SubresourceFilterRulesetDealerTest, NoRuleset) {
  EXPECT_FALSE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_FALSE(!!ruleset_dealer()->GetRuleset());
}

TEST_F(SubresourceFilterRulesetDealerTest, MostRecentlySetRulesetIsReturned) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));
  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset_1 =
      ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  ASSERT_TRUE(!!ref_to_ruleset_1);
  EXPECT_EQ(test_indexed_ruleset_1().contents,
            ReadRulesetContents(ref_to_ruleset_1.get()));

  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_2()));
  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset_2 =
      ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  ASSERT_TRUE(!!ref_to_ruleset_2);
  EXPECT_EQ(test_indexed_ruleset_1().contents,
            ReadRulesetContents(ref_to_ruleset_1.get()));
  EXPECT_EQ(test_indexed_ruleset_2().contents,
            ReadRulesetContents(ref_to_ruleset_2.get()));
}

TEST_F(SubresourceFilterRulesetDealerTest,
       MemoryMappedRulesetIsCachedAndReused) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset =
      ruleset_dealer()->GetRuleset();
  scoped_refptr<const MemoryMappedRuleset> another_ref_to_ruleset =
      ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_TRUE(!!ref_to_ruleset);
  EXPECT_TRUE(!!another_ref_to_ruleset);
  EXPECT_EQ(ref_to_ruleset.get(), another_ref_to_ruleset.get());
}

TEST_F(SubresourceFilterRulesetDealerTest, RulesetIsMemoryMappedLazily) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_FALSE(has_cached_ruleset());

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset =
      ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_TRUE(!!ref_to_ruleset);
  EXPECT_TRUE(has_cached_ruleset());
}

TEST_F(SubresourceFilterRulesetDealerTest, RulesetIsUnmappedEagerly) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset =
      ruleset_dealer()->GetRuleset();
  scoped_refptr<const MemoryMappedRuleset> another_ref_to_ruleset =
      ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(!!ref_to_ruleset);
  EXPECT_TRUE(!!another_ref_to_ruleset);
  EXPECT_TRUE(has_cached_ruleset());
  EXPECT_EQ(ref_to_ruleset.get(), another_ref_to_ruleset.get());

  another_ref_to_ruleset = nullptr;

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_TRUE(has_cached_ruleset());
  EXPECT_EQ(test_indexed_ruleset_1().contents,
            ReadRulesetContents(ref_to_ruleset.get()));

  ref_to_ruleset = nullptr;

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_FALSE(has_cached_ruleset());
}

TEST_F(SubresourceFilterRulesetDealerTest, RulesetIsUnmappedAndRemapped) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset =
      ruleset_dealer()->GetRuleset();

  ASSERT_TRUE(!!ref_to_ruleset);
  ASSERT_TRUE(has_cached_ruleset());

  ref_to_ruleset = nullptr;

  EXPECT_FALSE(has_cached_ruleset());

  ref_to_ruleset = ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_TRUE(has_cached_ruleset());
  ASSERT_TRUE(!!ref_to_ruleset);
  EXPECT_EQ(test_indexed_ruleset_1().contents,
            ReadRulesetContents(ref_to_ruleset.get()));

  ref_to_ruleset = nullptr;

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_FALSE(has_cached_ruleset());
}

TEST_F(SubresourceFilterRulesetDealerTest, NewRulesetIsMappedLazilyOnUpdate) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset =
      ruleset_dealer()->GetRuleset();

  ASSERT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  ASSERT_TRUE(!!ref_to_ruleset);
  ASSERT_TRUE(has_cached_ruleset());

  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_2()));

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_FALSE(has_cached_ruleset());

  // Check that nothing explodes if the last reference to the old ruleset is
  // dropped after it is no longer cached by the RulesetDealer.
  ref_to_ruleset = nullptr;

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_FALSE(has_cached_ruleset());
}

// If the last reference to the old version of the ruleset is dropped after the
// the version is already cached, that should have no effect on caching.
TEST_F(SubresourceFilterRulesetDealerTest,
       NewRulesetRemainsCachedAfterOldRulesetIsUnmapped) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset_1 =
      ruleset_dealer()->GetRuleset();

  ASSERT_TRUE(!!ref_to_ruleset_1);
  ASSERT_TRUE(has_cached_ruleset());

  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_2()));

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset_2 =
      ruleset_dealer()->GetRuleset();
  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());

  ref_to_ruleset_1 = nullptr;

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  scoped_refptr<const MemoryMappedRuleset> another_ref_to_ruleset_2 =
      ruleset_dealer()->GetRuleset();

  EXPECT_TRUE(ruleset_dealer()->IsRulesetAvailable());
  EXPECT_TRUE(has_cached_ruleset());
  ASSERT_TRUE(!!ref_to_ruleset_2);
  EXPECT_TRUE(!!another_ref_to_ruleset_2);
  EXPECT_EQ(ref_to_ruleset_2.get(), another_ref_to_ruleset_2.get());
  EXPECT_EQ(test_indexed_ruleset_2().contents,
            ReadRulesetContents(ref_to_ruleset_2.get()));
}

TEST_F(SubresourceFilterRulesetDealerTest,
       RulesetDealerDestroyedBeforeRuleset) {
  ruleset_dealer()->SetRulesetFile(
      testing::TestRuleset::Open(test_indexed_ruleset_1()));

  scoped_refptr<const MemoryMappedRuleset> ref_to_ruleset =
      ruleset_dealer()->GetRuleset();

  ASSERT_TRUE(!!ref_to_ruleset);
  ASSERT_TRUE(has_cached_ruleset());

  ResetRulesetDealer();

  EXPECT_EQ(test_indexed_ruleset_1().contents,
            ReadRulesetContents(ref_to_ruleset.get()));

  ref_to_ruleset = nullptr;
}

}  // namespace subresource_filter
