#include <mutex>

// dtoa requires two locks to be provided for safe operation in a
// multi-theaded environment.

static std::mutex mutex0;
static std::mutex mutex1;

extern "C" void ACQUIRE_DTOA_LOCK(int n) { !n ? mutex0.lock() : mutex1.lock(); }

extern "C" void FREE_DTOA_LOCK(int n) {
  !n ? mutex0.unlock() : mutex1.unlock();
}
