// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef NODE_API_TEST_CHILD_PROCESS_H
#define NODE_API_TEST_CHILD_PROCESS_H

#include <string>
#include <string_view>
#include <vector>

namespace node_api_tests {

// Struct to hold the result of a child process execution.
struct ProcessResult {
  uint32_t status;         // Exit status of the child process.
  std::string std_output;  // Standard output from the child process.
  std::string std_error;   // Standard error from the child process.
};

// Creates a child process to run the given command with the specified
// arguments.
ProcessResult SpawnSync(std::string_view command,
                        std::vector<std::string> args);

}  // namespace node_api_tests

#endif  // !NODE_API_TEST_CHILD_PROCESS_H