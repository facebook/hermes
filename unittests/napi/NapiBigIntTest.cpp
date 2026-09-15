/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include <cstdint>
#include <limits>

namespace {

using hermes::napi::NapiTestFixture;

/// Helper to open a handle scope and return the scope handle.
static napi_handle_scope openScope(napi_env env) {
  napi_handle_scope scope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_handle_scope(env, &scope));
  EXPECT_NE(nullptr, scope);
  return scope;
}

/// Helper to close a handle scope.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_create_bigint_int64
//===========================================================================

TEST_F(NapiTestFixture, CreateBigIntInt64_Zero) {
  napi_handle_scope scope = openScope(env_);

  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 0, &result));
  ASSERT_NE(nullptr, result);

  // Verify typeof is bigint.
  napi_valuetype type;
  ASSERT_EQ(napi_ok, napi_typeof(env_, result, &type));
  EXPECT_EQ(napi_bigint, type);

  // Extract and verify value.
  int64_t val = 99;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(0, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntInt64_Positive) {
  napi_handle_scope scope = openScope(env_);

  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 42, &result));

  int64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(42, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntInt64_Negative) {
  napi_handle_scope scope = openScope(env_);

  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, -123, &result));

  int64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(-123, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntInt64_MaxValue) {
  napi_handle_scope scope = openScope(env_);

  int64_t maxVal = std::numeric_limits<int64_t>::max();
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, maxVal, &result));

  int64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(maxVal, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntInt64_MinValue) {
  napi_handle_scope scope = openScope(env_);

  int64_t minVal = std::numeric_limits<int64_t>::min();
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, minVal, &result));

  int64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(minVal, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntInt64_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_create_bigint_int64(nullptr, 0, nullptr));
}

TEST_F(NapiTestFixture, CreateBigIntInt64_NullResult) {
  EXPECT_EQ(napi_invalid_arg, napi_create_bigint_int64(env_, 0, nullptr));
}

//===========================================================================
// napi_create_bigint_uint64
//===========================================================================

TEST_F(NapiTestFixture, CreateBigIntUint64_Zero) {
  napi_handle_scope scope = openScope(env_);

  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, 0, &result));

  uint64_t val = 99;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_uint64(env_, result, &val, &lossless));
  EXPECT_EQ(0u, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntUint64_Positive) {
  napi_handle_scope scope = openScope(env_);

  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, 12345, &result));

  uint64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_uint64(env_, result, &val, &lossless));
  EXPECT_EQ(12345u, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntUint64_MaxValue) {
  napi_handle_scope scope = openScope(env_);

  uint64_t maxVal = std::numeric_limits<uint64_t>::max();
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, maxVal, &result));

  uint64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_uint64(env_, result, &val, &lossless));
  EXPECT_EQ(maxVal, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntUint64_LargeUnsigned) {
  napi_handle_scope scope = openScope(env_);

  // A value with the high bit set — should be unsigned, not negative.
  uint64_t bigVal = uint64_t{1} << 63;
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, bigVal, &result));

  uint64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_uint64(env_, result, &val, &lossless));
  EXPECT_EQ(bigVal, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntUint64_NullEnv) {
  EXPECT_EQ(napi_invalid_arg, napi_create_bigint_uint64(nullptr, 0, nullptr));
}

TEST_F(NapiTestFixture, CreateBigIntUint64_NullResult) {
  EXPECT_EQ(napi_invalid_arg, napi_create_bigint_uint64(env_, 0, nullptr));
}

//===========================================================================
// napi_create_bigint_words
//===========================================================================

TEST_F(NapiTestFixture, CreateBigIntWords_ZeroWords) {
  napi_handle_scope scope = openScope(env_);

  uint64_t words[] = {0}; // dummy, word_count is 0
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 0, 0, words, &result));

  // Should be 0n.
  int64_t val = 99;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(0, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntWords_SingleWordPositive) {
  napi_handle_scope scope = openScope(env_);

  uint64_t words[] = {42};
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 0, 1, words, &result));

  int64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(42, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntWords_SingleWordNegative) {
  napi_handle_scope scope = openScope(env_);

  uint64_t words[] = {42};
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 1, 1, words, &result));

  int64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(-42, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntWords_TwoWordsPositive) {
  napi_handle_scope scope = openScope(env_);

  // Create a large positive number: 1 * 2^64 + 0 = 2^64.
  uint64_t words[] = {0, 1};
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 0, 2, words, &result));

  // Extracting as uint64 should not be lossless (value > UINT64_MAX).
  uint64_t val = 0;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_uint64(env_, result, &val, &lossless));
  EXPECT_EQ(0u, val); // low word
  EXPECT_FALSE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntWords_NegativeZero) {
  napi_handle_scope scope = openScope(env_);

  // -0 should be treated as 0.
  uint64_t words[] = {0};
  napi_value result = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 1, 1, words, &result));

  int64_t val = 99;
  bool lossless = false;
  ASSERT_EQ(
      napi_ok, napi_get_value_bigint_int64(env_, result, &val, &lossless));
  EXPECT_EQ(0, val);
  EXPECT_TRUE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CreateBigIntWords_NullEnv) {
  uint64_t words[] = {0};
  EXPECT_EQ(
      napi_invalid_arg,
      napi_create_bigint_words(nullptr, 0, 1, words, nullptr));
}

