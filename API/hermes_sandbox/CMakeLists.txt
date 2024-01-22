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
