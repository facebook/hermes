# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_tool(hvm
  hvm.cpp
  LINK_OBJLIBS hermesConsoleHost_obj
  LINK_LIBS hermesvm_a)

install(TARGETS hvm
  RUNTIME DESTINATION bin
)
