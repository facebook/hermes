# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Define libraries that provides Hermes wrapped in the C-API.
add_library(hermesabi hermes_vtable.cpp c_compat.c)
target_link_libraries(hermesabi PRIVATE hermesVMRuntime)
target_include_directories(hermesabi PUBLIC ..)
target_link_options(hermesabi PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

add_library(hermesabi_lean hermes_vtable.cpp c_compat.c)
target_link_libraries(hermesabi_lean PRIVATE hermesVMRuntimeLean)
target_include_directories(hermesabi_lean PUBLIC ..)
target_link_options(hermesabi_lean PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

# TODO: Handle building with EXCEPTION_ON_OOM.
if(HERMES_ENABLE_EH_RTTI)
  message(WARNING "Using the ABI with exceptions enabled is not supported.")
elseif (GCC_COMPATIBLE)
  target_compile_options(hermesabi PRIVATE -fno-rtti -fno-exceptions)
  target_compile_options(hermesabi_lean PRIVATE -fno-rtti -fno-exceptions)
elseif (MSVC)
  target_compile_options(hermesabi PRIVATE /GR- /EHs-c-)
  target_compile_options(hermesabi_lean PRIVATE /GR- /EHs-c-)
endif()

# Define the library for the C++ wrapper that wraps our C-API to provide a JSI
# interface. Note that this does not take a dependency on hermesabi, an
# integrator will instantiate the wrapper with an instance of the C-API.
add_library(hermesABIRuntimeWrapper STATIC HermesABIRuntimeWrapper.cpp)
target_include_directories(hermesABIRuntimeWrapper PUBLIC ..)
target_link_libraries(hermesABIRuntimeWrapper PUBLIC jsi hermesPublic)
