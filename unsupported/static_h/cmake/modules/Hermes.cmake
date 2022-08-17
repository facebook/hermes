# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckCXXSourceCompiles)
include(CMakePrintHelpers)

set(HERMES_TOOLS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(GCC 1)
  set(GCC_COMPATIBLE 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG 1)
  set(GCC_COMPATIBLE 1)
  if (CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
    set(CLANG_CL 1)
  endif ()
endif ()

# Emscripten with Fastcomp backend in WASM mode generates trapping conversions
# from float/double to int. We require non-trapping behavior.
if (EMSCRIPTEN AND EMSCRIPTEN_FASTCOMP)
  # Note that this flag only affects Wasm, not Asm.js.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s BINARYEN_TRAP_MODE=clamp")
endif()

# For compatibility, CMake adds /EHsc by default for MSVC. We want to set that
# flag per target, so remove it.
if (MSVC)
  string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif (MSVC)

# set stack reserved size to ~10MB
if (MSVC)
  # CMake previously automatically set this value for MSVC builds, but the
  # behavior was changed in CMake 2.8.11 (Issue 12437) to use the MSVC default
  # value (1 MB) which is not enough for some of our stack overflow tests.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
elseif (MINGW) # FIXME: Also cygwin?
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,16777216")
endif ()

if (WIN32)
  set(LLVM_HAVE_LINK_VERSION_SCRIPT 0)
  if (CYGWIN)
    set(LLVM_ON_WIN32 0)
    set(LLVM_ON_UNIX 1)
  else (CYGWIN)
    set(LLVM_ON_WIN32 1)
    set(LLVM_ON_UNIX 0)
  endif (CYGWIN)
else (WIN32)
  if (FUCHSIA OR UNIX)
    set(LLVM_ON_WIN32 0)
    set(LLVM_ON_UNIX 1)
    if (APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "AIX")
      set(LLVM_HAVE_LINK_VERSION_SCRIPT 0)
    else ()
      set(LLVM_HAVE_LINK_VERSION_SCRIPT 1)
    endif ()
  else (FUCHSIA OR UNIX)
    MESSAGE(SEND_ERROR "Unable to determine platform")
  endif (FUCHSIA OR UNIX)
endif (WIN32)

function(hermes_update_compile_flags name)
  get_property(sources TARGET ${name} PROPERTY SOURCES)
  if ("${sources}" MATCHES "\\.c(;|$)")
    set(update_src_props ON)
  endif ()

  set(flags "")

  if (HERMES_ENABLE_EH)
    if (GCC_COMPATIBLE)
      set(flags "${flags} -fexceptions")
    elseif (MSVC)
      set(flags "${flags} /EHsc")
    endif ()
  else ()
    if (GCC_COMPATIBLE)
      set(flags "${flags} -fno-exceptions")
    elseif (MSVC)
      set(flags "${flags} /EHs-c-")
    endif ()
  endif ()

  if (HERMES_ENABLE_RTTI)
    if (GCC_COMPATIBLE)
      set(flags "${flags} -frtti")
    elseif (MSVC)
      set(flags "${flags} /GR")
    endif ()
  else ()
    if (GCC_COMPATIBLE)
      set(flags "${flags} -fno-rtti")
    elseif (MSVC)
      set(flags "${flags} /GR-")
    endif ()
  endif ()

  if (update_src_props)
    foreach (fn ${sources})
      get_filename_component(suf ${fn} EXT)
      if ("${suf}" STREQUAL ".cpp")
        set_property(SOURCE ${fn} APPEND_STRING PROPERTY
          COMPILE_FLAGS "${flags}")
      endif ()
    endforeach ()
  else ()
    # Update target props, since all sources are C++.
    set_property(TARGET ${name} APPEND_STRING PROPERTY
      COMPILE_FLAGS "${flags}")
  endif ()
endfunction()

function(add_hermes_library name)
  cmake_parse_arguments(ARG "" "" "LINK_LIBS" ${ARGN})
  add_library(${name} STATIC ${ARG_UNPARSED_ARGUMENTS})
  target_link_libraries(${name} ${ARG_LINK_LIBS} ${HERMES_LINK_COMPONENTS})
  set_property(TARGET ${name} PROPERTY POSITION_INDEPENDENT_CODE ON)
  hermes_update_compile_flags(${name})
  if (HERMES_ENABLE_BITCODE)
    target_compile_options(${name} PUBLIC "-fembed-bitcode")
  endif ()
endfunction(add_hermes_library)

function(add_hermes_executable name)
  cmake_parse_arguments(ARG "" "" "LINK_LIBS" ${ARGN})
  add_executable(${name} ${ARG_UNPARSED_ARGUMENTS})
  target_link_libraries(${name} ${ARG_LINK_LIBS} ${HERMES_LINK_COMPONENTS})
  target_link_options(${name} PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})
  hermes_update_compile_flags(${name})
endfunction(add_hermes_executable)

function(add_hermes_tool name)
  add_hermes_executable(${name} ${ARGN})

  # In multi-config build systems, remove the per-config directory and let
  # CMake automatically add it in the component.
  if (NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
    string(REPLACE ${CMAKE_CFG_INTDIR} "" HERMES_TOOLS_OUTPUT_DIR "${HERMES_TOOLS_OUTPUT_DIR}")
  endif ()

  set_property(TARGET ${name} PROPERTY RUNTIME_OUTPUT_DIRECTORY
    "${HERMES_TOOLS_OUTPUT_DIR}")
endfunction(add_hermes_tool)

# find_package()is not able to find packages specified with <name>_DIR if
# CMAKE_SYSROOT or CMAKE_FIND_ROOT_PATH is set, because find_file() is being
# restricted in where it looks.
# This wrapper adds <name>_DIR to CMAKE_FIND_ROOT_PATH so find_package() can
# find it.
macro(find_global_package name)
  # If we have a CMAKE_SYSROOT or CMAKE_FIND_ROOT_PATH and <name>_ROOT is set,
  # temporarily add <name>_ROOT to the search path
  if (((NOT "${CMAKE_SYSROOT}" STREQUAL "") OR
  (NOT "${CMAKE_FIND_ROOT_PATH}" STREQUAL "")) AND
  (NOT "${${name}_ROOT}" STREQUAL ""))

    list(APPEND CMAKE_FIND_ROOT_PATH "${${name}_ROOT}")
    find_package(${name} ${ARGN})
    list(REMOVE_AT CMAKE_FIND_ROOT_PATH -1)

  else ()

    find_package(${name} ${ARGN})

  endif ()
endmacro(find_global_package)

function(append value)
  foreach (variable ${ARGN})
    set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
  endforeach (variable)
endfunction()

function(append_if condition value)
  if (${condition})
    foreach (variable ${ARGN})
      set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    endforeach (variable)
  endif ()
endfunction()

macro(add_flag_if_supported flag name)
  check_c_compiler_flag("-Werror ${flag}" "C_SUPPORTS_${name}")
  append_if("C_SUPPORTS_${name}" "${flag}" CMAKE_C_FLAGS)
  check_cxx_compiler_flag("-Werror ${flag}" "CXX_SUPPORTS_${name}")
  append_if("CXX_SUPPORTS_${name}" "${flag}" CMAKE_CXX_FLAGS)
endmacro()

function(add_flag_or_print_warning flag name)
  check_c_compiler_flag("-Werror ${flag}" "C_SUPPORTS_${name}")
  check_cxx_compiler_flag("-Werror ${flag}" "CXX_SUPPORTS_${name}")
  if (C_SUPPORTS_${name} AND CXX_SUPPORTS_${name})
    message(STATUS "Building with ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${flag}" PARENT_SCOPE)
  else ()
    message(WARNING "${flag} is not supported.")
  endif ()
endfunction()


#
# Enable warnings
#
if (XCODE)
  # For Xcode enable several build settings that correspond to
  # many warnings that are on by default in Clang but are
  # not enabled for historical reasons.  For versions of Xcode
  # that do not support these options they will simply
  # be ignored.
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VALUE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION "YES")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR "YES")
endif ()

