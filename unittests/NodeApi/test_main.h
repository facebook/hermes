// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef NODE_API_TEST_TEST_MAIN_H
#define NODE_API_TEST_TEST_MAIN_H

#include <gtest/gtest.h>
#include <filesystem>

namespace node_api_tests {

class TestFixtureBase : public ::testing::Test {
 public:
  static void InitializeGlobals(const char* test_exe_path) noexcept;

 protected:
  static std::filesystem::path node_lite_path_;
  static std::filesystem::path js_root_dir_;
};

}  // namespace node_api_tests

#endif  // !NODE_API_TEST_TEST_MAIN_H