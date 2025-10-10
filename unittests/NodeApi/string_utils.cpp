// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "string_utils.h"
#include <cstdarg>

namespace node_api_tests {

std::string FormatString(const char *format, ...) noexcept {
  va_list args1;
  va_start(args1, format);
  va_list args2;
  va_copy(args2, args1);
  std::string result =
      std::string(std::vsnprintf(nullptr, 0, format, args1), '\0');
  va_end(args1);
  std::vsnprintf(&result[0], result.size() + 1, format, args2);
  va_end(args2);
  return result;
}

std::string ReplaceAll(
    std::string str,
    std::string_view from,
    std::string_view to) noexcept {
  std::string result = std::move(str);
  if (from.empty())
    return result;
  size_t start_pos = 0;
  while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
    result.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case if 'to' contains 'from', like
                              // replacing 'x' with 'yx'
  }
  return result;
}

} // namespace node_api_tests