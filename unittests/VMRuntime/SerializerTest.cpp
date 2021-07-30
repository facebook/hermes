/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_SERIALIZE
#include "hermes/VM/Serializer.h"
#include "hermes/VM/Deserializer.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using SerializerTest = LargeHeapRuntimeTestFixture;

class Node {
 public:
  using TestFunction = uint32_t(uint32_t);

  uint32_t value_;

  Node *next_; // Field used to test NativePointer relocation.

  PinnedHermesValue hvNext_; // Field used to test HermesValue Relocation.

  TestFunction *function_;

  explicit Node(
      uint32_t value = 0,
      Node *next = nullptr,
      TestFunction *function = nullptr)
      : value_(value), next_(next), function_(function) {}

  void serialize(Serializer &s) {
    s.writeInt<uint32_t>(value_);
    s.writeRelocation(next_);
    s.writeHermesValue(hvNext_);
    s.writeRelocation((void *)function_);
    s.endObject(this);
  }

  static Node *deserialize(Deserializer &d) {
    Node *obj = new Node();
    obj->value_ = d.readInt<uint32_t>();
    d.readRelocation(&obj->next_, RelocationKind::NativePointer);
    d.readHermesValue(&obj->hvNext_);
    d.readRelocation(&obj->function_, RelocationKind::NativePointer);
    d.endObject(obj);
    return obj;
  }
};

uint32_t testFunction1(uint32_t i) {
  return i;
}

uint32_t testFunction2(uint32_t i) {
  return i * 2;
}

uint32_t testFunction3(uint32_t i) {
  return i * 3;
}

/// Gather function pointers of native functions and put them in \p vec.
static std::vector<void *> testExternalPtrs() {
  std::vector<void *> res;
  res.push_back((void *)testFunction1);
  res.push_back((void *)testFunction2);
  res.push_back((void *)testFunction3);
  return res;
}

TEST_F(SerializerTest, SerializeDeserializeTest) {
  Node n0(0);
  Node n1(2);
  Node n2(1);
  Node n3(3);
  n0.next_ = nullptr;
  n1.next_ = &n2;
  n2.next_ = &n3;
  n3.next_ = &n1;

  n0.hvNext_ = HermesValue::encodeObjectValue(nullptr);
  n1.hvNext_ = HermesValue::encodeObjectValue(&n2);
  n2.hvNext_ = HermesValue::encodeObjectValue(&n3);
  n3.hvNext_ = HermesValue::encodeObjectValue(&n1);

  // Also test external pointers mapping here.
  n1.function_ = testFunction1;
  n2.function_ = testFunction2;
  n3.function_ = testFunction3;

  // Now serialize.
  std::string str;
  llvh::raw_string_ostream os(str);
  Serializer s(os, runtime, testExternalPtrs);
  n0.serialize(s);
  n1.serialize(s);
  n2.serialize(s);
  n3.serialize(s);
  s.writeEpilogue();

  Deserializer d(
      llvh::MemoryBuffer::getMemBuffer(os.str()), runtime, testExternalPtrs);

  Node *n4 = Node::deserialize(d);
  Node *n5 = Node::deserialize(d);
  Node *n6 = Node::deserialize(d);
  Node *n7 = Node::deserialize(d);
  d.flushRelocationQueue();

  ASSERT_EQ(0u, n4->value_);
  ASSERT_EQ(2u, n5->value_);
  ASSERT_EQ(1u, n6->value_);
  ASSERT_EQ(3u, n7->value_);

  ASSERT_EQ(nullptr, n4->next_);
  ASSERT_EQ(n6, n5->next_);
  ASSERT_EQ(n7, n6->next_);
  ASSERT_EQ(n5, n7->next_);

  ASSERT_EQ(nullptr, n4->hvNext_.getObject());
  ASSERT_EQ(n6, n5->hvNext_.getObject());
  ASSERT_EQ(n7, n6->hvNext_.getObject());
  ASSERT_EQ(n5, n7->hvNext_.getObject());

  ASSERT_EQ(n5->function_(n5->value_), testFunction1(n1.value_));
  ASSERT_EQ(n6->function_(n6->value_), testFunction2(n2.value_));
  ASSERT_EQ(n7->function_(n7->value_), testFunction3(n3.value_));
}
} // namespace
#endif
