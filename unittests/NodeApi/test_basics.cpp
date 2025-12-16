// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include "child_process.h"
#include "test_main.h"

namespace fs = std::filesystem;

namespace node_api_tests {

class BasicsTest : public TestFixtureBase {
 protected:
  void SetUp() override { basics_js_dir_ = js_root_dir_ / "basics"; }

  ProcessResult RunScript(std::string_view script_filename) noexcept {
    return SpawnSync(node_lite_path_.string(),
                     {(basics_js_dir_ / script_filename).string()});
  }

  bool StringContains(std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string::npos;
  }

 private:
  fs::path basics_js_dir_;
};

TEST_F(BasicsTest, TestHello) {
  ProcessResult result = RunScript("hello.js");
  ASSERT_TRUE(StringContains(result.std_output, "Hello"));
}

TEST_F(BasicsTest, TestThrowString) {
  ProcessResult result = RunScript("throw_string.js");
  ASSERT_TRUE(StringContains(result.std_error, "Script failed"));
}

TEST_F(BasicsTest, TestAsyncResolved) {
  ProcessResult result = RunScript("async_resolved.js");
  ASSERT_TRUE(StringContains(result.std_output, "test async calling"));
  ASSERT_TRUE(
      StringContains(result.std_output, "Expected: test async resolved"));
}

TEST_F(BasicsTest, TestAsyncRejected) {
  ProcessResult result = RunScript("async_rejected.js");
  ASSERT_TRUE(StringContains(result.std_output, "test async calling"));
  ASSERT_TRUE(
      StringContains(result.std_error, "Expected: test async rejected"));
}

TEST_F(BasicsTest, TestMustCallSuccess) {
  ProcessResult result = RunScript("mustcall_success.js");
  ASSERT_TRUE(result.status == 0);
}

TEST_F(BasicsTest, TestMustCallFailure) {
  ProcessResult result = RunScript("mustcall_failure.js");
  ASSERT_TRUE(result.status != 0);
  ASSERT_TRUE(
      StringContains(result.std_error, "Mismatched noop function calls"));
}

TEST_F(BasicsTest, TestMustNotCallSuccess) {
  ProcessResult result = RunScript("mustnotcall_success.js");
  ASSERT_TRUE(result.status == 0);
}

TEST_F(BasicsTest, TestMustNotCallFailure) {
  ProcessResult result = RunScript("mustnotcall_failure.js");
  ASSERT_TRUE(result.status != 0);
  ASSERT_TRUE(
      StringContains(result.std_error, "Function should not have been called"));
}

}  // namespace node_api_tests
