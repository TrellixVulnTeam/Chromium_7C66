// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_LIB_BINDINGS_INTERNAL_H_
#define MOJO_PUBLIC_CPP_BINDINGS_LIB_BINDINGS_INTERNAL_H_

#include <stdint.h>

#include <functional>

#include "base/template_util.h"
#include "mojo/public/cpp/bindings/interface_id.h"
#include "mojo/public/cpp/bindings/lib/template_util.h"
#include "mojo/public/cpp/system/core.h"

namespace mojo {

template <typename T>
class ArrayDataView;

template <typename T>
class AssociatedInterfacePtrInfoDataView;

template <typename T>
class AssociatedInterfaceRequestDataView;

template <typename T>
class InterfacePtrDataView;

template <typename T>
class InterfaceRequestDataView;

template <typename K, typename V>
class MapDataView;

class NativeStructDataView;

class StringDataView;

namespace internal {

// Please note that this is a different value than |mojo::kInvalidHandleValue|,
// which is the "decoded" invalid handle.
const uint32_t kEncodedInvalidHandleValue = static_cast<uint32_t>(-1);

// A serialized union always takes 16 bytes:
//   4-byte size + 4-byte tag + 8-byte payload.
const uint32_t kUnionDataSize = 16;

template <typename T>
class Array_Data;

template <typename K, typename V>
class Map_Data;

class NativeStruct_Data;

using String_Data = Array_Data<char>;

inline size_t Align(size_t size) {
  return (size + 7) & ~0x7;
}

inline bool IsAligned(const void* ptr) {
  return !(reinterpret_cast<uintptr_t>(ptr) & 0x7);
}

// Pointers are encoded as relative offsets. The offsets are relative to the
// address of where the offset value is stored, such that the pointer may be
// recovered with the expression:
//
//   ptr = reinterpret_cast<char*>(offset) + *offset
//
// A null pointer is encoded as an offset value of 0.
//
inline void EncodePointer(const void* ptr, uint64_t* offset) {
  if (!ptr) {
    *offset = 0;
    return;
  }

  const char* p_obj = reinterpret_cast<const char*>(ptr);
  const char* p_slot = reinterpret_cast<const char*>(offset);
  DCHECK(p_obj > p_slot);

  *offset = static_cast<uint64_t>(p_obj - p_slot);
}

// Note: This function doesn't validate the encoded pointer value.
inline const void* DecodePointer(const uint64_t* offset) {
  if (!*offset)
    return nullptr;
  return reinterpret_cast<const char*>(offset) + *offset;
}

#pragma pack(push, 1)

struct StructHeader {
  uint32_t num_bytes;
  uint32_t version;
};
static_assert(sizeof(StructHeader) == 8, "Bad sizeof(StructHeader)");

struct ArrayHeader {
  uint32_t num_bytes;
  uint32_t num_elements;
};
static_assert(sizeof(ArrayHeader) == 8, "Bad_sizeof(ArrayHeader)");

template <typename T>
struct Pointer {
  using BaseType = T;

  void Set(T* ptr) { EncodePointer(ptr, &offset); }
  const T* Get() const { return static_cast<const T*>(DecodePointer(&offset)); }
  T* Get() {
    return static_cast<T*>(const_cast<void*>(DecodePointer(&offset)));
  }

  bool is_null() const { return offset == 0; }

  uint64_t offset;
};
static_assert(sizeof(Pointer<char>) == 8, "Bad_sizeof(Pointer)");

struct Handle_Data {
  Handle_Data() = default;
  explicit Handle_Data(uint32_t value) : value(value) {}

  bool is_valid() const { return value != kEncodedInvalidHandleValue; }

  uint32_t value;
};
static_assert(sizeof(Handle_Data) == 4, "Bad_sizeof(Handle_Data)");

struct Interface_Data {
  Handle_Data handle;
  uint32_t version;
};
static_assert(sizeof(Interface_Data) == 8, "Bad_sizeof(Interface_Data)");

struct AssociatedInterface_Data {
  InterfaceId interface_id;
  uint32_t version;
};
static_assert(sizeof(AssociatedInterface_Data) == 8,
              "Bad_sizeof(AssociatedInterface_Data)");

struct AssociatedInterfaceRequest_Data {
  InterfaceId interface_id;
};
static_assert(sizeof(AssociatedInterfaceRequest_Data) == 4,
              "Bad_sizeof(AssociatedInterfaceRequest_Data)");

#pragma pack(pop)

template <typename T>
T FetchAndReset(T* ptr) {
  T temp = *ptr;
  *ptr = T();
  return temp;
}

template <typename T>
struct IsUnionDataType {
 private:
  template <typename U>
  static YesType Test(const typename U::MojomUnionDataType*);

