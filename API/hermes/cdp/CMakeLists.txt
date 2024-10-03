# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Make a library to enable unit testing
add_hermes_library(cdpInternal ConsoleMessage.cpp DomainState.cpp LINK_LIBS jsi hermesapi)
