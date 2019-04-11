add_llvm_library(hermesInstrumentation
  STATIC
  PerfEventsLinux.cpp
  ProcessStats.cpp
  StatSamplingThread.cpp
  LINK_LIBS hermesSupport
)
