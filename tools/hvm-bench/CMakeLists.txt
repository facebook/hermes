# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_tool(interp-dispatch-bench
  interp-dispatch-bench.cpp
  ${ALL_HEADER_FILES}
  LINK_LIBS hermesvm_a
  )
