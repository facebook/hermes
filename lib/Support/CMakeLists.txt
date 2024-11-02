# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(LINK_OBJLIBS
    dtoa
    hermesPlatformUnicode
    LLVHSupport)

if (ANDROID)
  set(LINK_LIBS log)
endif()

add_hermes_library(hermesSupport
        Allocator.cpp
        Base64.cpp
        Base64vlq.cpp
        BigIntSupport.cpp
        CheckedMalloc.cpp
        Conversions.cpp
        ErrorHandling.cpp
        JSONEmitter.cpp
        MD5.cpp
        OSCompatEmscripten.cpp
        OSCompatPosix.cpp
        OSCompatWindows.cpp
        PageAccessTrackerPosix.cpp
        PerfSection.cpp
        SemaphorePosix.cpp
        SerialExecutor.cpp
        SHA1.cpp
        SNPrintfBuf.cpp
        SourceErrorManager.cpp
        SimpleDiagHandler.cpp
        StringTable.cpp
        UTF8.cpp
        UTF16Stream.cpp
        LEB128.cpp
        MemorySizeParser.cpp
        RandomSeedParser.cpp
        LINK_OBJLIBS ${LINK_OBJLIBS}
        LINK_LIBS ${LINK_LIBS}
)
