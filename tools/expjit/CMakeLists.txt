# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

enable_language(ASM)

add_hermes_tool(idisp
        idisp-exp.c loop1-jit.h
        idisp-jit.cpp FixedLRU.h JitEmitter.h
        LINK_LIBS shermes_console_a asmjit
)
