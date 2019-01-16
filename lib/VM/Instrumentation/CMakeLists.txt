add_llvm_library(hermesInstrumentation
  STATIC
  PerfEventsLinux.cpp
  ProcessStats.cpp
  StatSamplingThread.cpp
  PageAccessTracker.cpp
  LINK_LIBS hermesSupport
)
