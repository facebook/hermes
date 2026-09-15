/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * Test addon exercising napi_create_async_work / napi_queue_async_work
 * and napi_create_threadsafe_function end-to-end. Used by lit tests
 * under test/napi/ to verify ConsoleHost's NAPI host adapter wires
 * post_work and post_task into EventLoopControl correctly.
 *
 * Exports:
 *   doWork(input, callback)         -- async work that returns input*2
 *   doManyWorks(n, callback)        -- queues N concurrent async works
 *   postFromThread(n, callback)     -- threadsafe-function calls from
 *                                      a worker thread
 */

#include "hermes/napi/js_native_api.h"
#include "hermes/napi/node_api.h"

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/// Thread id of the thread that ran NAPI_MODULE_INIT (i.e. the JS thread).
static pthread_t g_js_tid;

//===========================================================================
// doWork(input, callback)
//===========================================================================

typedef struct {
  napi_async_work work;
  napi_ref callback;
  int32_t input;
  int32_t output;
  pthread_t execute_tid;
  pthread_t complete_tid;
} DoWorkCtx;

static void doWork_execute(napi_env env, void *data) {
  (void)env;
  DoWorkCtx *c = (DoWorkCtx *)data;
  c->execute_tid = pthread_self();
  c->output = c->input * 2;
}

static void doWork_complete(napi_env env, napi_status status, void *data) {
  (void)status;
  DoWorkCtx *c = (DoWorkCtx *)data;
  c->complete_tid = pthread_self();

  napi_value cb, recv, args[3];
  napi_get_reference_value(env, c->callback, &cb);
  napi_get_undefined(env, &recv);
  napi_create_int32(env, c->output, &args[0]);
  napi_get_boolean(env, !pthread_equal(c->execute_tid, g_js_tid), &args[1]);
  napi_get_boolean(env, pthread_equal(c->complete_tid, g_js_tid), &args[2]);
  napi_call_function(env, recv, cb, 3, args, NULL);

  napi_delete_reference(env, c->callback);
  napi_delete_async_work(env, c->work);
  free(c);
}

