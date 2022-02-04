# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(HERMES_BUILD_NODE_HERMES)
  set(HERMES_ENABLE_EH ON)
  set(CXX_STANDARD 14)
  set(CXX_STANDARD_REQUIRED ON)

  set_source_files_properties(InternalBindings/tty_wrap.cpp InternalBindings/pipe_wrap.cpp PROPERTIES COMPILE_FLAGS -frtti)

  add_subdirectory(third-party/libuv)
  add_subdirectory(nodelib)
  add_hermes_tool(node-hermes
    node-hermes.cpp RuntimeState.cpp InternalBindings/buffer.cpp
    InternalBindings/constants.cpp InternalBindings/file.cpp
    InternalBindings/util.cpp InternalBindings/tty_wrap.cpp
    InternalBindings/pipe_wrap.cpp InternalBindings/stream_base.cpp
    ${ALL_HEADER_FILES}
    LINK_LIBS hermesapi hermesNodeBytecode uv_a
    )
  target_include_directories(node-hermes PRIVATE third-party/libuv/include/)
endif()
