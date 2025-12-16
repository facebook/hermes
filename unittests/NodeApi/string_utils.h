// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef NODE_API_TEST_STRING_UTILS_H
#define NODE_API_TEST_STRING_UTILS_H

#include <string>
#include <string_view>

namespace node_api_tests {

std::string FormatString(const char *format, ...) noexcept;

std::string ReplaceAll(
    std::string str,
    std::string_view from,
    std::string_view to) noexcept;

} // namespace node_api_tests

#endif // !NODE_API_TEST_STRING_UTILS_H