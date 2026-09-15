/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi_impl.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/HandleRootOwner.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

using namespace hermes::vm;

//===========================================================================
// napi_threadsafe_function__ definition
//===========================================================================

/// A thread-safe function that can be called from any thread.
///
/// Data items are pushed onto an internal queue (thread-safe via mutex).
/// The event loop is notified to dispatch the items on the main thread,
/// where the user's \c call_js_cb is invoked for each item with the
/// JS function and context.
///
/// Lifecycle:
/// 1. Created via napi_create_threadsafe_function with an initial
///    thread count.
/// 2. Additional threads acquire via napi_acquire_threadsafe_function.
/// 3. Threads call napi_call_threadsafe_function to push data.
/// 4. Threads release via napi_release_threadsafe_function.
/// 5. When all threads have released and the queue is empty (or abort
///    mode is used), the TSFN is finalized and deleted.
///
/// The JS function value is a GC root, tracked via the env's TSFN
/// linked list.
struct napi_threadsafe_function__ {
  napi_env env;

  /// Mutex protecting queue, thread_count, and is_closing.
  std::mutex mutex;

  /// Condition variable for blocking mode (waits when queue is full)
  /// and for signaling closing state.
  std::condition_variable cond;

  /// Queue of user data pointers pushed by calling threads.
  std::queue<void *> queue;

  /// Maximum queue size. 0 means unlimited.
  size_t max_queue_size;

  /// Number of threads that have acquired this TSFN.
  size_t thread_count;

  /// Whether the TSFN is closing (no more calls accepted).
  bool is_closing = false;

  /// The user's callback invoked on the main thread for each queued
  /// item. If null at creation, a default implementation is used that
  /// calls the JS function with no arguments.
  napi_threadsafe_function_call_js call_js_cb;

  /// Opaque context pointer returned by
  /// napi_get_threadsafe_function_context.
  void *context;

  /// Pointer to the finalize data and callback, called when the TSFN
  /// is destroyed.
  void *finalize_data;
  napi_finalize finalize_cb;

  /// The JavaScript function to call. This is a GC root, marked via
  /// the env's TSFN list. Set to undefined if func was null at
  /// creation (call_js_cb-only mode).
  PinnedHermesValue js_func{};

  /// Whether the TSFN is "referenced" (prevents event loop exit).
  /// When true and the host provides register_loop_source, this tsfn
  /// contributes one to the env's tsfn loop ref count, which in turn
  /// holds a single host loop source. Tracked as an API state even
  /// when the host has no loop-source callbacks.
  bool is_ref = true;

  /// Whether this tsfn currently holds an env loop ref. True iff
  /// `is_ref` was true at create / ref time and the host provides
  /// loop-source callbacks. Used to know whether finalize / unref
  /// should release.
  bool loop_refed = false;

  /// Whether a dispatch is in flight (queued in the host loop, running,
  /// or both within the same handoff window). Set to true by the
  /// producer-side CAS that posts a task; cleared only by tsfnDispatch
  /// when it has nothing left to do. This invariant — at most one
  /// dispatch live at a time — is what makes inline finalize+delete
  /// safe: tsfnDispatch knows no copy of itself is queued behind it.
  std::atomic<bool> dispatch_pending{false};

  /// Doubly-linked list pointers for the env's TSFN list.
  napi_threadsafe_function__ *prev_ = nullptr;
  napi_threadsafe_function__ *next_ = nullptr;
};

//===========================================================================
// Default call_js_cb
//===========================================================================

/// Default JS callback: calls the JS function with undefined as
/// receiver and no arguments. Matches Node.js behavior.
static void defaultCallJsCb(
    napi_env env,
    napi_value js_callback,
    void *context,
    void *data) {
  (void)context;
  (void)data;
  if (env == nullptr || js_callback == nullptr) {
    return;
  }

  napi_value recv;
  napi_status status = napi_get_undefined(env, &recv);
  if (status != napi_ok) {
    return;
  }

  napi_call_function(env, recv, js_callback, 0, nullptr, nullptr);
}

//===========================================================================
// Dispatch on main thread
//===========================================================================

/// Maximum items dispatched per event loop iteration to prevent
/// starvation of other event loop tasks.
static constexpr unsigned kMaxDispatchCount = 1000;

