/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi_impl.h"

//===========================================================================
// Handle scopes
//===========================================================================

napi_status NAPI_CDECL
napi_open_handle_scope(napi_env env, napi_handle_scope *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  // Push a new scope descriptor recording the current slot position.
  HandleScopeDescriptor desc;
  desc.blockIndex = env->currentBlockIndex_;
  desc.slotIndex = env->currentSlotIndex_;
  env->scopeStack_.push_back(desc);
  ++env->open_handle_scopes;

  // Return the scope stack depth as the opaque scope handle. This is
  // just a tag for mismatch detection — the actual scope data lives in
  // the scopeStack_ vector, which may reallocate.
  *result = reinterpret_cast<napi_handle_scope>(
      static_cast<uintptr_t>(env->scopeStack_.size()));
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL
napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  CHECK_ENV(env);
  CHECK_ARG(env, scope);

  if (env->open_handle_scopes == 0) {
    return napi_set_last_error(env, napi_handle_scope_mismatch);
  }

  // Validate LIFO order: the scope being closed must be the topmost.
  auto expectedDepth = static_cast<uintptr_t>(env->scopeStack_.size());
  auto scopeDepth = reinterpret_cast<uintptr_t>(scope);
  if (scopeDepth != expectedDepth) {
    return napi_set_last_error(env, napi_handle_scope_mismatch);
  }

  // Restore the slot pointer to where it was when this scope was
  // opened, effectively reclaiming all handles allocated in this
  // scope.
  auto &desc = env->scopeStack_.back();
  env->currentBlockIndex_ = desc.blockIndex;
  env->currentSlotIndex_ = desc.slotIndex;
  env->scopeStack_.pop_back();
  --env->open_handle_scopes;

  return napi_clear_last_error(env);
}

//===========================================================================
// Escapable handle scopes
//===========================================================================

napi_status NAPI_CDECL napi_open_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  // An escapable scope requires a parent scope to exist, because the
  // escape slot is pre-allocated in the parent scope's storage.
  RETURN_STATUS_IF_FALSE(
      env, !env->scopeStack_.empty(), napi_handle_scope_mismatch);

  // Pre-allocate one slot in the parent (current) scope for the
  // escaped value. Initialize it to undefined — it will be
  // overwritten by napi_escape_handle if/when the caller escapes a
  // value.
  hermes::vm::PinnedHermesValue *escapeSlot =
      reinterpret_cast<hermes::vm::PinnedHermesValue *>(env->addToCurrentScope(
          hermes::vm::HermesValue::encodeUndefinedValue()));

  // Now push a new scope descriptor, like napi_open_handle_scope, but
  // marked as escapable with a pointer to the pre-allocated slot.
  HandleScopeDescriptor desc;
  desc.blockIndex = env->currentBlockIndex_;
  desc.slotIndex = env->currentSlotIndex_;
  desc.escapable = true;
  desc.escapeSlot = escapeSlot;
  env->scopeStack_.push_back(desc);
  ++env->open_handle_scopes;

  *result = reinterpret_cast<napi_escapable_handle_scope>(
      static_cast<uintptr_t>(env->scopeStack_.size()));
  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL napi_close_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope scope) {
  CHECK_ENV(env);
  CHECK_ARG(env, scope);

  if (env->open_handle_scopes == 0) {
    return napi_set_last_error(env, napi_handle_scope_mismatch);
  }

  // Validate LIFO order.
  auto expectedDepth = static_cast<uintptr_t>(env->scopeStack_.size());
  auto scopeDepth = reinterpret_cast<uintptr_t>(scope);
  if (scopeDepth != expectedDepth) {
    return napi_set_last_error(env, napi_handle_scope_mismatch);
  }

  // Restore the slot pointer to where it was when this scope was
  // opened.
  auto &desc = env->scopeStack_.back();
  env->currentBlockIndex_ = desc.blockIndex;
  env->currentSlotIndex_ = desc.slotIndex;
  env->scopeStack_.pop_back();
  --env->open_handle_scopes;

  return napi_clear_last_error(env);
}

napi_status NAPI_CDECL napi_escape_handle(
    napi_env env,
    napi_escapable_handle_scope scope,
    napi_value escapee,
    napi_value *result) {
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  CHECK_ARG(env, escapee);
  CHECK_ARG(env, result);

  // Validate the scope tag matches the current topmost scope.
  auto expectedDepth = static_cast<uintptr_t>(env->scopeStack_.size());
  auto scopeDepth = reinterpret_cast<uintptr_t>(scope);
  RETURN_STATUS_IF_FALSE(
      env, scopeDepth == expectedDepth, napi_handle_scope_mismatch);

  auto &desc = env->scopeStack_.back();

  // The scope must be escapable.
  RETURN_STATUS_IF_FALSE(env, desc.escapable, napi_invalid_arg);

  // Can only escape once per scope.
  if (desc.escapeCalled) {
    return napi_set_last_error(env, napi_escape_called_twice);
  }

  desc.escapeCalled = true;

  // Copy the escapee's value into the pre-allocated parent scope
  // slot.
  auto *phv = reinterpret_cast<hermes::vm::PinnedHermesValue *>(escapee);
  *desc.escapeSlot = *phv;

  // Return the parent scope slot as the new napi_value.
  *result = reinterpret_cast<napi_value>(desc.escapeSlot);
  return napi_clear_last_error(env);
}
