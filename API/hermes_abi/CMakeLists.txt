# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_VM_DEP hermesVMRuntime)
if(HERMES_BUILD_LEAN_LIBHERMES)
  set(HERMES_VM_DEP hermesVMRuntimeLean)
endif()

add_library(hermesabi SHARED hermes_abi.cpp)
target_link_libraries(hermesabi PRIVATE ${HERMES_VM_DEP})
target_include_directories(hermesabi PUBLIC ..)

# Disable exceptions if they are not required. While the ABI can never throw an
# exception, it may need exception handling to catch exceptions thrown in the
# VM.
if(NOT HERMES_ENABLE_EH)
  if(GCC_COMPATIBLE)
    target_compile_options(hermesabi PRIVATE -fno-rtti -fno-exceptions)
  elseif (MSVC)
    target_compile_options(hermesabi PRIVATE /GR- /EHs-c-)
  endif()
endif()

add_library(hermesAbiRuntime STATIC HermesABIRuntime.cpp)
target_link_libraries(hermesAbiRuntime PUBLIC hermesabi jsi hermesPublic)
