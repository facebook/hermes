# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(emhermesc
  emhermesc.cpp
  ${ALL_HEADER_FILES}
  LINK_LIBS hermesHBCBackend
  )

if (EMSCRIPTEN)
  target_link_options(emhermesc PRIVATE "SHELL: -s NODERAWFS=0 -s WASM=1")
  target_link_options(emhermesc PRIVATE "SHELL: -s ALLOW_MEMORY_GROWTH=1")
  target_link_options(emhermesc PRIVATE
    "SHELL: -s EXTRA_EXPORTED_RUNTIME_METHODS=[cwrap,ccall]")
  target_link_options(emhermesc PRIVATE "SHELL: -s MODULARIZE=1")
  target_link_options(emhermesc PRIVATE "SHELL: -s EXPORT_NAME=createHermesc")
  target_link_options(emhermesc PRIVATE
    "SHELL: -s BINARYEN_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 -s WASM_ASYNC_COMPILATION=0")
  target_link_options(emhermesc PRIVATE
    "SHELL: -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0")
endif ()
