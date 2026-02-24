/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <hermes/cdp/MessageTypes.h>
#include <hermes/cdp/RemoteObjectConverters.h>
#include <hermes/cdp/RemoteObjectsTable.h>
#include <hermes/hermes.h>

using namespace facebook::hermes::cdp;
using namespace facebook::hermes;
using namespace facebook;

class RemoteObjectConvertersTest : public ::testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  jsi::Value eval(const char *code) {
    return runtime_->global()
        .getPropertyAsFunction(*runtime_, "eval")
        .call(*runtime_, code);
  }

  std::unique_ptr<HermesRuntime> runtime_;
  std::unique_ptr<RemoteObjectsTable> objTable_;
};

void RemoteObjectConvertersTest::SetUp() {
  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  runtime_ = facebook::hermes::makeHermesRuntime(builder.build());
  objTable_ = std::make_unique<RemoteObjectsTable>();
}

void RemoteObjectConvertersTest::TearDown() {
  objTable_.reset();
  runtime_.reset();
}

TEST_F(RemoteObjectConvertersTest, ErrorWithMessage) {
  jsi::Value errorVal = eval("new Error('simple error')");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");
  EXPECT_EQ(remoteObj.className, "Error");

  // Check that description starts with "Error: simple error"
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_TRUE(remoteObj.description->find("Error: simple error") == 0);

  // Check that stack trace is included
  EXPECT_TRUE(remoteObj.description->find("\n    at") != std::string::npos);
}

TEST_F(RemoteObjectConvertersTest, ErrorWithEmptyStringMessage) {
  jsi::Value errorVal = eval("new Error('')");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // Empty message should have no colon, just "Error" followed by stack
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_TRUE(remoteObj.description->find("Error\n") == 0);
  EXPECT_TRUE(remoteObj.description->find("Error:") == std::string::npos);
}

TEST_F(RemoteObjectConvertersTest, ErrorWithNoMessage) {
  jsi::Value errorVal = eval("new Error()");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // No message should have no colon, just "Error" followed by stack
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_TRUE(remoteObj.description->find("Error\n") == 0);
  EXPECT_TRUE(remoteObj.description->find("Error:") == std::string::npos);
}

TEST_F(RemoteObjectConvertersTest, ErrorWithCustomName) {
  jsi::Value errorVal = eval(R"(
    (function() {
      const err = new Error('some message');
      err.name = 'CustomNameError';
      return err;
    })()
  )");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // Should use custom name
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_TRUE(
      remoteObj.description->find("CustomNameError: some message") == 0);
}

TEST_F(RemoteObjectConvertersTest, ErrorWithCustomNameEmptyMessage) {
  jsi::Value errorVal = eval(R"(
    (function() {
      const err = new Error();
      err.name = 'CustomNameError';
      return err;
    })()
  )");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // Should use custom name with no colon
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_TRUE(remoteObj.description->find("CustomNameError\n") == 0);
  EXPECT_TRUE(
      remoteObj.description->find("CustomNameError:") == std::string::npos);
}

TEST_F(RemoteObjectConvertersTest, CustomErrorClass) {
  jsi::Value errorVal = eval(R"(
    (function() {
      // Transpiled class CustomClassError extends Error {}
      function CustomClassError(...args) {
        const instance = Reflect.construct(Error, args);
        Reflect.setPrototypeOf(instance, Reflect.getPrototypeOf(this));
        return instance;
      }
      Object.setPrototypeOf(CustomClassError.prototype, Error.prototype);
      return new CustomClassError('custom class error');
    })()
  )");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // Should use constructor name for custom error classes
  EXPECT_TRUE(remoteObj.description.has_value());
  // Strict check - must be CustomClassError, not Error
  EXPECT_TRUE(
      remoteObj.description->find("CustomClassError: custom class error") == 0);
}

TEST_F(RemoteObjectConvertersTest, ErrorWithEmptyStack) {
  jsi::Value errorVal = eval(R"(
    (function() {
      const err = new Error('message');
      err.stack = '';
      return err;
    })()
  )");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // Empty stack should just show "Error: message" with no stack trace
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_EQ(*remoteObj.description, "Error: message");
}

TEST_F(RemoteObjectConvertersTest, ErrorWithNonV8FormatStack) {
  jsi::Value errorVal = eval(R"(
    (function() {
      const err = new Error('message');
      err.stack = 'Error: message\n unexpected stack format\nline 3';
      return err;
    })()
  )");

  ObjectSerializationOptions options;
  options.generatePreview = false;

  auto remoteObj = message::runtime::makeRemoteObject(
      *runtime_, errorVal, *objTable_, "test-group", options);

  EXPECT_EQ(remoteObj.type, "object");
  EXPECT_EQ(remoteObj.subtype, "error");

  // Non-V8 format stack should be included with newline prefix.
  // Since we can't trim it safely, the error line appears twice -
  // once from our name+message construction, and once from the stack itself.
  EXPECT_TRUE(remoteObj.description.has_value());
  EXPECT_EQ(
      *remoteObj.description,
      "Error: message\nError: message\n unexpected stack format\nline 3");
}
