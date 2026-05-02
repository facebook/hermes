# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_tool(napi-runner
  napi-runner.cpp
  LINK_LIBS hermesNapiCompile hermesNapi hermesvm_a)
