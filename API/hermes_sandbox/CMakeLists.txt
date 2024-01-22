# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if (EMSCRIPTEN)
  set(WASM_LINKER_ARGS "-sEXPORTED_FUNCTIONS=[_get_hermes_abi_vtable, _malloc, _realloc, _free, stackSave, stackRestore, stackAlloc]" -sSTANDALONE_WASM --no-entry -sALLOW_TABLE_GROWTH=1)

  # Define a target that builds the Hermes library as a wasm executable.
  add_executable(hermesSandboxImpl hermes_sandbox_impl.cpp )
  target_link_libraries(hermesSandboxImpl PRIVATE hermesabi)
  target_link_options(hermesSandboxImpl PRIVATE ${WASM_LINKER_ARGS})
endif()

add_library(hermesSandboxRuntime HermesSandboxRuntime.cpp external/wasm-rt-impl.c external/hermes_sandbox_impl_compiled.c)
target_include_directories(hermesSandboxRuntime PUBLIC .)
target_link_libraries(hermesSandboxRuntime PUBLIC jsi hermesPublic)

# Disable installing signal handlers to avoid having any globally visible effects.
# When signal handlers are disabled, the runtime will fall back to stack depth
# counting to detect stack overflows. Disable that to avoid imposing an
# arbitrary limit on the stack size.
target_compile_definitions(hermesSandboxRuntime PUBLIC WASM_RT_SKIP_SIGNAL_RECOVERY=1 WASM_RT_USE_STACK_DEPTH_COUNT=0)

# Disable warnings in external/generated code.
file(GLOB EXTERNAL_FILES external/*.c)
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set_property(SOURCE ${EXTERNAL_FILES} APPEND_STRING
    PROPERTY COMPILE_FLAGS "-w")
endif()
