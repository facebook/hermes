/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <fbjni/detail/utf8.h>

#include <vector>

using namespace std;
using namespace facebook::jni;

void testPair(const vector<uint8_t>& utf8, const vector<uint8_t>& modified, bool test_out_eq) {
  // utf8 -> modified utf8

  string utf8str(reinterpret_cast<const char*>(utf8.data()), utf8.size());

  size_t modlen = detail::modifiedLength(utf8str);
  EXPECT_EQ(modlen, modified.size());
  vector<uint8_t> out(modlen + 1);
  detail::utf8ToModifiedUTF8(utf8.data(), utf8.size(), out.data(), out.size());
  // we expect utf8ToModified to return null terminated string, but vector in modified
  // will not have \0 at the end, therefore we crop out buffer before comparing
  EXPECT_EQ('\0', out[modlen]);
  out.resize(modlen);
  if (test_out_eq) {
    EXPECT_EQ(out, modified);
  }

  // modified utf8 -> utf8

  string s = detail::modifiedUTF8ToUTF8(modified.data(), modified.size());
  if (test_out_eq) {
    EXPECT_EQ(s, utf8str);
  }
}

void testFailForTooShortBuffer(const vector<uint8_t>& utf8, int out_len) {
  const char* message =
#ifdef __ANDROID__
    "output buffer is too short";
#else
    "";
#endif

  ASSERT_DEATH({
      vector<uint8_t> out(out_len);
      detail::utf8ToModifiedUTF8(utf8.data(), utf8.size(), out.data(), out.size());
    }, message);
}

void vector_append(vector<uint8_t>& target, const vector<uint8_t>& source) {
  target.insert(target.end(), source.begin(), source.end());
}

TEST(ModifiedUTF8Test, pairs) {
  vector<uint8_t> zero = { 0 }; // U+0000
  vector<uint8_t> zero_modified = { 0xc0, 0x80 };
  vector<uint8_t> one_byte = { 'a' };  // U+0041
  vector<uint8_t> two_byte = { 0xd8, 0xa1 };  // U+00E1  small a with acute
  vector<uint8_t> three_byte = { 0xe4, 0xba, 0xba };  // U+4EBA  unihan ren
  vector<uint8_t> four_byte =
    { 0xf0, 0x9f, 0x98, 0xb8 };  // U+1F638  grinning cat face with smiling eyes
  vector<uint8_t> four_byte_modified = { 0xed, 0xa0, 0xbd, 0xed, 0xb8, 0xb8 };
  vector<uint8_t> four_byte_truncated(four_byte.begin(),
                                      four_byte.begin() + (four_byte.size() - 1));
  vector<uint8_t> modified_truncated(
    four_byte_modified.begin(),
    four_byte_modified.begin() + (four_byte_modified.size() - 1));

  testPair(zero, zero_modified, true);
  testPair(one_byte, one_byte, true);
  testPair(two_byte, two_byte, true);
  testPair(three_byte, three_byte, true);
  testPair(four_byte, four_byte_modified, true);

  // The output is not strictly defined, but should not crash.
  testPair(four_byte_truncated, four_byte_truncated, false);
  testPair(modified_truncated, modified_truncated, false);

  vector<uint8_t> all;
  vector_append(all, zero);
  vector_append(all, one_byte);
  vector_append(all, two_byte);
  vector_append(all, three_byte);
  vector_append(all, four_byte);
  vector_append(all, zero);
  vector_append(all, one_byte);
  vector_append(all, two_byte);
  vector_append(all, three_byte);
  vector_append(all, four_byte);

  vector<uint8_t> all_modified;
  vector_append(all_modified, zero_modified);
  vector_append(all_modified, one_byte);
  vector_append(all_modified, two_byte);
  vector_append(all_modified, three_byte);
  vector_append(all_modified, four_byte_modified);
  vector_append(all_modified, zero_modified);
  vector_append(all_modified, one_byte);
  vector_append(all_modified, two_byte);
  vector_append(all_modified, three_byte);
  vector_append(all_modified, four_byte_modified);

  testPair(all, all_modified, true);
}

TEST(ModifiedUTF8Test, conversionFailForTooShortBuffer) {
  vector<uint8_t> zero = { 0 };
  vector<uint8_t> one_byte = { 'a' };
  vector<uint8_t> four_byte = { 0xf0, 0x9f, 0x98, 0xb8 };

  testFailForTooShortBuffer(zero, 1);
  testFailForTooShortBuffer(zero, 2);
  testFailForTooShortBuffer(one_byte, 1);
  testFailForTooShortBuffer(four_byte, 2);
  testFailForTooShortBuffer(four_byte, 4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
