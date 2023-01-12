/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"

/*
function make(step) {
    function sum(from, to) {
      var sum = 0;
      for(; from <= to; from += step)
        sum += from;
      return sum;
    }
    return sum;
}
print(make(1)(1, 100));
*/

static const char s_ascii_pool[] = "sum\0print\0make\0";
static const uint32_t s_strings[] = {0, 3, 0, 4, 5, 0, 10, 4, 0};
static SHSymbolID s_symbols[3];
// put "make"
// get "print"
// get "make"
static char s_prop_cache[3 * SH_PROPERTY_CACHE_ENTRY_SIZE];

static SHLegacyValue unit_main(SHRuntime *shr);

static SHUnit s_this_unit = {
    .num_symbols = 3,
    .num_prop_cache_entries = 3,
    .ascii_pool = s_ascii_pool,
    .u16_pool = 0,
    .strings = s_strings,
    .symbols = s_symbols,
    .prop_cache = s_prop_cache,
    .unit_main = unit_main,
    .unit_main_strict = false,
    .unit_name = "sh_api_use",
};

static SHLegacyValue sum(SHRuntime *shr) {
  struct {
    SHLocals head;
    SHLegacyValue t0, t2, t3, t4, t6;
  } locals;
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 0);
  locals.head.count = 5;
  locals.t4 = _sh_ljs_param(frame, 1);
  locals.t3 = _sh_ljs_param(frame, 2);
  locals.t2 = _sh_ljs_get_env(shr, frame, 0);
  locals.t0 = _sh_ljs_double(0);

  if (!_sh_ljs_less_equal_rjs(shr, &locals.t4, &locals.t3))
    goto L1;
L2:
  locals.t0 = _sh_ljs_add_rjs(shr, &locals.t0, &locals.t4);
  locals.t6 = _sh_ljs_load_from_env(locals.t2, 0);
  locals.t4 = _sh_ljs_add_rjs(shr, &locals.t4, &locals.t6);
  if (_sh_ljs_less_equal_rjs(shr, &locals.t4, &locals.t3))
    goto L2;
L1:
  _sh_leave(shr, &locals.head, frame);
  return locals.t0;
}

static SHLegacyValue make(SHRuntime *shr) {
  struct {
    SHLocals head;
    SHLegacyValue t0;
  } locals;
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 0);
  locals.head.count = 1;
  locals.t0 = _sh_ljs_undefined();

  _sh_ljs_create_environment(shr, frame, &locals.t0, 1);
  _sh_ljs_store_to_env(shr, locals.t0, _sh_ljs_param(frame, 1), 0);
  locals.t0 =
      _sh_ljs_create_closure_loose(shr, &locals.t0, sum, s_symbols[0], 2);

  _sh_leave(shr, &locals.head, frame);
  return locals.t0;
}

static SHLegacyValue unit_main(SHRuntime *shr) {
  struct {
    SHLocals head;
    SHLegacyValue t0, t1, t2, t3;
  } locals;
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 11);
  locals.head.count = 4;
  locals.t0 = _sh_ljs_undefined();
  locals.t1 = _sh_ljs_undefined();
  locals.t2 = _sh_ljs_undefined();
  locals.t3 = _sh_ljs_undefined();

  _sh_ljs_declare_global_var(shr, s_symbols[2]);
  _sh_ljs_create_environment(shr, frame, &locals.t0, 0);
  locals.t1 =
      _sh_ljs_create_closure_loose(shr, &locals.t0, make, s_symbols[2], 2);
  locals.t0 = _sh_ljs_get_global_object(shr);
  _sh_ljs_put_by_id_loose_rjs(
      shr,
      &locals.t0,
      s_symbols[2],
      &locals.t1,
      s_prop_cache + SH_PROPERTY_CACHE_ENTRY_SIZE * 0);
  locals.t2 = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      s_symbols[1],
      s_prop_cache + SH_PROPERTY_CACHE_ENTRY_SIZE * 1);
  locals.t0 = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      s_symbols[2],
      s_prop_cache + SH_PROPERTY_CACHE_ENTRY_SIZE * 2);

  frame[5] = locals.t0;
  frame[4] = _sh_ljs_undefined(); // this
  frame[3] = _sh_ljs_double(1);
  locals.t3 = _sh_ljs_call(shr, frame, 1);

  frame[5] = locals.t3;
  frame[4] = _sh_ljs_undefined(); // this
  frame[3] = _sh_ljs_double(1);
  frame[2] = _sh_ljs_double(100);
  locals.t0 = _sh_ljs_call(shr, frame, 2);

  frame[5] = locals.t2;
  frame[4] = _sh_ljs_undefined();
  frame[3] = locals.t0;
  locals.t0 = _sh_ljs_call(shr, frame, 1);

  _sh_leave(shr, &locals.head, frame);
  return locals.t0;
}

int main(int argc, char **argv) {
  _SH_MODEL();
  SHRuntime *shr = _sh_init(argc, argv);
  bool success = _sh_initialize_units(shr, 1, &s_this_unit);
  _sh_done(shr);
  return !success;
}
