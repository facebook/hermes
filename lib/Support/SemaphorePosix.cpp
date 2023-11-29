/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _WINDOWS

#include "hermes/Support/Semaphore.h"

#include "hermes/Support/OSCompat.h"

#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/raw_ostream.h"

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

bool Semaphore::open(const char *semaphorePrefix) {
#ifdef __APPLE__
  // Generate a unique name for the semaphore by postfixing semaphorePrefix with
  // the thread ID. This ensures that no other semaphore will be created with
  // the same name, avoiding unintentional sharing.
  llvh::SmallVector<char, 64> semaphoreName;
  llvh::raw_svector_ostream OS(semaphoreName);

  // oscompat::global_thread_id returns the OS level thread ID, and is thus
  // system unique.
  OS << semaphorePrefix << oscompat::global_thread_id() << '\0';

  // Create a named semaphore with read/write. Use O_EXCL as an extra protection
  // layer -- sem_open will fail if semaphoreName is not unique.
  semPtr_ = sem_open(
      semaphoreName.data(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, /*value*/ 0);
  if (semPtr_ == SEM_FAILED) {
    perror("sem_open");
    return false;
  }
  assert(semPtr_ != nullptr && "sem_open should have succeeded");

  // Now unlink the semaphore from its temporary name. This ensures that no
  // other Semaphore will share the underlying OS object.
  if (sem_unlink(semaphoreName.data()) != 0) {
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
