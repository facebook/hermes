# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LIT_PATH "${HERMES_TOOLS_OUTPUT_DIR}/hermes-lit")
if (CMAKE_HOST_WIN32 AND NOT CYGWIN)
  # llvm-lit needs suffix.py for multiprocess to find a main module.
  set(HERMES_LIT_PATH "${HERMES_LIT_PATH}.py")
endif ()


# A raw function to create a lit target. This is used to implement the testuite
# management functions.
function(add_lit_target target comment)
  cmake_parse_arguments(ARG "" "" "PARAMS;DEPENDS;ARGS" ${ARGN})
  set(LIT_ARGS "${ARG_ARGS} ${LLVH_LIT_ARGS}")
  separate_arguments(LIT_ARGS)
  if (NOT CMAKE_CFG_INTDIR STREQUAL ".")
    list(APPEND LIT_ARGS --param build_mode=${CMAKE_CFG_INTDIR})
  endif ()

  set(LIT_COMMAND "${Python_EXECUTABLE};${HERMES_LIT_PATH}")
  list(APPEND LIT_COMMAND ${LIT_ARGS})
  foreach(param ${ARG_PARAMS})
    list(APPEND LIT_COMMAND --param ${param})
  endforeach()
  if (ARG_UNPARSED_ARGUMENTS)
    add_custom_target(${target}
      COMMAND ${LIT_COMMAND} ${ARG_UNPARSED_ARGUMENTS}
      COMMENT "${comment}"
      USES_TERMINAL
      )
  else()
    add_custom_target(${target}
      COMMAND ${CMAKE_COMMAND} -E echo "${target} does nothing, no tools built.")
    message(STATUS "${target} does nothing.")
  endif()

  # Add lit test dependencies.
  set(llvm_utils_deps
    FileCheck count not
    )
  foreach(dep ${llvm_utils_deps})
    if (TARGET ${dep})
      add_dependencies(${target} ${dep})
    endif()
  endforeach()

  if (ARG_DEPENDS)
    add_dependencies(${target} ${ARG_DEPENDS})
  endif()

  # Tests should be excluded from "Build Solution".
  set_target_properties(${target} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD ON)
endfunction()

# A function to add a set of lit test suites to be driven through 'check-*' targets.
function(add_lit_testsuite target comment)
  cmake_parse_arguments(ARG "" "" "PARAMS;DEPENDS;ARGS" ${ARGN})

  # Produce a specific suffixed check rule.
  add_lit_target(${target} ${comment}
    ${ARG_UNPARSED_ARGUMENTS}
    PARAMS ${ARG_PARAMS}
    DEPENDS ${ARG_DEPENDS}
    ARGS ${ARG_ARGS}
    )
endfunction()

function(add_unittest test_suite test_name)
  # Our current version of gtest does not properly recognize C++11 support
  # with MSVC, so it falls back to tr1 / experimental classes.  Since LLVM
  # itself requires C++11, we can safely force it on unconditionally so that
  # we don't have to fight with the buggy gtest check.
  add_definitions(-DGTEST_LANG_CXX11=1)
  add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

  include_directories(${LLVH_SOURCE_DIR}/utils/unittest/googletest/include)
  include_directories(${LLVH_SOURCE_DIR}/utils/unittest/googlemock/include)

#  if (SUPPORTS_VARIADIC_MACROS_FLAG)
#    list(APPEND LLVM_COMPILE_FLAGS "-Wno-variadic-macros")
#  endif ()
#  # Some parts of gtest rely on this GNU extension, don't warn on it.
#  if(SUPPORTS_GNU_ZERO_VARIADIC_MACRO_ARGUMENTS_FLAG)
#    list(APPEND LLVM_COMPILE_FLAGS "-Wno-gnu-zero-variadic-macro-arguments")
#  endif()

  add_hermes_executable(${test_name} ${ARGN})

  target_link_libraries(${test_name} gtest_main gtest)

  add_dependencies(${test_suite} ${test_name})
  get_target_property(test_suite_folder ${test_suite} FOLDER)
  if (NOT ${test_suite_folder} STREQUAL "NOTFOUND")
    set_property(TARGET ${test_name} PROPERTY FOLDER "${test_suite_folder}")
  endif ()
endfunction()
