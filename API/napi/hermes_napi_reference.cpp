/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi_impl.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/WeakRef.h"

using namespace hermes::vm;

//===========================================================================
// napi_env__ reference list management
//===========================================================================

void napi_env__::addReference(napi_ref__ *ref) {
  ref->prev_ = nullptr;
  ref->next_ = refListHead_;
  if (refListHead_) {
    refListHead_->prev_ = ref;
  }
  refListHead_ = ref;
}

void napi_env__::removeReference(napi_ref__ *ref) {
  if (ref->prev_) {
    ref->prev_->next_ = ref->next_;
  } else {
    // This is the head.
    refListHead_ = ref->next_;
  }
  if (ref->next_) {
    ref->next_->prev_ = ref->prev_;
  }
  ref->prev_ = nullptr;
  ref->next_ = nullptr;
}

void napi_env__::markReferences(RootAcceptor &acceptor) {
  for (napi_ref__ *ref = refListHead_; ref; ref = ref->next_) {
    if (ref->isStrong()) {
      acceptor.accept(ref->value);
    }
  }
}

//===========================================================================
// napi_create_reference
//===========================================================================

napi_status NAPI_CDECL napi_create_reference(
    napi_env env,
    napi_value value,
    uint32_t initial_refcount,
    napi_ref *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  auto hv = *reinterpret_cast<const PinnedHermesValue *>(value);

  // Before NAPI v10, only objects, functions, and symbols can be
  // referenced. From v10 onwards, all value types are supported.
  if (env->module_api_version < 10) {
    bool isRefable = hv.isObject() || hv.isSymbol();
    RETURN_STATUS_IF_FALSE(env, isRefable, napi_invalid_arg);
  }

  auto *ref = new napi_ref__;
  ref->env = env;
  ref->refcount = initial_refcount;
  ref->value = hv;

  // If this is a weak reference, move the value into a weak slot.
  if (initial_refcount == 0) {
    ref->createWeakSlot(env->runtime);
  }

  env->addReference(ref);
  *result = ref;
  return napi_clear_last_error(env);
}

//===========================================================================
// napi_delete_reference
//===========================================================================

napi_status NAPI_CDECL
napi_delete_reference(node_api_basic_env env, napi_ref ref) {
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  // If this reference is already being cleaned up (e.g., during env
  // teardown and the finalizer calls napi_delete_reference on itself),
  // just return OK — the caller will complete the deletion.
  if (ref->deletionPending_) {
    return napi_clear_last_error(env);
  }

  // Matching V8 behavior: napi_delete_reference does NOT invoke the
  // user's finalize callback. The finalizer is only called during GC
  // or env teardown.

  // Release the weak slot if one exists.
  ref->releaseWeakSlot();

  // Remove from the env's reference list.
  const_cast<napi_env>(env)->removeReference(ref);

  delete ref;
  return napi_clear_last_error(env);
}

//===========================================================================
// napi_reference_ref
//===========================================================================

napi_status NAPI_CDECL
napi_reference_ref(napi_env env, napi_ref ref, uint32_t *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  // Transitioning from weak (0) to strong (1).
  if (ref->refcount == 0) {
    if (!ref->weakSlot_) {
      // Known dead (state 3). No-op, matching V8 behavior.
      if (result) {
        *result = 0;
      }
      return napi_clear_last_error(env);
    }
    if (!ref->weakSlot_->hasValue()) {
      // The referent has been collected. Transition to known dead.
      ref->releaseWeakSlot();
      if (result) {
        *result = 0;
      }
      return napi_clear_last_error(env);
    }
    // Restore the value from the weak slot.
    ref->value = ref->getWeakValue(env->runtime);
    ref->releaseWeakSlot();
  }

  ref->refcount++;

  if (result) {
    *result = ref->refcount;
  }
  return napi_clear_last_error(env);
}

//===========================================================================
// napi_reference_unref
//===========================================================================

napi_status NAPI_CDECL
napi_reference_unref(napi_env env, napi_ref ref, uint32_t *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  // Cannot unref if already at 0.
  RETURN_STATUS_IF_FALSE(env, ref->refcount > 0, napi_generic_failure);

  ref->refcount--;

  // Transitioning from strong (1) to weak (0).
  if (ref->refcount == 0) {
    ref->createWeakSlot(env->runtime);
  }

  if (result) {
    *result = ref->refcount;
  }
  return napi_clear_last_error(env);
}

//===========================================================================
// napi_get_reference_value
//===========================================================================

napi_status NAPI_CDECL
napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, ref);
  CHECK_ARG(env, result);

  if (ref->isStrong()) {
    // Strong reference: value is always valid.
    *result = env->addToCurrentScope(ref->value);
  } else if (ref->weakSlot_ && ref->weakSlot_->hasValue()) {
    // Weak reference, still alive. Read through the weak slot.
    *result = env->addToCurrentScope(ref->getWeakValue(env->runtime));
  } else {
    // Weak reference: either known dead (no slot) or just collected.
    *result = nullptr;
  }

  return napi_clear_last_error(env);
}
