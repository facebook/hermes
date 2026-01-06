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

if (HERMES_USE_BOOST_CONTEXT)
  include_directories(${PROJECT_SOURCE_DIR}/external/boost/boost_1_86_0)
endif()

include_directories(${PROJECT_SOURCE_DIR}/external/fast_float/)

add_hermes_library(hermesSupport
        Allocator.cpp
        Base64.cpp
        Base64vlq.cpp
        BigIntSupport.cpp
        CheckedMalloc.cpp
        Conversions.cpp
        ErrorHandling.cpp
        FastStrToDouble.cpp
        JSONEmitter.cpp
        MD5.cpp
        OSCompatCommon.cpp
        OSCompatEmscripten.cpp
        OSCompatWasi.cpp
        OSCompatPosix.cpp
        OSCompatWindows.cpp
        PageAccessTrackerPosix.cpp
        PerfSection.cpp
        SemaphorePosix.cpp
        SerialExecutor.cpp
        StackExecutor.cpp
        StackOverflowGuard.cpp
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
        LINK_OBJLIBS ${LINK_OBJLIBS} ${BOOST_CONTEXT_LIB}
        LINK_LIBS ${LINK_LIBS}
)
