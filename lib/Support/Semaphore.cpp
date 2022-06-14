/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// The only user of Semaphore.h is SamplingProfiler, which is stubbed out
// in Windows. As a result, it is unnecessary to implement Semaphore on Windows.
#ifndef _WINDOWS

#include "hermes/Support/Semaphore.h"

#include <cstdio>
#include <string>

namespace hermes {

sem_t *Semaphore::getSemaphorePtr() {
#ifdef __APPLE__
  return semPtr_;
#else
  return &sem_;
#endif
}

bool Semaphore::open(const char *semaphoreName) {
#ifdef __APPLE__
  // Create named semaphore with read/write.
  semPtr_ = sem_open(semaphoreName, O_CREAT, S_IRUSR | S_IWUSR, /*value*/ 0);
  if (semPtr_ == SEM_FAILED) {
    perror("sem_open");
    return false;
  }
  assert(semPtr_ != nullptr && "sem_open should have succeeded");
  if (sem_unlink(semaphoreName) != 0) {
    perror("sem_unlink");
    return false;
  }
#else
  auto ret = sem_init(&sem_, /*pshared*/ 0, /*value*/ 0);
  if (ret != 0) {
    perror("sem_init");
    return false;
  }
#endif
  return true;
}

bool Semaphore::close() {
#ifdef __APPLE__
  if (sem_close(semPtr_) != 0) {
    perror("sem_close");
    return false;
  }
#else
  if (sem_destroy(&sem_) != 0) {
    perror("sem_destroy");
    return false;
  }
#endif
  return true;
}

bool Semaphore::notifyOne() {
  int ret = sem_post(getSemaphorePtr());
  if (ret != 0) {
    perror("sem_post");
    return false;
  }
  return true;
}

bool Semaphore::wait() {
  int ret = sem_wait(getSemaphorePtr());
  if (ret != 0) {
    perror("sem_wait");
    return false;
  }
  return true;
}

} // namespace hermes

#endif // not _WINDOWS