  template <typename U>
  static NoType Test(...);

  EnsureTypeIsComplete<T> check_t_;

 public:
  static const bool value =
      sizeof(Test<T>(0)) == sizeof(YesType) && !IsConst<T>::value;
};

enum class MojomTypeCategory : uint32_t {
  ARRAY = 1 << 0,
  ASSOCIATED_INTERFACE = 1 << 1,
  ASSOCIATED_INTERFACE_REQUEST = 1 << 2,
  BOOLEAN = 1 << 3,
  ENUM = 1 << 4,
  HANDLE = 1 << 5,
  INTERFACE = 1 << 6,
  INTERFACE_REQUEST = 1 << 7,
  MAP = 1 << 8,
  // POD except boolean and enum.
  POD = 1 << 9,
  STRING = 1 << 10,
  STRUCT = 1 << 11,
  UNION = 1 << 12
};

inline constexpr MojomTypeCategory operator&(MojomTypeCategory x,
                                             MojomTypeCategory y) {
  return static_cast<MojomTypeCategory>(static_cast<uint32_t>(x) &
                                        static_cast<uint32_t>(y));
}

inline constexpr MojomTypeCategory operator|(MojomTypeCategory x,
                                             MojomTypeCategory y) {
  return static_cast<MojomTypeCategory>(static_cast<uint32_t>(x) |
                                        static_cast<uint32_t>(y));
}

template <typename T, bool is_enum = std::is_enum<T>::value>
struct MojomTypeTraits {
  using Data = T;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category = MojomTypeCategory::POD;
};

template <typename T>
struct MojomTypeTraits<ArrayDataView<T>, false> {
  using Data = Array_Data<typename MojomTypeTraits<T>::DataAsArrayElement>;
  using DataAsArrayElement = Pointer<Data>;

  static const MojomTypeCategory category = MojomTypeCategory::ARRAY;
};

template <typename T>
struct MojomTypeTraits<AssociatedInterfacePtrInfoDataView<T>, false> {
  using Data = AssociatedInterface_Data;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category =
      MojomTypeCategory::ASSOCIATED_INTERFACE;
};

template <typename T>
struct MojomTypeTraits<AssociatedInterfaceRequestDataView<T>, false> {
  using Data = AssociatedInterfaceRequest_Data;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category =
      MojomTypeCategory::ASSOCIATED_INTERFACE_REQUEST;
};

template <>
struct MojomTypeTraits<bool, false> {
  using Data = bool;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category = MojomTypeCategory::BOOLEAN;
};

template <typename T>
struct MojomTypeTraits<T, true> {
  using Data = int32_t;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category = MojomTypeCategory::ENUM;
};

template <typename T>
struct MojomTypeTraits<ScopedHandleBase<T>, false> {
  using Data = Handle_Data;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category = MojomTypeCategory::HANDLE;
};

template <typename T>
struct MojomTypeTraits<InterfacePtrDataView<T>, false> {
  using Data = Interface_Data;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category = MojomTypeCategory::INTERFACE;
};

template <typename T>
struct MojomTypeTraits<InterfaceRequestDataView<T>, false> {
  using Data = Handle_Data;
  using DataAsArrayElement = Data;

  static const MojomTypeCategory category =
      MojomTypeCategory::INTERFACE_REQUEST;
};

template <typename K, typename V>
struct MojomTypeTraits<MapDataView<K, V>, false> {
  using Data = Map_Data<typename MojomTypeTraits<K>::DataAsArrayElement,
                        typename MojomTypeTraits<V>::DataAsArrayElement>;
  using DataAsArrayElement = Pointer<Data>;

  static const MojomTypeCategory category = MojomTypeCategory::MAP;
};

template <>
struct MojomTypeTraits<NativeStructDataView, false> {
  using Data = internal::NativeStruct_Data;
  using DataAsArrayElement = Pointer<Data>;

  static const MojomTypeCategory category = MojomTypeCategory::STRUCT;
};

template <>
struct MojomTypeTraits<StringDataView, false> {
  using Data = String_Data;
  using DataAsArrayElement = Pointer<Data>;

  static const MojomTypeCategory category = MojomTypeCategory::STRING;
};

template <typename T, MojomTypeCategory categories>
struct BelongsTo {
  static const bool value =
      static_cast<uint32_t>(MojomTypeTraits<T>::category & categories) != 0;
};

template <typename T>
struct EnumHashImpl {
  static_assert(std::is_enum<T>::value, "Incorrect hash function.");

  size_t operator()(T input) const {
    using UnderlyingType = typename base::underlying_type<T>::type;
    return std::hash<UnderlyingType>()(static_cast<UnderlyingType>(input));
  }
};

}  // namespace internal
}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_LIB_BINDINGS_INTERNAL_H_
