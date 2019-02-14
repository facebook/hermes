add_llvm_library(hermesInstrumentation
  STATIC
  PerfEventsLinux.cpp
  ProcessStats.cpp
  StatSamplingThread.cpp
  PageAccessTrackerPosix.cpp
  LINK_LIBS hermesSupport
)
