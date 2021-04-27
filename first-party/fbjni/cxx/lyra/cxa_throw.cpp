/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <atomic>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <cassert>
#ifndef _WIN32
#include <unwind.h>
#include <cxxabi.h>
#endif

#include <lyra/lyra_exceptions.h>

namespace facebook {
namespace lyra {

using namespace detail;

namespace {
std::atomic<bool> enableBacktraces{true};

const ExceptionTraceHolder* getExceptionTraceHolderInException(
    std::exception_ptr ptr) {
  try {
    std::rethrow_exception(ptr);
  } catch (const ExceptionTraceHolder& holder) {
    return &holder;
  } catch (...) {
    return nullptr;
  }
}
} // namespace

void enableCxaThrowHookBacktraces(bool enable) {
  enableBacktraces.store(enable, std::memory_order_relaxed);
}

[[gnu::noreturn]] void (*original_cxa_throw)(void*, const std::type_info*, void (*) (void *));

#if defined(_LIBCPP_VERSION)

// We want to attach stack traces to C++ exceptions. Our API contract is that
// calling lyra::getExceptionTrace on the exception_ptr for an exception should
// return the stack trace for that exception.
//
// We accomplish this by providing a hook for __cxa_throw, which creates an
// ExceptionTraceHolder object (which captures the stack trace for the
// exception), and creates a mapping from the pointer to the exception object
// (which is a parameter to __cxa_throw) to its ExceptionTraceHolder object.
// This mapping can then be queried by lyra::getExceptionTrace to get the stack
// trace for the exception. We have a custom exception destructor to destroy the
// trace object and call the original destructor for the exception object, and
// our __cxa_throw hook calls the original __cxa_throw to perform the actual
// exception throwing and passes this custom destructor.
//
// This works because __cxa_throw is only called when creating a new exception
// object (that has been freshly allocated via __cxa_allocate_exception), so at
// that point, we're able to capture the original stack trace for the exception.
// Even if that exception is later rethrown, we'll still maintain its original
// stack trace, assuming that std::current_exception creates a reference to the
// current exception instead of copying it (which is true for both libstdc++ and
// libc++), such that we can still look up the exception object via pointer.
//
// We don't have to worry about any pointer adjustments for the exception object
// (e.g. for converting to or from a base class subobject pointer), because a
// std::exception_ptr will always capture the pointer to the exception object
// itself and not any subobjects.
//
// Our map must be global, since exceptions can be transferred across threads.
// Consequently, we must use a mutex to guard all map operations.

typedef void (*destructor_type)(void*);

namespace {
struct ExceptionState {
  ExceptionTraceHolder trace;
  destructor_type destructor;
};

// We create our map and mutex as function statics and leak them intentionally,
// to ensure they've been initialized before any global constructors and are
// also available to use inside any global destructors.
std::unordered_map<void*, ExceptionState>* get_exception_state_map() {
  static auto* exception_state_map =
      new std::unordered_map<void*, ExceptionState>();
  return exception_state_map;
}

std::mutex* get_exception_state_map_mutex() {
  static auto* exception_state_map_mutex = new std::mutex();
  return exception_state_map_mutex;
}

void trace_destructor(void* exception_obj) {
  destructor_type original_destructor = nullptr;

  {
    std::lock_guard<std::mutex> lock(*get_exception_state_map_mutex());
    auto* exception_state_map = get_exception_state_map();
    auto it = exception_state_map->find(exception_obj);
    if (it == exception_state_map->end()) {
      // This really shouldn't happen, but if it does, just leaking the trace
      // and exception object seems better than crashing.
      return;
    }

    original_destructor = it->second.destructor;
    exception_state_map->erase(it);
  }

  if (original_destructor) {
    original_destructor(exception_obj);
  }
}
} // namespace

[[noreturn]] void
cxa_throw(void* obj, const std::type_info* type, destructor_type destructor) {
  // TODO(T61689492): Re-enable Lyra stack traces for libc++ arm64
#if !defined(__aarch64__)
  if (enableBacktraces.load(std::memory_order_relaxed)) {
    std::lock_guard<std::mutex> lock(*get_exception_state_map_mutex());
    get_exception_state_map()->emplace(
        obj, ExceptionState{ExceptionTraceHolder(), destructor});
  }
#endif

  original_cxa_throw(obj, type, trace_destructor);
}

const ExceptionTraceHolder* detail::getExceptionTraceHolder(
    std::exception_ptr ptr) {
  {
    std::lock_guard<std::mutex> lock(*get_exception_state_map_mutex());
    // The exception object pointer isn't a public member of std::exception_ptr,
    // and there isn't any public method to get it. However, for both libstdc++
    // and libc++, it's the first pointer inside the exception_ptr, and we can
    // rely on the ABI of those libraries to remain stable, so we can just
    // access it directly.
    void* exception_obj = *reinterpret_cast<void**>(&ptr);
    auto* exception_state_map = get_exception_state_map();
    auto it = exception_state_map->find(exception_obj);
    if (it != exception_state_map->end()) {
      return &it->second.trace;
    }
  }

  // Fall back to attempting to retrieve the ExceptionTraceHolder directly from
  // the exception (to support e.g. fbthrow).
  return getExceptionTraceHolderInException(ptr);
}
#elif !defined(_WIN32)

namespace {

const auto traceHolderType =
  static_cast<const abi::__class_type_info*>(&typeid(ExceptionTraceHolder));

// lyra's __cxa_throw attaches stack trace information to thrown exceptions. It basically does:
//   1. capture stack trace
//   2. construct a new type_info struct that:
//     a. holds the ExceptionTraceHolder
//     b. supports upcasting to lyra::ExceptionTraceHolder* (by just returning the holder member)
//     c. acts like the original exception type_info otherwise
//   3. call original __cxa_throw() with original exception pointer, the
//      HijackedExceptionTypeInfo, and HijackedExceptionTypeInfo::destructor
//      (which will both delete the constructed type info and call the original
//      destructor).
struct HijackedExceptionTypeInfo : public abi::__class_type_info {
  HijackedExceptionTypeInfo(void* obj, const std::type_info* base, void(*destructor)(void*))
      : abi::__class_type_info{base->name()}, base_{base}, orig_dest_{destructor} {
  }

