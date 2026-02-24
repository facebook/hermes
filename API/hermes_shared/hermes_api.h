/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
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

/// Set the Intl provider mode for a runtime.
/// mode values: 0 = Default (global resolution),
///              1 = ForceWinGlob (always use NLS APIs),
///              2 = CustomVtable (use the provided vtable).
/// vtable: ICU vtable pointer (only used when mode == 2, else pass NULL).
struct hermes_icu_vtable;
JSR_API hermes_config_set_intl_provider(
    jsr_config config,
    uint8_t mode,
    const struct hermes_icu_vtable *vtable);

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

//=============================================================================
// Modern inspector API implementation
//=============================================================================

typedef struct hermes_runtime_s *hermes_runtime;
typedef struct hermes_cdp_debug_api_s *hermes_cdp_debug_api;
typedef struct hermes_cdp_agent_s *hermes_cdp_agent;
typedef struct hermes_cdp_state_s *hermes_cdp_state;
typedef struct hermes_stack_trace_s *hermes_stack_trace;
typedef struct hermes_sampling_profile_s *hermes_sampling_profile;

typedef enum {
  hermes_status_ok = 0,
  hermes_status_error = 1,
} hermes_status;

typedef enum {
  hermes_console_api_type_log,
  hermes_console_api_type_debug,
  hermes_console_api_type_info,
  hermes_console_api_type_error,
  hermes_console_api_type_warning,
  hermes_console_api_type_dir,
  hermes_console_api_type_dir_xml,
  hermes_console_api_type_table,
  hermes_console_api_type_trace,
  hermes_console_api_type_start_group,
  hermes_console_api_type_start_group_collapsed,
  hermes_console_api_type_end_group,
  hermes_console_api_type_clear,
  hermes_console_api_type_assert,
  hermes_console_api_type_time_end,
  hermes_console_api_type_count,
} hermes_console_api_type;

typedef enum {
  hermes_call_stack_frame_kind_js_function,
  hermes_call_stack_frame_kind_native_function,
  hermes_call_stack_frame_kind_host_function,
  hermes_call_stack_frame_kind_gc,
} hermes_call_stack_frame_kind;

typedef void(NAPI_CDECL *hermes_release_callback)(void *data_to_release);

typedef void(NAPI_CDECL *hermes_run_runtime_task_callback)(
    void *cb_data,
    hermes_runtime runtime);

typedef struct {
  void *data;
  hermes_run_runtime_task_callback invoke;
  hermes_release_callback release;
} hermes_run_runtime_task_functor;

typedef void(NAPI_CDECL *hermes_enqueue_runtime_task_callback)(
    void *cb_data,
    hermes_run_runtime_task_functor runtime_task);

typedef struct {
  void *data;
  hermes_enqueue_runtime_task_callback invoke;
  hermes_release_callback release;
} hermes_enqueue_runtime_task_functor;

typedef void(NAPI_CDECL *hermes_enqueue_frontend_message_callback)(
    void *cb_data,
    const char *json_utf8,
    size_t json_size);

typedef struct {
  void *data;
  hermes_enqueue_frontend_message_callback invoke;
  hermes_release_callback release;
} hermes_enqueue_frontend_message_functor;

typedef void(NAPI_CDECL *hermes_on_sampling_profile_info_callback)(
    void *cb_data,
    size_t sample_count);

typedef void(NAPI_CDECL *hermes_on_sampling_profile_sample_callback)(
    void *cb_data,
    uint64_t timestamp,
    uint64_t thread_id,
    size_t frame_count);

typedef void(NAPI_CDECL *hermes_on_sampling_profile_frame_callback)(
    void *cb_data,
    hermes_call_stack_frame_kind kind,
    uint32_t script_id,
    const char *function_name,
    size_t function_name_size,
    const char *script_url,
    size_t script_url_size,
    uint32_t line_number,
    uint32_t column_number);

typedef hermes_status(NAPI_CDECL *hermes_create_cdp_debug_api)(
    hermes_runtime runtime,
    hermes_cdp_debug_api *result);