TEST_F(NapiTestFixture, CreateBigIntWords_NullWords) {
  napi_value result = nullptr;
  EXPECT_EQ(
      napi_invalid_arg, napi_create_bigint_words(env_, 0, 1, nullptr, &result));
}

TEST_F(NapiTestFixture, CreateBigIntWords_NullResult) {
  uint64_t words[] = {0};
  EXPECT_EQ(
      napi_invalid_arg, napi_create_bigint_words(env_, 0, 1, words, nullptr));
}

//===========================================================================
// napi_get_value_bigint_int64
//===========================================================================

TEST_F(NapiTestFixture, GetBigIntInt64_LossyFromLargePositive) {
  napi_handle_scope scope = openScope(env_);

  // Create a BigInt larger than INT64_MAX.
  uint64_t maxVal = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, maxVal + 1, &bi));

  int64_t val = 0;
  bool lossless = true;
  ASSERT_EQ(napi_ok, napi_get_value_bigint_int64(env_, bi, &val, &lossless));
  // The truncated value should be INT64_MIN (since maxVal + 1 == 2^63).
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), val);
  EXPECT_FALSE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntInt64_LossyFromNegative) {
  napi_handle_scope scope = openScope(env_);

  // Create a value smaller than INT64_MIN using words.
  // -2^63 - 1 needs two words.
  uint64_t words[] = {1, 1}; // magnitude = 2^64 + 1
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 1, 2, words, &bi));

  int64_t val = 0;
  bool lossless = true;
  ASSERT_EQ(napi_ok, napi_get_value_bigint_int64(env_, bi, &val, &lossless));
  EXPECT_FALSE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntInt64_TypeErrorOnNumber) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  int64_t val = 0;
  bool lossless = false;
  EXPECT_EQ(
      napi_bigint_expected,
      napi_get_value_bigint_int64(env_, num, &val, &lossless));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntInt64_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_value_bigint_int64(nullptr, nullptr, nullptr, nullptr));
}

TEST_F(NapiTestFixture, GetBigIntInt64_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 0, &bi));

  bool lossless = false;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_value_bigint_int64(env_, bi, nullptr, &lossless));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntInt64_NullLossless) {
  napi_handle_scope scope = openScope(env_);

  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 0, &bi));

  int64_t val = 0;
  EXPECT_EQ(
      napi_invalid_arg, napi_get_value_bigint_int64(env_, bi, &val, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_get_value_bigint_uint64
//===========================================================================

TEST_F(NapiTestFixture, GetBigIntUint64_LossyFromNegative) {
  napi_handle_scope scope = openScope(env_);

  // Negative BigInts are not losslessly representable as uint64.
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, -1, &bi));

  uint64_t val = 0;
  bool lossless = true;
  ASSERT_EQ(napi_ok, napi_get_value_bigint_uint64(env_, bi, &val, &lossless));
  EXPECT_FALSE(lossless);
  // The truncated value should be UINT64_MAX (all bits set).
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), val);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntUint64_LossyFromTooLarge) {
  napi_handle_scope scope = openScope(env_);

  // Create a value larger than UINT64_MAX using words.
  uint64_t words[] = {0, 1}; // 2^64
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 0, 2, words, &bi));

  uint64_t val = 0;
  bool lossless = true;
  ASSERT_EQ(napi_ok, napi_get_value_bigint_uint64(env_, bi, &val, &lossless));
  EXPECT_FALSE(lossless);
  EXPECT_EQ(0u, val); // low word

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntUint64_TypeErrorOnString) {
  napi_handle_scope scope = openScope(env_);

  napi_value str = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_string_utf8(env_, "hello", NAPI_AUTO_LENGTH, &str));

  uint64_t val = 0;
  bool lossless = false;
  EXPECT_EQ(
      napi_bigint_expected,
      napi_get_value_bigint_uint64(env_, str, &val, &lossless));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntUint64_NullEnv) {
  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_value_bigint_uint64(nullptr, nullptr, nullptr, nullptr));
}

//===========================================================================
// napi_get_value_bigint_words
//===========================================================================

