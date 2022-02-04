/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringRefUtils.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(SmallXStringTest, SmokeTest) {
  SmallU16String<16> buf("hello");
  ASSERT_EQ(createUTF16Ref(u"hello"), buf.arrayRef());

  buf.append("1");
  ASSERT_EQ(createUTF16Ref(u"hello1"), buf.arrayRef());

  buf.append(u"23");
  ASSERT_EQ(createUTF16Ref(u"hello123"), buf.arrayRef());
  ASSERT_EQ(8u, buf.size());

  buf.append('4').append(u'5');
  ASSERT_EQ(createUTF16Ref(u"hello12345"), buf.arrayRef());
  ASSERT_EQ(10u, buf.size());
}
} // namespace
