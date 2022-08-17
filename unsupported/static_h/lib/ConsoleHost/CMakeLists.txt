# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  ConsoleHost.cpp
  MemorySizeParser.cpp
  RandomSeedParser.cpp
)

add_hermes_library(hermesConsoleHost STATIC ${source_files}
  LINK_LIBS hermesVMRuntime hermesInstrumentation
)