TEST_F(NapiTestFixture, GetBigIntWords_QueryWordCount) {
  napi_handle_scope scope = openScope(env_);

  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 42, &bi));

  // Query mode: sign_bit and words are null.
  size_t word_count = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(env_, bi, nullptr, &word_count, nullptr));
  EXPECT_GE(word_count, 1u);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_PositiveRoundtrip) {
  napi_handle_scope scope = openScope(env_);

  // Create 42n.
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, 42, &bi));

  // Query word count.
  size_t word_count = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(env_, bi, nullptr, &word_count, nullptr));

  // Extract words.
  int sign_bit = -1;
  std::vector<uint64_t> words(word_count, 0);
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(
          env_, bi, &sign_bit, &word_count, words.data()));
  EXPECT_EQ(0, sign_bit);
  EXPECT_GE(word_count, 1u);
  EXPECT_EQ(42u, words[0]);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_NegativeRoundtrip) {
  napi_handle_scope scope = openScope(env_);

  // Create -42n.
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, -42, &bi));

  // Query word count.
  size_t word_count = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(env_, bi, nullptr, &word_count, nullptr));

  // Extract words.
  int sign_bit = -1;
  std::vector<uint64_t> words(word_count, 0);
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(
          env_, bi, &sign_bit, &word_count, words.data()));
  EXPECT_EQ(1, sign_bit);
  EXPECT_GE(word_count, 1u);
  EXPECT_EQ(42u, words[0]); // magnitude

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_RoundtripViaWords) {
  napi_handle_scope scope = openScope(env_);

  // Create a large number via words, then extract and compare.
  uint64_t inWords[] = {0xDEADBEEF12345678ULL, 0x1234};
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 0, 2, inWords, &bi));

  // Query word count.
  size_t word_count = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(env_, bi, nullptr, &word_count, nullptr));
  EXPECT_GE(word_count, 2u);

  // Extract.
  int sign_bit = -1;
  std::vector<uint64_t> outWords(word_count, 0);
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(
          env_, bi, &sign_bit, &word_count, outWords.data()));
  EXPECT_EQ(0, sign_bit);
  EXPECT_EQ(0xDEADBEEF12345678ULL, outWords[0]);
  EXPECT_EQ(0x1234u, outWords[1]);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_NegativeRoundtripViaWords) {
  napi_handle_scope scope = openScope(env_);

  // Create a large negative number via words, then extract and compare.
  uint64_t inWords[] = {0xABCDEF0123456789ULL, 0x42};
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_words(env_, 1, 2, inWords, &bi));

  // Query word count.
  size_t word_count = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(env_, bi, nullptr, &word_count, nullptr));

  // Extract.
  int sign_bit = -1;
  std::vector<uint64_t> outWords(word_count, 0);
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(
          env_, bi, &sign_bit, &word_count, outWords.data()));
  EXPECT_EQ(1, sign_bit);
  EXPECT_EQ(0xABCDEF0123456789ULL, outWords[0]);
  EXPECT_EQ(0x42u, outWords[1]);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_ZeroBigInt) {
  napi_handle_scope scope = openScope(env_);

  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 0, &bi));

  size_t word_count = 0;
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(env_, bi, nullptr, &word_count, nullptr));

  // Zero may use 0 or 1 words depending on implementation.
  int sign_bit = -1;
  std::vector<uint64_t> words(std::max(word_count, size_t{1}), 99);
  ASSERT_EQ(
      napi_ok,
      napi_get_value_bigint_words(
          env_, bi, &sign_bit, &word_count, words.data()));
  EXPECT_EQ(0, sign_bit);
  if (word_count > 0) {
    EXPECT_EQ(0u, words[0]);
  }

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_TypeErrorOnNumber) {
  napi_handle_scope scope = openScope(env_);

  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  size_t word_count = 0;
  EXPECT_EQ(
      napi_bigint_expected,
      napi_get_value_bigint_words(env_, num, nullptr, &word_count, nullptr));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, GetBigIntWords_NullWordCount) {
  napi_handle_scope scope = openScope(env_);

  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, 42, &bi));

  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_value_bigint_words(env_, bi, nullptr, nullptr, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// Cross-type extraction tests
//===========================================================================

TEST_F(NapiTestFixture, BigInt_Uint64AsInt64_Lossy) {
  napi_handle_scope scope = openScope(env_);

  // Create UINT64_MAX via uint64. As int64, this is -1 (lossy).
  uint64_t maxVal = std::numeric_limits<uint64_t>::max();
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_uint64(env_, maxVal, &bi));

  int64_t val = 0;
  bool lossless = true;
  ASSERT_EQ(napi_ok, napi_get_value_bigint_int64(env_, bi, &val, &lossless));
  EXPECT_EQ(-1, val); // bit pattern is all 1s
  EXPECT_FALSE(lossless);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, BigInt_Int64AsUint64_NegativeLossy) {
  napi_handle_scope scope = openScope(env_);

  // Create -1n via int64. As uint64, this is UINT64_MAX (lossy).
  napi_value bi = nullptr;
  ASSERT_EQ(napi_ok, napi_create_bigint_int64(env_, -1, &bi));

  uint64_t val = 0;
  bool lossless = true;
  ASSERT_EQ(napi_ok, napi_get_value_bigint_uint64(env_, bi, &val, &lossless));
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), val);
  EXPECT_FALSE(lossless);

  closeScope(env_, scope);
}

} // namespace
