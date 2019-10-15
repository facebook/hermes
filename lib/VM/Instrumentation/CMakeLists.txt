# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_llvm_library(hermesInstrumentation
  STATIC
  PerfEventsLinux.cpp
  ProcessStats.cpp
  StatSamplingThread.cpp
  LINK_LIBS hermesSupport
)
