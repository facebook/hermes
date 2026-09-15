/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

#include "hermes/VM/HermesValue.h"

namespace {

using hermes::napi::NapiTestFixture;
using namespace hermes::vm;

/// Helper to open a handle scope and return the scope handle, asserting
/// success.
static napi_handle_scope openScope(napi_env env) {
  napi_handle_scope scope = nullptr;
  EXPECT_EQ(napi_ok, napi_open_handle_scope(env, &scope));
  EXPECT_NE(nullptr, scope);
  return scope;
}

/// Helper to close a handle scope, asserting success.
static void closeScope(napi_env env, napi_handle_scope scope) {
  EXPECT_EQ(napi_ok, napi_close_handle_scope(env, scope));
}

//===========================================================================
// napi_get_cb_info tests
//===========================================================================

class NapiFunctionTest : public NapiTestFixture {
 protected:
  /// Build a napi_callback_info__ from an array of PinnedHermesValue.
  /// The array layout must be:
  ///   [argN-1, ..., arg1, arg0, thisVal]
  /// where thisVal is the LAST element, and args are in reverse order
  /// before it (matching the Hermes stack frame layout where args are
  /// at decreasing addresses from 'this').
  ///
  /// \param frame pointer to the first element of the frame array.
  /// \param frameSize total number of elements in the frame array.
  /// \param argc number of JS arguments (excluding 'this').
  /// \param data user data pointer.
  /// \param newTarget pointer to a PinnedHermesValue for new.target
  ///   (nullptr means use a default 'undefined').
  napi_callback_info__ makeCallbackInfo(
      PinnedHermesValue *frame,
      size_t frameSize,
      unsigned argc,
      void *data,
      PinnedHermesValue *newTarget) {
    // 'this' is the last element in the frame array.
    PinnedHermesValue *thisArg = &frame[frameSize - 1];

    napi_callback_info__ cbinfo;
    cbinfo.env = env_;
    cbinfo.thisArg = thisArg;
    cbinfo.argsBase = thisArg;
    cbinfo.argc = argc;
    cbinfo.data = data;
    cbinfo.newTarget = newTarget ? newTarget : &undefinedVal_;
    return cbinfo;
  }

  PinnedHermesValue undefinedVal_{};
};

TEST_F(NapiFunctionTest, GetCbInfoNullEnv) {
  napi_callback_info__ cbinfo{};
  size_t argc = 0;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_cb_info(nullptr, &cbinfo, &argc, nullptr, nullptr, nullptr));
}

