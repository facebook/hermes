# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

enable_language(ASM)

add_hermes_tool(loop1
        loop1-exp.c
        loop1-bench.s
        loop1-jit.cpp loop1-jit.h
        LINK_LIBS shermes_console_a asmjit
)
add_hermes_tool(idisp
        idisp-exp.c
        idisp-jit.cpp
        LINK_LIBS shermes_console_a asmjit
)
add_hermes_tool(idispn
        idisp-exp.c
        idispn-jit.cpp
        LINK_LIBS shermes_console_a asmjit
)
add_hermes_tool(idispnr
        idisp-exp.c
        idispnr-jit.cpp
        LINK_LIBS shermes_console_a asmjit
)
