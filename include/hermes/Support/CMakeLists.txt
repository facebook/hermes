INCLUDE (CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("
#include <type_traits>
int main() {
  static_assert(std::is_trivially_copyable<int>::value, \"int must be copyable\");
  return 0;
}"
  HAVE_IS_TRIVIALLY_COPYABLE)

configure_file(Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Config.h
               ESCAPE_QUOTES)

