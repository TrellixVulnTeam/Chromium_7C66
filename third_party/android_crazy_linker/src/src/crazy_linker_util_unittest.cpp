// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crazy_linker_util.h"

#include <minitest/minitest.h>

namespace crazy {

TEST(GetBaseNamePtr, Test) {
  const char kString[] = "/tmp/foo";
  EXPECT_EQ(kString + 5, GetBaseNamePtr(kString));
}

TEST(String, Empty) {
  String s;
  EXPECT_TRUE(s.IsEmpty());
  EXPECT_EQ(0u, s.size());
  EXPECT_EQ('\0', s.c_str()[0]);
}

TEST(String, CStringConstructor) {
  String s("Simple");
  EXPECT_STREQ("Simple", s.c_str());
  EXPECT_EQ(6U, s.size());
}

TEST(String, CStringConstructorWithLength) {
  String s2("Simple", 3);
  EXPECT_STREQ("Sim", s2.c_str());
  EXPECT_EQ(3U, s2.size());
}

TEST(String, CopyConstructor) {
  String s1("Simple");
  String s2(s1);

  EXPECT_EQ(6U, s2.size());
  EXPECT_STREQ(s2.c_str(), s1.c_str());
}

TEST(String, CharConstructor) {
  String s('H');
  EXPECT_EQ(1U, s.size());
  EXPECT_STREQ("H", s.c_str());
}

TEST(String, AppendCString) {
  String s("Foo");
  s += "Bar";
  EXPECT_STREQ("FooBar", s.c_str());
  s += "Zoo";
  EXPECT_STREQ("FooBarZoo", s.c_str());
}

TEST(String, AppendOther) {
  String s("Foo");
  String s2("Bar");
  s += s2;
  EXPECT_STREQ("FooBar", s.c_str());
}

TEST(String, ArrayAccess) {
  String s("FooBar");
  EXPECT_EQ('F', s[0]);
  EXPECT_EQ('o', s[1]);
  EXPECT_EQ('o', s[2]);
  EXPECT_EQ('B', s[3]);
  EXPECT_EQ('a', s[4]);
  EXPECT_EQ('r', s[5]);
  EXPECT_EQ('\0', s[6]);
}

TEST(String, Resize) {
  String s("A very long string to have fun");
  s.Resize(10);
  EXPECT_EQ(10U, s.size());
  EXPECT_STREQ("A very lon", s.c_str());
}

TEST(String, ResizeToZero) {
  String s("Long string to force a dynamic allocation");
  s.Resize(0);
  EXPECT_EQ(0U, s.size());
  EXPECT_STREQ("", s.c_str());
}

TEST(Vector, IsEmpty) {
  Vector<void*> v;
  EXPECT_TRUE(v.IsEmpty());
}

TEST(Vector, PushBack) {
  Vector<int> v;
  v.PushBack(12345);
  EXPECT_FALSE(v.IsEmpty());
  EXPECT_EQ(1U, v.GetCount());
  EXPECT_EQ(12345, v[0]);
}

TEST(Vector, PushBack2) {
  const int kMaxCount = 500;
  Vector<int> v;
  for (int n = 0; n < kMaxCount; ++n)
    v.PushBack(n * 100);

  EXPECT_FALSE(v.IsEmpty());
  EXPECT_EQ(static_cast<size_t>(kMaxCount), v.GetCount());
}

TEST(Vector, At) {
  const int kMaxCount = 500;
  Vector<int> v;
  for (int n = 0; n < kMaxCount; ++n)
    v.PushBack(n * 100);

  for (int n = 0; n < kMaxCount; ++n) {
    TEST_TEXT << "Checking v[" << n << "]";
    EXPECT_EQ(n * 100, v[n]);
  }
}

TEST(Vector, IndexOf) {
  const int kMaxCount = 500;
  Vector<int> v;
  for (int n = 0; n < kMaxCount; ++n)
    v.PushBack(n * 100);

  for (int n = 0; n < kMaxCount; ++n) {
    TEST_TEXT << "Checking v.IndexOf(" << n * 100 << ")";
    EXPECT_EQ(n, v.IndexOf(n * 100));
  }
}

TEST(Vector, InsertAt) {
  const int kMaxCount = 500;

  for (size_t k = 0; k < kMaxCount; ++k) {
    Vector<int> v;
    for (int n = 0; n < kMaxCount; ++n)
      v.PushBack(n * 100);

    v.InsertAt(k, -1000);

    EXPECT_EQ(kMaxCount + 1, v.GetCount());
    for (int n = 0; n < v.GetCount(); ++n) {
      TEST_TEXT << "Checking v[" << n << "]";
      int expected;
      if (n < k)
        expected = n * 100;
      else if (n == k)
        expected = -1000;
      else
        expected = (n - 1) * 100;
      EXPECT_EQ(expected, v[n]);
    }
  }
}

TEST(Vector, RemoveAt) {
  const int kMaxCount = 500;

  for (size_t k = 0; k < kMaxCount; ++k) {
    Vector<int> v;
    for (int n = 0; n < kMaxCount; ++n)
      v.PushBack(n * 100);

    v.RemoveAt(k);

    EXPECT_EQ(kMaxCount - 1, v.GetCount());
    for (int n = 0; n < kMaxCount - 1; ++n) {
      TEST_TEXT << "Checking v[" << n << "]";
      int expected = (n < k) ? (n * 100) : ((n + 1) * 100);
      EXPECT_EQ(expected, v[n]);
    }
  }
}

TEST(Vector, PopFirst) {
  const int kMaxCount = 500;
  Vector<int> v;
  for (int n = 0; n < kMaxCount; ++n)
    v.PushBack(n * 100);

  for (int n = 0; n < kMaxCount; ++n) {
    int first = v.PopFirst();
    TEST_TEXT << "Checking " << n << "-th PopFirst()";
    EXPECT_EQ(n * 100, first);
    EXPECT_EQ(kMaxCount - 1 - n, v.GetCount());
  }
  EXPECT_EQ(0u, v.GetCount());
  EXPECT_TRUE(v.IsEmpty());
}

TEST(Set, Empty) {
  Set<int> s;
  EXPECT_TRUE(s.IsEmpty());
  EXPECT_EQ(0U, s.GetCount());
}

TEST(Set, OneItem) {
  Set<int> s;

  EXPECT_FALSE(s.Has(0));

  EXPECT_TRUE(s.Add(0));
  EXPECT_TRUE(s.Has(0));
  EXPECT_FALSE(s.IsEmpty());
  EXPECT_EQ(1U, s.GetCount());
}

TEST(Set, ThreeItems) {
  Set<int> s;

  EXPECT_TRUE(s.Add(0));
  EXPECT_TRUE(s.Add(1));
  EXPECT_TRUE(s.Add(2));

  EXPECT_FALSE(s.Has(-1));
  EXPECT_TRUE(s.Has(0));
  EXPECT_TRUE(s.Has(1));
  EXPECT_TRUE(s.Has(2));
  EXPECT_FALSE(s.Has(3));

  EXPECT_EQ(3U, s.GetCount());
}

}  // namespace crazy
