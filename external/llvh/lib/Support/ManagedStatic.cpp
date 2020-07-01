//===-- ManagedStatic.cpp - Static Global wrapper -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the ManagedStatic class and llvm_shutdown().
//
//===----------------------------------------------------------------------===//

#include "llvh/Support/ManagedStatic.h"
#include "llvh/Config/config.h"
#include "llvh/Support/Mutex.h"
#include "llvh/Support/MutexGuard.h"
#include "llvh/Support/Threading.h"
#include <cassert>
using namespace llvh;

static const ManagedStaticBase *StaticList = nullptr;
static sys::Mutex *ManagedStaticMutex = nullptr;
static llvh::once_flag mutex_init_flag;

static void initializeMutex() {
  ManagedStaticMutex = new sys::Mutex();
}

static sys::Mutex* getManagedStaticMutex() {
  llvh::call_once(mutex_init_flag, initializeMutex);
  return ManagedStaticMutex;
}

void ManagedStaticBase::RegisterManagedStatic(void *(*Creator)(),
                                              void (*Deleter)(void*)) const {
  assert(Creator);
  if (llvm_is_multithreaded()) {
    MutexGuard Lock(*getManagedStaticMutex());

    if (!Ptr.load(std::memory_order_relaxed)) {
      void *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = this;
    }
  } else {
    assert(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = this;
  }
}

void ManagedStaticBase::destroy() const {
  assert(DeleterFn && "ManagedStatic not initialized correctly!");
  assert(StaticList == this &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = Next;
  Next = nullptr;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = nullptr;
  DeleterFn = nullptr;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
void llvh::llvm_shutdown() {
  MutexGuard Lock(*getManagedStaticMutex());

  while (StaticList)
    StaticList->destroy();
}
