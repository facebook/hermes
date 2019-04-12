set(link_libs
    dtoa
    hermesRegex
    hermesPlatformUnicode)

if (ANDROID)
  list(APPEND link_libs log)
endif()

add_llvm_library(hermesSupport
        Allocator.cpp
        ASAN.cpp
        CheckedMalloc.cpp
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
        StringTable.cpp
        UTF8.cpp
        LEB128.cpp
        LINK_LIBS ${link_libs}
)

hermes_link_icu(hermesSupport)
