// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include <memory>
#include <vector>

#include "components/tracing/core/proto_zero_message_handle.h"
#include "components/tracing/test/example_proto/library.pbzero.h"
#include "components/tracing/test/example_proto/test_messages.pb.h"
#include "components/tracing/test/example_proto/test_messages.pbzero.h"
#include "components/tracing/test/fake_scattered_buffer.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace tracing {
namespace proto {

namespace pbtest = pbzero::foo::bar;
namespace pbgold = foo::bar;  // Official C++ protobuf compiler.

using tracing::v2::FakeScatteredBuffer;
using tracing::v2::ProtoZeroMessage;
using tracing::v2::ScatteredStreamWriter;
using tracing::v2::ProtoZeroMessageHandle;
using tracing::v2::ProtoZeroMessageHandleBase;
using tracing::v2::proto::ProtoFieldDescriptor;

constexpr size_t kChunkSize = 42;

class ProtoZeroConformanceTest : public ::testing::Test {
 public:
  void SetUp() override {
    buffer_.reset(new FakeScatteredBuffer(kChunkSize));
    stream_writer_.reset(new ScatteredStreamWriter(buffer_.get()));
  }

  void TearDown() override {
    root_messages_.clear();
    stream_writer_.reset();
    buffer_.reset();
    google::protobuf::ShutdownProtobufLibrary();
  }

 protected:
  template <class T>
  T* CreateMessage() {
    T* message = new T();
    root_messages_.push_back(std::unique_ptr<T>(message));
    message->Reset(stream_writer_.get());
    return message;
  }

  size_t GetNumSerializedBytes() {
    return buffer_->chunks().size() * kChunkSize -
           stream_writer_->bytes_available();
  }

  void GetSerializedBytes(size_t start, size_t length, uint8_t* buffer) {
    return buffer_->GetBytes(start, length, buffer);
  }