TEST_F(NapiFunctionTest, GetCbInfoNullCbinfo) {
  auto scope = openScope(env_);
  size_t argc = 0;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_cb_info(env_, nullptr, &argc, nullptr, nullptr, nullptr));
  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoArgcOnly) {
  auto scope = openScope(env_);

  // Frame: [this_val]
  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeTrustedNumberValue(42);

  auto cbinfo = makeCallbackInfo(frame, 1, 0, nullptr, nullptr);

  size_t argc = 99;
  EXPECT_EQ(
      napi_ok,
      napi_get_cb_info(env_, &cbinfo, &argc, nullptr, nullptr, nullptr));
  EXPECT_EQ(0u, argc);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoThisArg) {
  auto scope = openScope(env_);

  // Frame: [this_val], this is number 42.
  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeTrustedNumberValue(42);

  auto cbinfo = makeCallbackInfo(frame, 1, 0, nullptr, nullptr);

  napi_value this_arg = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_get_cb_info(env_, &cbinfo, nullptr, nullptr, &this_arg, nullptr));
  ASSERT_NE(nullptr, this_arg);

  // Extract the this value and verify it's 42.
  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, this_arg, &val));
  EXPECT_EQ(42.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoData) {
  auto scope = openScope(env_);

  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeUndefinedValue();

  int userData = 123;
  auto cbinfo = makeCallbackInfo(frame, 1, 0, &userData, nullptr);

  void *data = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_get_cb_info(env_, &cbinfo, nullptr, nullptr, nullptr, &data));
  EXPECT_EQ(&userData, data);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoWithArgs) {
  auto scope = openScope(env_);

  // Frame layout (in array order):
  //   [arg2, arg1, arg0, this_val]
  // arg0 = 10, arg1 = 20, arg2 = 30, this = undefined
  PinnedHermesValue frame[4];
  frame[0] = HermesValue::encodeTrustedNumberValue(30); // arg2
  frame[1] = HermesValue::encodeTrustedNumberValue(20); // arg1
  frame[2] = HermesValue::encodeTrustedNumberValue(10); // arg0
  frame[3] = HermesValue::encodeUndefinedValue(); // this

  auto cbinfo = makeCallbackInfo(frame, 4, 3, nullptr, nullptr);

  // Request all 3 args.
  size_t argc = 3;
  napi_value argv[3] = {};
  napi_value this_arg = nullptr;
  EXPECT_EQ(
      napi_ok,
      napi_get_cb_info(env_, &cbinfo, &argc, argv, &this_arg, nullptr));
  EXPECT_EQ(3u, argc);

  // Verify arg values.
  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, argv[0], &val));
  EXPECT_EQ(10.0, val);

  EXPECT_EQ(napi_ok, napi_get_value_double(env_, argv[1], &val));
  EXPECT_EQ(20.0, val);

  EXPECT_EQ(napi_ok, napi_get_value_double(env_, argv[2], &val));
  EXPECT_EQ(30.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoFewerArgsThanRequested) {
  auto scope = openScope(env_);

  // Frame: [arg0, this_val]
  // 1 actual arg, but we request 3.
  PinnedHermesValue frame[2];
  frame[0] = HermesValue::encodeTrustedNumberValue(99); // arg0
  frame[1] = HermesValue::encodeUndefinedValue(); // this

  auto cbinfo = makeCallbackInfo(frame, 2, 1, nullptr, nullptr);

  size_t argc = 3;
  napi_value argv[3] = {};
  EXPECT_EQ(
      napi_ok, napi_get_cb_info(env_, &cbinfo, &argc, argv, nullptr, nullptr));
  // argc is set to actual count.
  EXPECT_EQ(1u, argc);

  // First arg is the actual value.
  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, argv[0], &val));
  EXPECT_EQ(99.0, val);

  // Remaining args are undefined.
  napi_valuetype type;
  EXPECT_EQ(napi_ok, napi_typeof(env_, argv[1], &type));
  EXPECT_EQ(napi_undefined, type);

  EXPECT_EQ(napi_ok, napi_typeof(env_, argv[2], &type));
  EXPECT_EQ(napi_undefined, type);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoMoreArgsThanRequested) {
  auto scope = openScope(env_);

  // Frame: [arg2, arg1, arg0, this_val]
  // 3 actual args, but we only request 1.
  PinnedHermesValue frame[4];
  frame[0] = HermesValue::encodeTrustedNumberValue(30);
  frame[1] = HermesValue::encodeTrustedNumberValue(20);
  frame[2] = HermesValue::encodeTrustedNumberValue(10);
  frame[3] = HermesValue::encodeUndefinedValue();

  auto cbinfo = makeCallbackInfo(frame, 4, 3, nullptr, nullptr);

  size_t argc = 1;
  napi_value argv[1] = {};
  EXPECT_EQ(
      napi_ok, napi_get_cb_info(env_, &cbinfo, &argc, argv, nullptr, nullptr));
  // argc is set to actual count (3), even though we only got 1 in argv.
  EXPECT_EQ(3u, argc);

  // Only the first arg is copied.
  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, argv[0], &val));
  EXPECT_EQ(10.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoAllFields) {
  auto scope = openScope(env_);

  // Frame: [arg1, arg0, this_val]
  PinnedHermesValue frame[3];
  frame[0] = HermesValue::encodeBoolValue(true); // arg1
  frame[1] = HermesValue::encodeTrustedNumberValue(7); // arg0
  frame[2] = HermesValue::encodeTrustedNumberValue(100); // this

  int userData = 42;
  auto cbinfo = makeCallbackInfo(frame, 3, 2, &userData, nullptr);

  size_t argc = 2;
  napi_value argv[2] = {};
  napi_value this_arg = nullptr;
  void *data = nullptr;
  EXPECT_EQ(
      napi_ok, napi_get_cb_info(env_, &cbinfo, &argc, argv, &this_arg, &data));

  // Verify all fields.
  EXPECT_EQ(2u, argc);
  EXPECT_EQ(&userData, data);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, this_arg, &val));
  EXPECT_EQ(100.0, val);

  EXPECT_EQ(napi_ok, napi_get_value_double(env_, argv[0], &val));
  EXPECT_EQ(7.0, val);

  bool bval = false;
  EXPECT_EQ(napi_ok, napi_get_value_bool(env_, argv[1], &bval));
  EXPECT_EQ(true, bval);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetCbInfoArgvRequiresArgc) {
  auto scope = openScope(env_);

  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeUndefinedValue();
  auto cbinfo = makeCallbackInfo(frame, 1, 0, nullptr, nullptr);

  // Passing argv but no argc should fail.
  napi_value argv[1] = {};
  EXPECT_EQ(
      napi_invalid_arg,
      napi_get_cb_info(env_, &cbinfo, nullptr, argv, nullptr, nullptr));

  closeScope(env_, scope);
}

