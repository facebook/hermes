# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if (EMSCRIPTEN)
  add_hermes_tool(hermes-parser-wasm
    hermes-parser-wasm.cpp
    HermesParserJSSerializer.cpp
    HermesParserDiagHandler.cpp
    ${ALL_HEADER_FILES}
    )

  target_link_libraries(hermes-parser-wasm
    hermesAST
    hermesParser
    hermesSema
    LLVHSupport
  )

  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s NODERAWFS=0 -s WASM=1")
  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s ALLOW_MEMORY_GROWTH=1")
  target_link_options(hermes-parser-wasm PRIVATE
    "SHELL: -s EXPORTED_RUNTIME_METHODS=[cwrap,ccall,HEAP8,HEAPU8,HEAP16,HEAPU16,HEAP32,HEAPU32,HEAPF32,HEAP64,HEAPU64,HEAPF64]")
  target_link_options(hermes-parser-wasm PRIVATE
    "SHELL: -s EXPORTED_FUNCTIONS=[_malloc,_free]")
  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s MODULARIZE=1")
  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s EXPORT_NAME=hermes_parser_wasm")
  target_link_options(hermes-parser-wasm PRIVATE
    "SHELL: -s BINARYEN_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 -s WASM_ASYNC_COMPILATION=0")
  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s NODEJS_CATCH_EXIT=0")
  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s NODEJS_CATCH_REJECTION=0")
  target_link_options(hermes-parser-wasm PRIVATE "SHELL: -s STACK_SIZE=5MB")
endif ()