/// Called on the main thread by the event loop to process queued items.
static void tsfnDispatch(void *tsfn_data) {
  auto *tsfn = static_cast<napi_threadsafe_function__ *>(tsfn_data);
  napi_env env = tsfn->env;

  // dispatch_pending stays true for the whole call. Clearing it here
  // would let a producer push CAS true and post a second dispatch
  // while we are still draining — and if we then drained the last
  // item and thread_count==0 we would delete tsfn out from under that
  // queued copy. The tail of this function clears it only when we are
  // sure nothing more is queued behind us.

  unsigned iterations = kMaxDispatchCount;
  bool has_more = true;

  while (has_more && iterations-- > 0) {
    void *data = nullptr;
    bool popped = false;
    bool should_finalize = false;

    {
      std::lock_guard<std::mutex> lock(tsfn->mutex);

      if (tsfn->is_closing) {
        // Drain remaining items with env==nullptr to signal closing.
        while (!tsfn->queue.empty()) {
          void *item = tsfn->queue.front();
          tsfn->queue.pop();
          tsfn->call_js_cb(nullptr, nullptr, tsfn->context, item);
        }
        should_finalize = true;
        has_more = false;
      } else {
        size_t size = tsfn->queue.size();
        if (size > 0) {
          data = tsfn->queue.front();
          tsfn->queue.pop();
          popped = true;
          // If the queue was full and we just freed a slot, wake any
          // blocked Push() callers.
          if (size == tsfn->max_queue_size && tsfn->max_queue_size > 0) {
            tsfn->cond.notify_one();
          }
          size--;
        }

        if (size == 0 && tsfn->thread_count == 0) {
          // All threads released and queue empty — close.
          tsfn->is_closing = true;
          if (tsfn->max_queue_size > 0) {
            tsfn->cond.notify_all();
          }
          should_finalize = true;
          has_more = false;
        } else {
          has_more = (size > 0);
        }
      }
    }

    if (popped) {
      // Open a handle scope for each dispatch.
      napi_handle_scope scope = nullptr;
      napi_open_handle_scope(env, &scope);

      napi_value js_callback = nullptr;
      if (!tsfn->js_func.isUndefined()) {
        js_callback = env->addToCurrentScope(tsfn->js_func);
      }

      tsfn->call_js_cb(env, js_callback, tsfn->context, data);

      napi_close_handle_scope(env, scope);
    }

    if (should_finalize) {
      // Call finalize callback if set.
      if (tsfn->finalize_cb) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        tsfn->finalize_cb(env, tsfn->finalize_data, tsfn->context);
        napi_close_handle_scope(env, scope);
      }

      // Release the env-level loop ref, if this tsfn was holding one.
      // The env unregisters its host source on the 1→0 transition; the
      // loop may then exit if no other sources or tasks remain.
      if (tsfn->loop_refed) {
        env->releaseTsfnLoopRef();
        tsfn->loop_refed = false;
      }

      // Remove from env's TSFN list and delete.
      env->removeTsfn(tsfn);
      delete tsfn;
      return;
    }
  }

  // Loop exited without finalizing. Decide whether to re-post or to
  // release the dispatch_pending flag.
  if (has_more) {
    // Iterations exhausted but queue still has items. dispatch_pending
    // is already true; just re-post so we keep the single-dispatch
    // invariant intact.
    env->host_->post_task(env->host_->data, tsfn, tsfnDispatch);
    return;
  }

  // Queue was empty on our last check. Reacquire the mutex and re-check
  // under the lock: a producer push or release that lost the race to
  // post (because dispatch_pending was true) might have left state we
  // must drain or finalize before clearing the flag.
  std::lock_guard<std::mutex> lock(tsfn->mutex);
  if (tsfn->queue.empty() && tsfn->thread_count > 0 && !tsfn->is_closing) {
    // Genuinely idle: drop the flag. The next producer push will see
    // false, CAS to true under our mutex's release/acquire pairing,
    // and post a fresh dispatch.
    tsfn->dispatch_pending.store(false, std::memory_order_release);
    return;
  }

  // A push landed, or release dropped thread_count to zero, or abort
  // closed us — re-post and let the next dispatch drain and/or
  // finalize. We do not inline the finalize here to keep that code
  // path in one place.
  env->host_->post_task(env->host_->data, tsfn, tsfnDispatch);
}

//===========================================================================
// napi_env__ TSFN management
//===========================================================================

