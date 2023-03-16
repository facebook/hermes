/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SEMAPHORE_H
#define HERMES_SUPPORT_SEMAPHORE_H

#ifdef _WINDOWS
#error "Semaphore is not implemented because no code uses it in Windows"
#else

#include <semaphore.h>

namespace hermes {

/// Encapsulates POSIX semaphore support across platform.
/// Apple does not support unamed semaphore so we will use named semaphore.
/// Android does not support named semaphore so use unamed semaphore instead.
class Semaphore {
 private:
#ifdef __APPLE__
  sem_t *semPtr_{nullptr};
#else
  sem_t sem_;
#endif

 private:
  /// \return the raw semaphore pointer.
  sem_t *getSemaphorePtr();

 public:
  /// Initialize an unnamed OS semaphore.
  /// \p semaphorePrefix is used to create a temporary named semaphore if
  /// unnamed semaphores are not supported on target platform. It must be a
  /// valid semaphore name (see
  /// https://man7.org/linux/man-pages/man7/sem_overview.7.html), which will be
  /// suffixed with the thread ID to ensure the created semaphore is not
  /// accessible from other any other thread.
  bool open(const char *semaphorePrefix);

  /// Destory the underlying OS semaphore.
  bool close();

  /// Notify one thread blocked on this semaphore to wait up.
  bool notifyOne();

  /// Wait to be notified.
  bool wait();
};

} // namespace hermes

#endif // _WINDOWS

#endif // HERMES_SUPPORT_SEMAPHORE_H
