/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Serializer.h"
#include "hermes/VM/Deserializer.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using SerializerTest = LargeHeapRuntimeTestFixture;

class Node {
 public:
  uint32_t value_;

  Node *next_; // Field used to test NativePointer relocation.

  PinnedHermesValue hvNext_; // Field used to test HermesValue Relocation.

  explicit Node(uint32_t value = 0, Node *next = nullptr)
      : value_(value), next_(next) {}

  void serialize(Serializer &s) {
    s.writeInt<uint32_t>(value_);
    s.writeRelocation(next_);
    s.writeHermesValue(hvNext_, true);
    s.endObject(this);
  }

  static Node *deserialize(Deserializer &d) {
    Node *obj = new Node();
    obj->value_ = d.readInt<uint32_t>();
    d.readRelocation(&obj->next_, RelocationKind::NativePointer);
    d.readHermesValue(&obj->hvNext_, true);
    d.endObject(obj);
    return obj;
  }
};

TEST_F(SerializerTest, SerializeDeserializeTest) {
  Node n0(0);
  Node n1(2);
  Node n2(1);
  Node n3(3);
  n0.next_ = nullptr;
  n1.next_ = &n2;
  n2.next_ = &n3;
  n3.next_ = &n1;

  n0.hvNext_ = HermesValue::encodeNativePointer(nullptr);
  n1.hvNext_ = HermesValue::encodeNativePointer(&n2);
  n2.hvNext_ = HermesValue::encodeNativePointer(&n3);
  n3.hvNext_ = HermesValue::encodeNativePointer(&n1);

  // Now serialize.
  std::string str;
  llvm::raw_string_ostream os(str);
  Serializer s(os, runtime);
  n0.serialize(s);
  n1.serialize(s);
  n2.serialize(s);
  n3.serialize(s);
  s.writeEpilogue();

  Deserializer d(llvm::MemoryBuffer::getMemBuffer(os.str()), runtime);

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

  ASSERT_EQ(nullptr, n4->hvNext_.getNativePointer<Node>());
  ASSERT_EQ(n6, n5->hvNext_.getNativePointer<Node>());
  ASSERT_EQ(n7, n6->hvNext_.getNativePointer<Node>());
  ASSERT_EQ(n5, n7->hvNext_.getNativePointer<Node>());
}
} // namespace
