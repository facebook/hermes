/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace {

using hermes::napi::NapiTestFixture;

/// Helper to open a handle scope and return the scope handle.
static napi_handle_scope openScope(napi_env env) {
  napi_handle_scope scope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_handle_scope(env, &scope));
  return scope;
}

/// Helper to close a handle scope.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_type_tag_object - argument validation
//===========================================================================

TEST_F(NapiTestFixture, TypeTagObject_NullEnv) {
  napi_type_tag tag = {1, 2};
  EXPECT_EQ(napi_invalid_arg, napi_type_tag_object(nullptr, nullptr, &tag));
}

TEST_F(NapiTestFixture, TypeTagObject_NullValue) {
  napi_type_tag tag = {1, 2};
  napi_handle_scope scope = openScope(env_);
  EXPECT_EQ(napi_invalid_arg, napi_type_tag_object(env_, nullptr, &tag));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagObject_NullTag) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));
  EXPECT_EQ(napi_invalid_arg, napi_type_tag_object(env_, obj, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_type_tag_object - regular objects
//===========================================================================

TEST_F(NapiTestFixture, TypeTagObject_BasicObject) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_type_tag tag = {0xDEADBEEFCAFEBABEULL, 0x1234567890ABCDEFULL};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, obj, &tag));

  // Verify the tag.
  bool result = false;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj, &tag, &result));
  EXPECT_TRUE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagObject_RetagFails) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_type_tag tag1 = {1, 2};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, obj, &tag1));

  // Re-tagging should fail with napi_invalid_arg.
  napi_type_tag tag2 = {3, 4};
  EXPECT_EQ(napi_invalid_arg, napi_type_tag_object(env_, obj, &tag2));

  // Original tag should still be valid.
  bool result = false;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj, &tag1, &result));
  EXPECT_TRUE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagObject_WrongTagReturnsFalse) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_type_tag tag = {1, 2};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, obj, &tag));

  // Check with a different tag.
  napi_type_tag wrongTag = {3, 4};
  bool result = true;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj, &wrongTag, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagObject_UntaggedReturnsFalse) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // Check an untagged object.
  napi_type_tag tag = {1, 2};
  bool result = true;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj, &tag, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagObject_ZeroTag) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  // Zero tag should work like any other tag.
  napi_type_tag zeroTag = {0, 0};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, obj, &zeroTag));

  bool result = false;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj, &zeroTag, &result));
  EXPECT_TRUE(result);

  // Non-zero tag should not match.
  napi_type_tag otherTag = {1, 0};
  result = true;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj, &otherTag, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

//===========================================================================
// napi_type_tag_object - external values
//===========================================================================

TEST_F(NapiTestFixture, TypeTagExternal_Basic) {
  napi_handle_scope scope = openScope(env_);

  int nativeData = 42;
  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, &nativeData, nullptr, nullptr, &ext));

  napi_type_tag tag = {0x1111111111111111ULL, 0x2222222222222222ULL};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, ext, &tag));

  // Verify the tag.
  bool result = false;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, ext, &tag, &result));
  EXPECT_TRUE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagExternal_RetagFails) {
  napi_handle_scope scope = openScope(env_);

  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, nullptr, nullptr, nullptr, &ext));

  napi_type_tag tag1 = {1, 2};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, ext, &tag1));

  // Re-tagging should fail.
  napi_type_tag tag2 = {3, 4};
  EXPECT_EQ(napi_invalid_arg, napi_type_tag_object(env_, ext, &tag2));

  // Original tag should still be valid.
  bool result = false;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, ext, &tag1, &result));
  EXPECT_TRUE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagExternal_WrongTagReturnsFalse) {
  napi_handle_scope scope = openScope(env_);

  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, nullptr, nullptr, nullptr, &ext));

  napi_type_tag tag = {1, 2};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, ext, &tag));

  napi_type_tag wrongTag = {5, 6};
  bool result = true;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, ext, &wrongTag, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, TypeTagExternal_UntaggedReturnsFalse) {
  napi_handle_scope scope = openScope(env_);

  napi_value ext = nullptr;
  ASSERT_EQ(
      napi_ok, napi_create_external(env_, nullptr, nullptr, nullptr, &ext));

  napi_type_tag tag = {1, 2};
  bool result = true;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, ext, &tag, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

//===========================================================================
// napi_check_object_type_tag - argument validation
//===========================================================================

TEST_F(NapiTestFixture, CheckTypeTag_NullEnv) {
  napi_type_tag tag = {1, 2};
  bool result = false;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_check_object_type_tag(nullptr, nullptr, &tag, &result));
}

TEST_F(NapiTestFixture, CheckTypeTag_NullValue) {
  napi_type_tag tag = {1, 2};
  bool result = false;
  napi_handle_scope scope = openScope(env_);
  EXPECT_EQ(
      napi_invalid_arg,
      napi_check_object_type_tag(env_, nullptr, &tag, &result));
  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CheckTypeTag_NullTag) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  bool result = false;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_check_object_type_tag(env_, obj, nullptr, &result));

  closeScope(env_, scope);
}

TEST_F(NapiTestFixture, CheckTypeTag_NullResult) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj));

  napi_type_tag tag = {1, 2};
  EXPECT_EQ(
      napi_invalid_arg, napi_check_object_type_tag(env_, obj, &tag, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_check_object_type_tag - non-object returns false
//===========================================================================

TEST_F(NapiTestFixture, CheckTypeTag_NonObjectReturnsFalse) {
  napi_handle_scope scope = openScope(env_);

  // Check with a number (non-object).
  napi_value num = nullptr;
  ASSERT_EQ(napi_ok, napi_create_double(env_, 42.0, &num));

  napi_type_tag tag = {1, 2};
  bool result = true;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, num, &tag, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

//===========================================================================
// Multiple objects with different tags
//===========================================================================

TEST_F(NapiTestFixture, TypeTag_MultipleObjects) {
  napi_handle_scope scope = openScope(env_);

  napi_value obj1 = nullptr;
  napi_value obj2 = nullptr;
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj1));
  ASSERT_EQ(napi_ok, napi_create_object(env_, &obj2));

  napi_type_tag tag1 = {1, 0};
  napi_type_tag tag2 = {2, 0};
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, obj1, &tag1));
  ASSERT_EQ(napi_ok, napi_type_tag_object(env_, obj2, &tag2));

  // Each object should match its own tag.
  bool result = false;
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj1, &tag1, &result));
  EXPECT_TRUE(result);

  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj2, &tag2, &result));
  EXPECT_TRUE(result);

  // And not match the other's tag.
  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj1, &tag2, &result));
  EXPECT_FALSE(result);

  ASSERT_EQ(napi_ok, napi_check_object_type_tag(env_, obj2, &tag1, &result));
  EXPECT_FALSE(result);

  closeScope(env_, scope);
}

} // namespace
