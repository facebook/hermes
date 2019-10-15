# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(link_libs
    dtoa
    hermesRegex
    hermesPlatformUnicode)

if (ANDROID)
  list(APPEND link_libs log)
endif()

add_llvm_library(hermesSupport
        Allocator.cpp
        Base64vlq.cpp
        CheckedMalloc.cpp
        CompactArray.cpp
        Conversions.cpp
        ErrorHandling.cpp
        JSONEmitter.cpp
        OSCompatPosix.cpp
        OSCompatWindows.cpp
        PageAccessTrackerPosix.cpp
        PerfSection.cpp
        RegExpSerialization.cpp
        Semaphore.cpp
        SHA1.cpp
        SNPrintfBuf.cpp
        SourceErrorManager.cpp
        SimpleDiagHandler.cpp
        StringKind.cpp
        StringTable.cpp
        UTF8.cpp
        LEB128.cpp
        LINK_LIBS ${link_libs}
)

hermes_link_icu(hermesSupport)