if (MSVC)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
    # For MSVC 2013, disable iterator null pointer checking in debug mode,
    # especially so std::equal(nullptr, nullptr, nullptr) will not assert.
    add_definitions("-D_DEBUG_POINTER_IMPL=")
  endif ()

  if (MSVC11)
    add_definitions(-D_VARIADIC_MAX=10)
  endif ()

  # Add definitions that make MSVC much less annoying.
  add_definitions(
    # For some reason MS wants to deprecate a bunch of standard functions...
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_SCL_SECURE_NO_DEPRECATE
    -D_SCL_SECURE_NO_WARNINGS
  )

  # Tell MSVC to use the Unicode version of the Win32 APIs instead of ANSI.
  #    add_definitions(
  #      -DUNICODE
  #      -D_UNICODE
  #    )

  # Turn warnings into errors.
  #    append("/WX" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

  # Remove unreferenced data or functions that are COMDATs, or that only have
  # internal linkage.
  append("/Zc:inline" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

  # "Generate Intrinsic Functions".
  append("/Oi" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

  # "Enforce type conversion rules".
  append("/Zc:rvalueCast" CMAKE_CXX_FLAGS)

  if (NOT CLANG_CL)
    set(msvc_warning_flags
      # Disabled warnings.
      -wd4141 # Suppress ''modifier' : used more than once' (because of __forceinline combined with inline)
      -wd4146 # Suppress 'unary minus operator applied to unsigned type, result still unsigned'
      -wd4180 # Suppress 'qualifier applied to function type has no meaning; ignored'
      -wd4244 # Suppress ''argument' : conversion from 'type1' to 'type2', possible loss of data'
      -wd4258 # Suppress ''var' : definition from the for loop is ignored; the definition from the enclosing scope is used'
      -wd4267 # Suppress ''var' : conversion from 'size_t' to 'type', possible loss of data'
      -wd4291 # Suppress ''declaration' : no matching operator delete found; memory will not be freed if initialization throws an exception'
      -wd4345 # Suppress 'behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized'
      -wd4351 # Suppress 'new behavior: elements of array 'array' will be default initialized'
      -wd4355 # Suppress ''this' : used in base member initializer list'
      -wd4456 # Suppress 'declaration of 'var' hides local variable'
      -wd4457 # Suppress 'declaration of 'var' hides function parameter'
      -wd4458 # Suppress 'declaration of 'var' hides class member'
      -wd4459 # Suppress 'declaration of 'var' hides global declaration'
      -wd4503 # Suppress ''identifier' : decorated name length exceeded, name was truncated'
      -wd4624 # Suppress ''derived class' : destructor could not be generated because a base class destructor is inaccessible'
      -wd4722 # Suppress 'function' : destructor never returns, potential memory leak
      -wd4800 # Suppress ''type' : forcing value to bool 'true' or 'false' (performance warning)'
      -wd4100 # Suppress 'unreferenced formal parameter'
      -wd4127 # Suppress 'conditional expression is constant'
      -wd4512 # Suppress 'assignment operator could not be generated'
      -wd4505 # Suppress 'unreferenced local function has been removed'
      -wd4610 # Suppress '<class> can never be instantiated'
      -wd4510 # Suppress 'default constructor could not be generated'
      -wd4702 # Suppress 'unreachable code'
      -wd4245 # Suppress 'signed/unsigned mismatch'
      -wd4706 # Suppress 'assignment within conditional expression'
      -wd4310 # Suppress 'cast truncates constant value'
      -wd4701 # Suppress 'potentially uninitialized local variable'
      -wd4703 # Suppress 'potentially uninitialized local pointer variable'
      -wd4389 # Suppress 'signed/unsigned mismatch'
      -wd4611 # Suppress 'interaction between '_setjmp' and C++ object destruction is non-portable'
      -wd4805 # Suppress 'unsafe mix of type <type> and type <type> in operation'
      -wd4204 # Suppress 'nonstandard extension used : non-constant aggregate initializer'
      -wd4577 # Suppress 'noexcept used with no exception handling mode specified; termination on exception is not guaranteed'
      -wd4091 # Suppress 'typedef: ignored on left of '' when no variable is declared'
      # C4592 is disabled because of false positives in Visual Studio 2015
      # Update 1. Re-evaluate the usefulness of this diagnostic with Update 2.
      -wd4592 # Suppress ''var': symbol will be dynamically initialized (implementation limitation)
      -wd4319 # Suppress ''operator' : zero extending 'type' to 'type' of greater size'

      # Ideally, we'd like this warning to be enabled, but MSVC 2013 doesn't
      # support the 'aligned' attribute in the way that clang sources requires (for
      # any code that uses the LLVM_ALIGNAS macro), so this is must be disabled to
      # avoid unwanted alignment warnings.
      # When we switch to requiring a version of MSVC that supports the 'alignas'
      # specifier (MSVC 2015?) this warning can be re-enabled.
      -wd4324 # Suppress 'structure was padded due to __declspec(align())'

      # Promoted warnings.
      -w14062 # Promote 'enumerator in switch of enum is not handled' to level 1 warning.

      # Promoted warnings to errors.
      -we4238 # Promote 'nonstandard extension used : class rvalue used as lvalue' to error.
      )
  endif (NOT CLANG_CL)
  foreach (flag ${msvc_warning_flags})
    append("${flag}" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  endforeach (flag)
endif (MSVC)

if (GCC_COMPATIBLE)
  # Don't add -Wall for clang-cl, because it maps -Wall to -Weverything for
  # MSVC compatibility.  /W4 is added above instead.
  if (NOT CLANG_CL)
    append("-Wall" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  endif ()

  append("-Wextra -Wno-unused-parameter -Wwrite-strings" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  append("-Wcast-qual" CMAKE_CXX_FLAGS)

  # Turn off missing field initializer warnings for gcc to avoid noise from
  # false positives with empty {}. Turn them on otherwise (they're off by
  # default for clang).
  check_cxx_compiler_flag("-Wmissing-field-initializers" CXX_SUPPORTS_MISSING_FIELD_INITIALIZERS_FLAG)
  if (CXX_SUPPORTS_MISSING_FIELD_INITIALIZERS_FLAG)
    if (CMAKE_COMPILER_IS_GNUCXX)
      append("-Wno-missing-field-initializers" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    else ()
      append("-Wmissing-field-initializers" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    endif ()
  endif ()

  # Disable gcc's potentially uninitialized use analysis as it presents lots of
  # false positives.
  if (CMAKE_COMPILER_IS_GNUCXX)
    check_cxx_compiler_flag("-Wmaybe-uninitialized" MAYBE_UNINITIALIZED_FLAG)
    append_if(MAYBE_UNINITIALIZED_FLAG "-Wno-maybe-uninitialized" CMAKE_CXX_FLAGS)
    if (NOT MAYBE_UNINITIALIZED_FLAG)
      # Only recent versions of gcc make the distinction between -Wuninitialized
      # and -Wmaybe-uninitialized. If -Wmaybe-uninitialized isn't supported, just
      # turn off all uninitialized use warnings.
      check_cxx_compiler_flag("-Wuninitialized" UNINITIALIZED_FLAG)
      append_if(UNINITIALIZED_FLAG "-Wno-uninitialized" CMAKE_CXX_FLAGS)
    endif ()

    # Suppress uninteresting warnings about initializing ArrayRef from initializer lists.
    check_cxx_compiler_flag("-Winit-list-lifetime" INIT_LIST_LIFETIME_FLAG)
    append_if(INIT_LIST_LIFETIME_FLAG "-Wno-init-list-lifetime" CMAKE_CXX_FLAGS)

    # Suppress the redundant move warnings in GCC 9, because it leads to suboptimal
    # recommendations for older compilers that do not implement C++ Core Issue 1579.
    check_cxx_compiler_flag("-Wredundant-move" REDUNDANT_MOVE_FLAG)
    append_if(REDUNDANT_MOVE_FLAG "-Wno-redundant-move" CMAKE_CXX_FLAGS)
  endif ()

  # This warning generates a lot of noise in gtest.
  check_cxx_compiler_flag("-Wdeprecated-copy" DEPRECATED_COPY_FLAG)
  append_if(DEPRECATED_COPY_FLAG "-Wno-deprecated-copy" CMAKE_CXX_FLAGS)

  # Disable -Wclass-memaccess, a C++-only warning from GCC 8 that fires on
  # LLVM's ADT classes.
  check_cxx_compiler_flag("-Wclass-memaccess" CXX_SUPPORTS_CLASS_MEMACCESS_FLAG)
  append_if(CXX_SUPPORTS_CLASS_MEMACCESS_FLAG "-Wno-class-memaccess" CMAKE_CXX_FLAGS)

  # The LLVM libraries have no stable C++ API, so -Wnoexcept-type is not useful.
  check_cxx_compiler_flag("-Wnoexcept-type" CXX_SUPPORTS_NOEXCEPT_TYPE_FLAG)
  append_if(CXX_SUPPORTS_NOEXCEPT_TYPE_FLAG "-Wno-noexcept-type" CMAKE_CXX_FLAGS)

  # Check if -Wnon-virtual-dtor warns even though the class is marked final.
  # If it does, don't add it. So it won't be added on clang 3.4 and older.
  # This also catches cases when -Wnon-virtual-dtor isn't supported by
  # the compiler at all.  This flag is not activated for gcc since it will
  # incorrectly identify a protected non-virtual base when there is a friend
  # declaration. Don't activate this in general on Windows as this warning has
  # too many false positives on COM-style classes, which are destroyed with
  # Release() (PR32286).
  if (NOT CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++17 -Werror=non-virtual-dtor")
    CHECK_CXX_SOURCE_COMPILES("class base {public: virtual void anchor();protected: ~base();};
                             class derived final : public base { public: ~derived();};
                             int main() { return 0; }"
      CXX_WONT_WARN_ON_FINAL_NONVIRTUALDTOR)
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    append_if(CXX_WONT_WARN_ON_FINAL_NONVIRTUALDTOR
      "-Wnon-virtual-dtor" CMAKE_CXX_FLAGS)
  endif ()

  # Enable -Wdelete-non-virtual-dtor if available.
  add_flag_if_supported("-Wdelete-non-virtual-dtor" DELETE_NON_VIRTUAL_DTOR_FLAG)

  # Avoid triggering arbitrary UB when converting doubles to ints.
  # TODO(T108716033) Evaluate adding this flag back in once a new clang is
  # released or XCode is fixed.
  # add_flag_if_supported("-fno-strict-float-cast-overflow" NO_STRICT_FLOAT_CAST_OVERFLOW_FLAG)

  # Disable range loop analysis warnings.
  check_cxx_compiler_flag("-Wrange-loop-analysis" RANGE_ANALYSIS_FLAG)
  append_if(RANGE_ANALYSIS_FLAG "-Wno-range-loop-analysis" CMAKE_CXX_FLAGS)
endif (GCC_COMPATIBLE)