void napi_env__::addTsfn(napi_threadsafe_function__ *tsfn) {
  tsfn->prev_ = nullptr;
  tsfn->next_ = tsfnListHead_;
  if (tsfnListHead_) {
    tsfnListHead_->prev_ = tsfn;
  }
  tsfnListHead_ = tsfn;
}

void napi_env__::removeTsfn(napi_threadsafe_function__ *tsfn) {
  if (tsfn->prev_) {
    tsfn->prev_->next_ = tsfn->next_;
  } else {
    tsfnListHead_ = tsfn->next_;
  }
  if (tsfn->next_) {
    tsfn->next_->prev_ = tsfn->prev_;
  }
  tsfn->prev_ = nullptr;
  tsfn->next_ = nullptr;
}

void napi_env__::markTsfns(hermes::vm::RootAcceptor &acceptor) {
  for (auto *tsfn = tsfnListHead_; tsfn; tsfn = tsfn->next_) {
    if (!tsfn->js_func.isUndefined()) {
      acceptor.accept(tsfn->js_func);
    }
  }
}

//===========================================================================
// TSFN cleanup (called from napi_env__ destructor)
//===========================================================================

/// Clean up all active TSFNs during env teardown. Drains remaining
/// queue items with env=nullptr (closing signal), calls finalize
/// callbacks, and deletes the TSFN structs.
void hermes_napi_cleanup_tsfns(napi_env env) {
  while (env->tsfnListHead_) {
    auto *tsfn = env->tsfnListHead_;
    env->tsfnListHead_ = tsfn->next_;
    // Drain remaining items with env=nullptr to signal closing.
    while (!tsfn->queue.empty()) {
      void *item = tsfn->queue.front();
      tsfn->queue.pop();
      tsfn->call_js_cb(nullptr, nullptr, tsfn->context, item);
    }
    if (tsfn->finalize_cb) {
      tsfn->finalize_cb(env, tsfn->finalize_data, tsfn->context);
    }
    // Release the env-level loop ref so the env's counter stays
    // accurate during teardown. The env destructor unregisters the
    // host source after this loop finishes (see ~napi_env__).
    if (tsfn->loop_refed) {
      env->releaseTsfnLoopRef();
      tsfn->loop_refed = false;
    }
    delete tsfn;
  }
}

//===========================================================================
// NAPI thread-safe function API
//===========================================================================

napi_status NAPI_CDECL napi_create_threadsafe_function(
    napi_env env,
    napi_value func,
    napi_value async_resource,
    napi_value async_resource_name,
    size_t max_queue_size,
    size_t initial_thread_count,
    void *thread_finalize_data,
    napi_finalize thread_finalize_cb,
    void *context,
    napi_threadsafe_function_call_js call_js_cb,
    napi_threadsafe_function *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);
  RETURN_STATUS_IF_FALSE(env, initial_thread_count > 0, napi_invalid_arg);

  // A host must be provided for thread-safe functions.
  RETURN_STATUS_IF_FALSE(env, env->host_ != nullptr, napi_generic_failure);
  RETURN_STATUS_IF_FALSE(
      env, env->host_->post_task != nullptr, napi_generic_failure);

  // If func is null, call_js_cb must be provided (call_js_cb-only
  // mode). If func is provided, it must be a function.
  if (func == nullptr) {
    RETURN_STATUS_IF_FALSE(env, call_js_cb != nullptr, napi_invalid_arg);
  } else {
    auto *phv = reinterpret_cast<const PinnedHermesValue *>(func);
    RETURN_STATUS_IF_FALSE(
        env, phv->isObject() && vmisa<Callable>(*phv), napi_invalid_arg);
  }

  // async_resource and async_resource_name are for Node.js async hooks
  // tracing. Hermes does not have async hooks, so we accept and ignore
  // these parameters (async_resource_name is not even validated, to
  // avoid allocating a string just for diagnostics).
  (void)async_resource;
  (void)async_resource_name;

  auto *tsfn = new napi_threadsafe_function__;
  tsfn->env = env;
  tsfn->max_queue_size = max_queue_size;
  tsfn->thread_count = initial_thread_count;
  tsfn->context = context;
  tsfn->finalize_data = thread_finalize_data;
  tsfn->finalize_cb = thread_finalize_cb;
  tsfn->call_js_cb = call_js_cb ? call_js_cb : defaultCallJsCb;

  if (func != nullptr) {
    auto *phv = reinterpret_cast<const PinnedHermesValue *>(func);
    tsfn->js_func = *phv;
  }
  // else: js_func stays undefined (default-constructed PinnedHermesValue)

  env->addTsfn(tsfn);

  // A referenced TSFN keeps the loop alive for its lifetime. Without
  // this, producer threads racing the event loop's shutdown could
  // dispatch into a torn-down env. The env coalesces all referenced
  // tsfns into one host loop source; see napi_env__::acquireTsfnLoopRef.
  if (tsfn->is_ref && env->host_->ref_loop != nullptr) {
    env->acquireTsfnLoopRef();
    tsfn->loop_refed = true;
  }

  *result = tsfn;
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL napi_get_threadsafe_function_context(
    napi_threadsafe_function func,
    void **result) {
  if (func == nullptr)
    return napi_invalid_arg;
  if (result == nullptr)
    return napi_invalid_arg;

  *result = func->context;
  return napi_ok;
}

