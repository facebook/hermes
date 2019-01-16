#ifndef HERMES_SUPPORT_THREADLOCAL_H
#define HERMES_SUPPORT_THREADLOCAL_H

#include <hermes/Platform/Logging.h>

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

namespace hermes {

/// Wrapper calls for thread local using pthread API.
/// Mainly copied from fbandroid/libraries/profilo/deps/fb/ThreadLocal.h
template <typename T>
class ThreadLocal {
 public:
  /// Constructor that has to be called from a thread-neutral place.
  ThreadLocal() : key_(0) {
    initialize();
  }

  /// Accessing thread local slot pointer via overloaded operator.
  T *operator->() const {
    return get();
  }

  /// Accessing thread local slot pointer via method call.
  T *get() const {
    return (T *)pthread_getspecific(key_);
  }

  /// Release and \return thread local slot.
  T *release() {
    T *obj = get();
    pthread_setspecific(key_, nullptr);
    return obj;
  }

  /// Set thread local slot to \p other.
  void set(T *other = nullptr) {
    T *old = (T *)pthread_getspecific(key_);
    if (old != other) {
      pthread_setspecific(key_, other);
    }
  }

 private:
  void initialize() {
    int ret = pthread_key_create(&key_, nullptr);
    if (ret != 0) {
      const char *msg = "(unknown error)";
      switch (ret) {
        case EAGAIN:
          msg = "PTHREAD_KEYS_MAX (1024) is exceeded";
          break;
        case ENOMEM:
          msg = "Out-of-memory";
          break;
      }
      (void)msg;
      hermesLog("ThreadLocal", "pthread_key_create failed: %d %s", ret, msg);
      abort();
    }
  }

  pthread_key_t key_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_THREADLOCAL_H