//===========================================================================
// napi_get_new_target tests
//===========================================================================

TEST_F(NapiFunctionTest, GetNewTargetRegularCall) {
  auto scope = openScope(env_);

  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeUndefinedValue(); // this

  // For a regular call, new.target is undefined (default).
  auto cbinfo = makeCallbackInfo(frame, 1, 0, nullptr, nullptr);

  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_get_new_target(env_, &cbinfo, &result));
  // For regular calls, result should be nullptr.
  EXPECT_EQ(nullptr, result);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetNewTargetConstructorCall) {
  auto scope = openScope(env_);

  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeUndefinedValue(); // this

  // Simulate a constructor call by setting new.target to a non-undefined
  // value (e.g., a number for testing purposes).
  PinnedHermesValue newTarget;
  newTarget = HermesValue::encodeTrustedNumberValue(99);

  auto cbinfo = makeCallbackInfo(frame, 1, 0, nullptr, &newTarget);

  napi_value result = nullptr;
  EXPECT_EQ(napi_ok, napi_get_new_target(env_, &cbinfo, &result));
  ASSERT_NE(nullptr, result);

  double val = 0;
  EXPECT_EQ(napi_ok, napi_get_value_double(env_, result, &val));
  EXPECT_EQ(99.0, val);

  closeScope(env_, scope);
}

TEST_F(NapiFunctionTest, GetNewTargetNullEnv) {
  napi_callback_info__ cbinfo{};
  napi_value result = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_new_target(nullptr, &cbinfo, &result));
}

TEST_F(NapiFunctionTest, GetNewTargetNullCbinfo) {
  napi_value result = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_new_target(env_, nullptr, &result));
}

TEST_F(NapiFunctionTest, GetNewTargetNullResult) {
  napi_callback_info__ cbinfo{};
  EXPECT_EQ(napi_invalid_arg, napi_get_new_target(env_, &cbinfo, nullptr));
}

TEST_F(NapiFunctionTest, GetCbInfoZeroArgs) {
  auto scope = openScope(env_);

  // Frame: [this_val] with no args.
  PinnedHermesValue frame[1];
  frame[0] = HermesValue::encodeTrustedNumberValue(55);

  auto cbinfo = makeCallbackInfo(frame, 1, 0, nullptr, nullptr);

  // Request 0 args.
  size_t argc = 0;
  EXPECT_EQ(
      napi_ok,
      napi_get_cb_info(env_, &cbinfo, &argc, nullptr, nullptr, nullptr));
  EXPECT_EQ(0u, argc);

  closeScope(env_, scope);
}

} // namespace