napi_status NAPI_CDECL napi_call_threadsafe_function(
    napi_threadsafe_function func,
    void *data,
    napi_threadsafe_function_call_mode is_blocking) {
  if (func == nullptr)
    return napi_invalid_arg;

  {
    std::unique_lock<std::mutex> lock(func->mutex);

    // Wait if queue is full and blocking mode is requested.
    while (func->max_queue_size > 0 &&
           func->queue.size() >= func->max_queue_size && !func->is_closing) {
      if (is_blocking == napi_tsfn_nonblocking) {
        return napi_queue_full;
      }
      func->cond.wait(lock);
    }

    if (func->is_closing) {
      if (func->thread_count == 0) {
        return napi_invalid_arg;
      }
      return napi_closing;
    }

    func->queue.push(data);
  }

  // Post a dispatch task if one is not already pending.
  bool expected = false;
  if (func->dispatch_pending.compare_exchange_strong(
          expected, true, std::memory_order_acq_rel)) {
    func->env->host_->post_task(func->env->host_->data, func, tsfnDispatch);
  }

  return napi_ok;
}

napi_status NAPI_CDECL
napi_acquire_threadsafe_function(napi_threadsafe_function func) {
  if (func == nullptr)
    return napi_invalid_arg;

  std::lock_guard<std::mutex> lock(func->mutex);

  if (func->is_closing) {
    return napi_closing;
  }

  func->thread_count++;
  return napi_ok;
}

napi_status NAPI_CDECL napi_release_threadsafe_function(
    napi_threadsafe_function func,
    napi_threadsafe_function_release_mode mode) {
  if (func == nullptr)
    return napi_invalid_arg;

  bool should_dispatch = false;

  {
    std::lock_guard<std::mutex> lock(func->mutex);

    if (func->thread_count == 0) {
      return napi_invalid_arg;
    }

    func->thread_count--;

    if (func->thread_count == 0 || mode == napi_tsfn_abort) {
      if (!func->is_closing) {
        func->is_closing = (mode == napi_tsfn_abort);
        if (func->is_closing && func->max_queue_size > 0) {
          // Wake any threads blocked in Push().
          func->cond.notify_all();
        }
        should_dispatch = true;
      }
    }
  }

  // Post a dispatch task to trigger finalization on the main thread.
  if (should_dispatch) {
    bool expected = false;
    if (func->dispatch_pending.compare_exchange_strong(
            expected, true, std::memory_order_acq_rel)) {
      func->env->host_->post_task(func->env->host_->data, func, tsfnDispatch);
    }
  }

  return napi_ok;
}

napi_status NAPI_CDECL napi_ref_threadsafe_function(
    node_api_basic_env env,
    napi_threadsafe_function func) {
  CHECK_ENV(env);
  if (func == nullptr) {
    return napi_set_last_error(env, napi_invalid_arg);
  }

  if (!func->is_ref) {
    func->is_ref = true;
    if (!func->loop_refed && env->host_->ref_loop != nullptr) {
      env->acquireTsfnLoopRef();
      func->loop_refed = true;
    }
  }
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL napi_unref_threadsafe_function(
    node_api_basic_env env,
    napi_threadsafe_function func) {
  CHECK_ENV(env);
  if (func == nullptr) {
    return napi_set_last_error(env, napi_invalid_arg);
  }

  if (func->is_ref) {
    func->is_ref = false;
    if (func->loop_refed) {
      env->releaseTsfnLoopRef();
      func->loop_refed = false;
    }
  }
  return napi_clear_last_error(env);
}