static napi_value doWork(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  DoWorkCtx *c = (DoWorkCtx *)calloc(1, sizeof(DoWorkCtx));
  napi_get_value_int32(env, argv[0], &c->input);
  napi_create_reference(env, argv[1], 1, &c->callback);

  napi_value resource_name;
  napi_create_string_utf8(env, "doWork", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_async_work(
      env, NULL, resource_name, doWork_execute, doWork_complete, c, &c->work);
  napi_queue_async_work(env, c->work);
  return NULL;
}

//===========================================================================
// doManyWorks(n, callback)
//===========================================================================

typedef struct {
  napi_ref callback;
  int32_t remaining;
  pthread_mutex_t mu;
} ManyCtx;

typedef struct {
  napi_async_work work;
  ManyCtx *shared;
} ManyItem;

static void many_execute(napi_env env, void *data) {
  (void)env;
  (void)data;
  // No-op: source counting is what we're actually testing.
}

static void many_complete(napi_env env, napi_status status, void *data) {
  (void)status;
  ManyItem *item = (ManyItem *)data;
  ManyCtx *s = item->shared;

  pthread_mutex_lock(&s->mu);
  --s->remaining;
  int32_t left = s->remaining;
  pthread_mutex_unlock(&s->mu);

  napi_delete_async_work(env, item->work);
  free(item);

  if (left == 0) {
    napi_value cb, recv;
    napi_get_reference_value(env, s->callback, &cb);
    napi_get_undefined(env, &recv);
    napi_call_function(env, recv, cb, 0, NULL, NULL);
    napi_delete_reference(env, s->callback);
    pthread_mutex_destroy(&s->mu);
    free(s);
  }
}

static napi_value doManyWorks(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  int32_t n = 0;
  napi_get_value_int32(env, argv[0], &n);

  ManyCtx *s = (ManyCtx *)calloc(1, sizeof(ManyCtx));
  s->remaining = n;
  pthread_mutex_init(&s->mu, NULL);
  napi_create_reference(env, argv[1], 1, &s->callback);

  napi_value resource_name;
  napi_create_string_utf8(env, "many", NAPI_AUTO_LENGTH, &resource_name);

  for (int32_t i = 0; i < n; i++) {
    ManyItem *item = (ManyItem *)calloc(1, sizeof(ManyItem));
    item->shared = s;
    napi_create_async_work(
        env,
        NULL,
        resource_name,
        many_execute,
        many_complete,
        item,
        &item->work);
    napi_queue_async_work(env, item->work);
  }
  return NULL;
}

//===========================================================================
// postFromThread(n, callback)
//===========================================================================

typedef struct {
  napi_threadsafe_function tsfn;
  int32_t n;
} TsfnCtx;

/// Called on the JS thread for each tsfn dispatch. `data` is the int
/// argument passed by the worker. When `data == n`, invoke the JS
/// callback (stored in `context`) to signal completion.
static void
tsfn_call_js(napi_env env, napi_value js_callback, void *context, void *data) {
  (void)js_callback;
  napi_ref *cbRef = (napi_ref *)context;
  intptr_t i = (intptr_t)data;

  napi_value cb, recv, arg;
  napi_get_reference_value(env, *cbRef, &cb);
  napi_get_undefined(env, &recv);
  napi_create_int32(env, (int32_t)i, &arg);
  napi_call_function(env, recv, cb, 1, &arg, NULL);
}

static void tsfn_finalize(napi_env env, void *data, void *hint) {
  (void)hint;
  napi_ref *cbRef = (napi_ref *)data;
  napi_delete_reference(env, *cbRef);
  free(cbRef);
}

static void *tsfn_worker_main(void *arg) {
  TsfnCtx *c = (TsfnCtx *)arg;
  // We inherit the initial thread-count slot from create; no acquire
  // needed (and no acquire is safe — the JS thread does not release,
  // so refcount cannot drop to 0 until this worker releases below).
  for (int32_t i = 1; i <= c->n; i++) {
    napi_call_threadsafe_function(
        c->tsfn, (void *)(intptr_t)i, napi_tsfn_blocking);
  }
  napi_release_threadsafe_function(c->tsfn, napi_tsfn_release);
  free(c);
  return NULL;
}

static napi_value postFromThread(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  int32_t n = 0;
  napi_get_value_int32(env, argv[0], &n);

  napi_ref *cbRef = (napi_ref *)calloc(1, sizeof(napi_ref));
  napi_create_reference(env, argv[1], 1, cbRef);

  TsfnCtx *c = (TsfnCtx *)calloc(1, sizeof(TsfnCtx));
  c->n = n;

  napi_value resource_name;
  napi_create_string_utf8(env, "tsfn", NAPI_AUTO_LENGTH, &resource_name);
  napi_create_threadsafe_function(
      env,
      NULL, /* func: use call_js_cb instead */
      NULL, /* async_resource */
      resource_name,
      0, /* max_queue_size: unlimited */
      1, /* initial_thread_count */
      cbRef, /* thread_finalize_data: free'd by tsfn_finalize */
      tsfn_finalize,
      cbRef, /* context: passed to tsfn_call_js */
      tsfn_call_js,
      &c->tsfn);

  // The initial_thread_count=1 slot is owned by the worker — we do not
  // release on the JS thread. The worker releases when it has finished
  // calling, which drops refcount to 0 and triggers tsfn_finalize.
  pthread_t worker;
  pthread_create(&worker, NULL, tsfn_worker_main, c);
  pthread_detach(worker);
  return NULL;
}

//===========================================================================
// Module init
//===========================================================================

NAPI_MODULE_INIT() {
  g_js_tid = pthread_self();

  napi_value fn;
  napi_create_function(env, "doWork", NAPI_AUTO_LENGTH, doWork, NULL, &fn);
  napi_set_named_property(env, exports, "doWork", fn);
  napi_create_function(
      env, "doManyWorks", NAPI_AUTO_LENGTH, doManyWorks, NULL, &fn);
  napi_set_named_property(env, exports, "doManyWorks", fn);
  napi_create_function(
      env, "postFromThread", NAPI_AUTO_LENGTH, postFromThread, NULL, &fn);
  napi_set_named_property(env, exports, "postFromThread", fn);

  return exports;
}