  bool __is_pointer_p() const override {
    return base_->__is_pointer_p();
  }

  bool __is_function_p() const override {
    return base_->__is_function_p();
  }

  bool __do_catch(const type_info *__thr_type, void **__thr_obj, unsigned __outer) const override {
    return base_->__do_catch(__thr_type, __thr_obj, __outer);
  }

  bool __do_upcast(const abi::__class_type_info *__target, void **__obj_ptr) const override {
    if (__target == traceHolderType) {
      *__obj_ptr = (void*)&stack_;
      return true;
    }
    return base_->__do_upcast(__target, __obj_ptr);
  }

  static void destructor(void* obj) {
    auto exc_ptr = reinterpret_cast<std::exception_ptr*>(&obj);
    auto info = reinterpret_cast<const::std::type_info*>(exc_ptr->__cxa_exception_type());
    auto mutable_info = static_cast<HijackedExceptionTypeInfo*>(const_cast<std::type_info*>(info));
    if (mutable_info->orig_dest_) {
      mutable_info->orig_dest_(obj);
    }
    delete mutable_info;
  }

 private:
  const std::type_info* base_;
  void (*orig_dest_)(void*);
  ExceptionTraceHolder stack_;
};

} // namespace

[[noreturn]] void cxa_throw(void* obj, const std::type_info* type, void (*destructor) (void *)) {
  if (enableBacktraces.load(std::memory_order_relaxed)) {
    if (!type->__do_upcast(traceHolderType, &obj)) {
      type = new HijackedExceptionTypeInfo(obj, type, destructor);
      destructor = HijackedExceptionTypeInfo::destructor;
    }
  }
  original_cxa_throw(obj, type, destructor);
}

const ExceptionTraceHolder* detail::getExceptionTraceHolder(
    std::exception_ptr ptr) {
  return getExceptionTraceHolderInException(ptr);
}

#endif // libc++/Windows

} // namespace lyra
} // namespace facebook