 private:
  std::unique_ptr<FakeScatteredBuffer> buffer_;
  std::unique_ptr<ScatteredStreamWriter> stream_writer_;
  std::vector<std::unique_ptr<ProtoZeroMessage>> root_messages_;
};

TEST_F(ProtoZeroConformanceTest, SimpleFieldsNoNesting) {
  auto msg = CreateMessage<pbtest::EveryField>();

  msg->set_field_int32(-1);
  msg->set_field_int64(-333123456789ll);
  msg->set_field_uint32(600);
  msg->set_field_uint64(333123456789ll);
  msg->set_field_sint32(-5);
  msg->set_field_sint64(-9000);
  msg->set_field_fixed32(12345);
  msg->set_field_fixed64(444123450000ll);
  msg->set_field_sfixed32(-69999);
  msg->set_field_sfixed64(-200);
  msg->set_field_float(3.14f);
  msg->set_field_double(0.5555);
  msg->set_field_bool(true);
  msg->set_small_enum(pbtest::SmallEnum::TO_BE);
  msg->set_signed_enum(pbtest::SignedEnum::NEGATIVE);
  msg->set_big_enum(pbtest::BigEnum::BEGIN);
  msg->set_field_string("FizzBuzz");
  msg->set_field_bytes(reinterpret_cast<const uint8_t*>("\x11\x00\xBE\xEF"), 4);
  msg->add_repeated_int32(1);
  msg->add_repeated_int32(-1);
  msg->add_repeated_int32(100);
  msg->add_repeated_int32(2000000);

  size_t msg_size = GetNumSerializedBytes();
  EXPECT_EQ(126u, msg_size);

  std::unique_ptr<uint8_t[]> msg_binary(new uint8_t[msg_size]);
  GetSerializedBytes(0, msg_size, msg_binary.get());

  pbgold::EveryField gold_msg;
  gold_msg.ParseFromArray(msg_binary.get(), msg_size);
  EXPECT_EQ(-1, gold_msg.field_int32());
  EXPECT_EQ(-333123456789ll, gold_msg.field_int64());
  EXPECT_EQ(600u, gold_msg.field_uint32());
  EXPECT_EQ(333123456789ull, gold_msg.field_uint64());
  EXPECT_EQ(-5, gold_msg.field_sint32());
  EXPECT_EQ(-9000, gold_msg.field_sint64());
  EXPECT_EQ(12345u, gold_msg.field_fixed32());
  EXPECT_EQ(444123450000ull, gold_msg.field_fixed64());
  EXPECT_EQ(-69999, gold_msg.field_sfixed32());
  EXPECT_EQ(-200, gold_msg.field_sfixed64());
  EXPECT_EQ(3.14f, gold_msg.field_float());
  EXPECT_EQ(0.5555, gold_msg.field_double());
  EXPECT_EQ(true, gold_msg.field_bool());
  EXPECT_EQ(pbgold::SmallEnum::TO_BE, gold_msg.small_enum());
  EXPECT_EQ(pbgold::SignedEnum::NEGATIVE, gold_msg.signed_enum());
  EXPECT_EQ(pbgold::BigEnum::BEGIN, gold_msg.big_enum());
  EXPECT_EQ("FizzBuzz", gold_msg.field_string());
  EXPECT_EQ(std::string("\x11\x00\xBE\xEF", 4), gold_msg.field_bytes());
  EXPECT_EQ(4, gold_msg.repeated_int32_size());
  EXPECT_EQ(1, gold_msg.repeated_int32(0));
  EXPECT_EQ(-1, gold_msg.repeated_int32(1));
  EXPECT_EQ(100, gold_msg.repeated_int32(2));
  EXPECT_EQ(2000000, gold_msg.repeated_int32(3));
}

TEST_F(ProtoZeroConformanceTest, NestedMessages) {
  auto msg_a = CreateMessage<pbtest::NestedA>();

  pbtest::NestedA::NestedB* msg_b = msg_a->add_repeated_a();
  pbtest::NestedA::NestedB::NestedC* msg_c = msg_b->set_value_b();
  msg_c->set_value_c(321);
  msg_b = msg_a->add_repeated_a();
  msg_c = msg_a->set_super_nested();
  msg_c->set_value_c(1000);
  msg_a->Finalize();

  size_t msg_size = GetNumSerializedBytes();
  EXPECT_EQ(26u, msg_size);

  std::unique_ptr<uint8_t[]> msg_binary(new uint8_t[msg_size]);
  GetSerializedBytes(0, msg_size, msg_binary.get());

  pbgold::NestedA gold_msg_a;
  gold_msg_a.ParseFromArray(msg_binary.get(), msg_size);
  EXPECT_EQ(2, gold_msg_a.repeated_a_size());
  EXPECT_EQ(321, gold_msg_a.repeated_a(0).value_b().value_c());
  EXPECT_FALSE(gold_msg_a.repeated_a(1).has_value_b());
  EXPECT_EQ(1000, gold_msg_a.super_nested().value_c());
}

TEST(ProtoZeroTest, Simple) {
  // Test the includes for indirect public import: library.pbzero.h ->
  // library_internals/galaxies.pbzero.h -> upper_import.pbzero.h .
  EXPECT_LE(0u, sizeof(pbtest::TrickyPublicImport));
}

TEST(ProtoZeroTest, Reflection) {
  // Tests camel case conversion as well.
  EXPECT_EQ(1, pbtest::CamelCaseFields::kFooBarBazFieldNumber);
  EXPECT_EQ(2, pbtest::CamelCaseFields::kBarBazFieldNumber);
  EXPECT_EQ(3, pbtest::CamelCaseFields::kMooMooFieldNumber);
  EXPECT_EQ(4, pbtest::CamelCaseFields::kURLEncoderFieldNumber);
  EXPECT_EQ(5, pbtest::CamelCaseFields::kXMapFieldNumber);
  EXPECT_EQ(6, pbtest::CamelCaseFields::kUrLENcoDerFieldNumber);
  EXPECT_EQ(7, pbtest::CamelCaseFields::kBigBangFieldNumber);
  EXPECT_EQ(8, pbtest::CamelCaseFields::kU2FieldNumber);
  EXPECT_EQ(9, pbtest::CamelCaseFields::kBangBigFieldNumber);

  const ProtoFieldDescriptor* reflection =
       pbtest::EveryField::GetFieldDescriptor(
           pbtest::EveryField::kFieldInt32FieldNumber);
  EXPECT_STREQ("field_int32", reflection->name());
  EXPECT_EQ(ProtoFieldDescriptor::Type::TYPE_INT32, reflection->type());
  EXPECT_EQ(1u, reflection->number());
  EXPECT_FALSE(reflection->is_repeated());
  EXPECT_TRUE(reflection->is_valid());

  EXPECT_FALSE(pbtest::TransgalacticParcel::GetFieldDescriptor(42)->is_valid());
}

}  // namespace proto
}  // namespace tracing
