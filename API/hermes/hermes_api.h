/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_HERMES_API_H
#define HERMES_HERMES_API_H

#include "js_runtime_api.h"

EXTERN_C_START

typedef struct hermes_local_connection_s *hermes_local_connection;
typedef struct hermes_remote_connection_s *hermes_remote_connection;

//=============================================================================
// jsr_runtime
//=============================================================================

JSR_API hermes_dump_crash_data(jsr_runtime runtime, int32_t fd);
JSR_API hermes_sampling_profiler_enable();
JSR_API hermes_sampling_profiler_disable();
JSR_API hermes_sampling_profiler_add(jsr_runtime runtime);
JSR_API hermes_sampling_profiler_remove(jsr_runtime runtime);
JSR_API hermes_sampling_profiler_dump_to_file(const char *filename);

//=============================================================================
// jsr_config
//=============================================================================

JSR_API hermes_config_enable_default_crash_handler(
    jsr_config config,
    bool value);

//=============================================================================
// Setting inspector singleton
//=============================================================================

typedef int32_t(NAPI_CDECL *hermes_inspector_add_page_cb)(
    const char *title,
    const char *vm,
    void *connectFunc);

typedef void(NAPI_CDECL *hermes_inspector_remove_page_cb)(int32_t page_id);

JSR_API hermes_set_inspector(
    hermes_inspector_add_page_cb add_page_cb,
    hermes_inspector_remove_page_cb remove_page_cb);

//=============================================================================
// Local and remote inspector connections.
// Local is defined in Hermes VM, Remote is defined by inspector outside of VM.
//=============================================================================

typedef void(NAPI_CDECL *hermes_remote_connection_send_message_cb)(
    hermes_remote_connection remote_connection,
    const char *message);

typedef void(NAPI_CDECL *hermes_remote_connection_disconnect_cb)(
    hermes_remote_connection remote_connection);

JSR_API hermes_create_local_connection(
    void *connect_func,
    hermes_remote_connection remote_connection,
    hermes_remote_connection_send_message_cb on_send_message_cb,
    hermes_remote_connection_disconnect_cb on_disconnect_cb,
    jsr_data_delete_cb on_delete_cb,
    void *deleter_data,
    hermes_local_connection *local_connection);

JSR_API hermes_delete_local_connection(
    hermes_local_connection local_connection);

JSR_API hermes_local_connection_send_message(
    hermes_local_connection local_connection,
    const char *message);

JSR_API hermes_local_connection_disconnect(
    hermes_local_connection local_connection);

EXTERN_C_END

#endif // !HERMES_HERMES_API_H
