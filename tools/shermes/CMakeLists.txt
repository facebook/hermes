# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Define targets for the C++ console bindings. We define both a static and a
# dynamic library, so they can be linked the same way as hermesvm.
add_library(shermes_console SHARED ConsoleBindings.cpp)
target_link_libraries(shermes_console PRIVATE hermesvm)

add_library(shermes_console_a STATIC ConsoleBindings.cpp)
# Add hermesvm as a dependency so headers are available, even though we aren't
# actually linking anything here.
target_link_libraries(shermes_console_a PRIVATE hermesvm_a)

set(SHERMES_CC "cc" CACHE STRING "C compiler to invoke")
set(SHERMES_CC_SYSCFLAGS "" CACHE STRING "C compiler system flags")
set(SHERMES_CC_SYSLDFLAGS "" CACHE STRING "C compiler system linker flags")
set(SHERMES_CC_INCLUDE_PATH
  "${CMAKE_BINARY_DIR}/lib/config:${CMAKE_SOURCE_DIR}/include" CACHE STRING
  "Include path when invoking CC")
set(SHERMES_CC_LIB_PATH "$<TARGET_FILE_DIR:hermesvm>:$<TARGET_FILE_DIR:jsi>:$<TARGET_FILE_DIR:shermes_console>" CACHE STRING
  "Library path when invoking CC")

file(GENERATE
        OUTPUT $<CONFIG>/config.h
        CONTENT "\
#define SHERMES_CC                    R\"(${SHERMES_CC})\"
#define SHERMES_CC_SYSCFLAGS          R\"(${SHERMES_CC_SYSCFLAGS})\"
#define SHERMES_CC_SYSLDFLAGS         R\"(${SHERMES_CC_SYSLDFLAGS})\"
#define SHERMES_CC_INCLUDE_PATH       R\"(${SHERMES_CC_INCLUDE_PATH})\"
#define SHERMES_CC_LIB_PATH           R\"(${SHERMES_CC_LIB_PATH})\""
        )

include_directories(${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

add_hermes_tool(shermes
  shermes.cpp
  ParseJSFile.cpp ParseJSFile.h
  compile.cpp compile.h
  LINK_LIBS hermescompiler dl)

# The compiler is used as part of the build of the VM.
# During cross-compilation, the compiler needs to be built for the host system,
# then used to build the VM for the target system.
# Namespace added to avoid clashing the imported binary with the target binary.
export(TARGETS shermes APPEND FILE ${CMAKE_BINARY_DIR}/ImportHostCompilers.cmake NAMESPACE imported-)

# A target building both the shermes compiler and the libraries potentially
# needed for linking of a compiled binary.
# Note that it is not a given that we always want to link against these
# libraries, we might want to link against release libraries from a debug
# compiler, so this is not the default for shermes.
add_custom_target(shermes-dep)
add_dependencies(shermes-dep shermes hermesvm_a hermesvmlean_a hermesvm hermesvmlean shermes_console shermes_console_a)
