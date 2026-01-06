#include "llvh/Config/llvm-config.h"

#if LLVM_ENABLE_THREADS
#include <cassert>
#include <mutex>

// Our modified dtoa requires one locks (lock 1) to be provided for maintaining
// its power of 5 cache in a multi-threaded environment.

static std::mutex mutex;

extern "C" void ACQUIRE_DTOA_LOCK(int n) {
  assert(n == 1 && "only dtoa lock 1 is supported");
  mutex.lock();
}

extern "C" void FREE_DTOA_LOCK(int n) {
  assert(n == 1 && "only dtoa lock 1 is supported");
  mutex.unlock();
}
#else
extern "C" void ACQUIRE_DTOA_LOCK(int) {
}

extern "C" void FREE_DTOA_LOCK(int) {
}
#endif