typedef hermes_status(NAPI_CDECL *hermes_release_cdp_debug_api)(
    hermes_cdp_debug_api cdp_debug_api);

typedef hermes_status(NAPI_CDECL *hermes_cdp_debug_api_add_console_message)(
    hermes_cdp_debug_api cdp_debug_api,
    double timestamp,
    hermes_console_api_type type,
    const char *args_property_name,
    hermes_stack_trace stack_trace);

typedef hermes_status(NAPI_CDECL *hermes_create_cdp_agent)(
    hermes_cdp_debug_api cdp_debug_api,
    int32_t execution_context_id,
    hermes_enqueue_runtime_task_functor enqueue_runtime_task_callback,
    hermes_enqueue_frontend_message_functor enqueue_frontend_message_callback,
    hermes_cdp_state cdp_state,
    hermes_cdp_agent *result);
typedef hermes_status(NAPI_CDECL *hermes_release_cdp_agent)(
    hermes_cdp_agent cdp_agent);

typedef hermes_status(NAPI_CDECL *hermes_cdp_agent_get_state)(
    hermes_cdp_agent cdp_agent,
    hermes_cdp_state *result);
typedef hermes_status(NAPI_CDECL *hermes_release_cdp_state)(
    hermes_cdp_state cdp_state);

typedef hermes_status(NAPI_CDECL *hermes_cdp_agent_handle_command)(
    hermes_cdp_agent cdp_agent,
    const char *json_utf8,
    size_t json_size);
typedef hermes_status(NAPI_CDECL *hermes_cdp_agent_enable_runtime_domain)(
    hermes_cdp_agent cdp_agent);
typedef hermes_status(NAPI_CDECL *hermes_cdp_agent_enable_debugger_domain)(
    hermes_cdp_agent cdp_agent);

typedef hermes_status(NAPI_CDECL *hermes_capture_stack_trace)(
    hermes_runtime runtime,
    hermes_stack_trace *result);
typedef hermes_status(NAPI_CDECL *hermes_release_stack_trace)(
    hermes_stack_trace stack_trace);

typedef hermes_status(NAPI_CDECL *hermes_enable_sampling_profiler)(
    hermes_runtime runtime);
typedef hermes_status(NAPI_CDECL *hermes_disable_sampling_profiler)(
    hermes_runtime runtime);

typedef hermes_status(NAPI_CDECL *hermes_collect_sampling_profile)(
    hermes_runtime runtime,
    void *cb_data,
    hermes_on_sampling_profile_info_callback on_info_callback,
    hermes_on_sampling_profile_sample_callback on_sample_callback,
    hermes_on_sampling_profile_frame_callback on_frame_callback,
    hermes_sampling_profile *result);
typedef hermes_status(NAPI_CDECL *hermes_release_sampling_profile)(
    hermes_sampling_profile profile);

typedef struct {
  void *reserved0;
  void *reserved1;
  void *reserved2;

  hermes_create_cdp_debug_api create_cdp_debug_api;
  hermes_release_cdp_debug_api release_cdp_debug_api;

  hermes_cdp_debug_api_add_console_message add_console_message;

  hermes_create_cdp_agent create_cdp_agent;
  hermes_release_cdp_agent release_cdp_agent;

  hermes_cdp_agent_get_state cdp_agent_get_state;
  hermes_release_cdp_state release_cdp_state;

  hermes_cdp_agent_handle_command cdp_agent_handle_command;
  hermes_cdp_agent_enable_runtime_domain cdp_agent_enable_runtime_domain;
  hermes_cdp_agent_enable_debugger_domain cdp_agent_enable_debugger_domain;

  hermes_capture_stack_trace capture_stack_trace;
  hermes_release_stack_trace release_stack_trace;

  hermes_enable_sampling_profiler enable_sampling_profiler;
  hermes_disable_sampling_profiler disable_sampling_profiler;
  hermes_collect_sampling_profile collect_sampling_profile;
  hermes_release_sampling_profile release_sampling_profile;
} hermes_inspector_vtable;

JSR_API hermes_get_inspector_vtable(const hermes_inspector_vtable **vtable);

EXTERN_C_END

#endif // !HERMES_HERMES_API_H
