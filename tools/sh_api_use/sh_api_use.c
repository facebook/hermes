/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"

/*
function sum(from, to) {
  var sum = 0;
  for(; from <= to; ++from)
    sum += from;
  return sum;
}
print(sum(1, 100));
*/

static const char s_ascii_pool[] = "sum\0print\0";
static const uint32_t s_strings[] = {0, 3, 0, 4, 5, 0};
static SHSymbolID s_symbols[2];
static char s_prop_cache[2 * SH_PROPERTY_CACHE_ENTRY_SIZE];

static SHLegacyValue unit_main(SHRuntime *shr);

static SHUnit s_this_unit = {
    .num_symbols = 2,
    .num_prop_cache_entries = 2,
    .ascii_pool = s_ascii_pool,
    .u16_pool = 0,
    .strings = s_strings,
    .symbols = s_symbols,
    .prop_cache = s_prop_cache,
    .unit_main = unit_main,
    .unit_name = "sh_api_use",
};

static SHLegacyValue sum(SHRuntime *shr) {
  struct {
    SHLocals head;
    SHLegacyValue from;
    SHLegacyValue to;
    SHLegacyValue sum;
  } locals;
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 0);
  locals.head.count = 3;
  locals.from = _sh_ljs_param(frame, 1);
  locals.to = _sh_ljs_param(frame, 2);
  locals.sum = _sh_ljs_double(0);

  if (!_sh_ljs_less_equal_rjs(shr, &locals.from, &locals.to))
    goto L1;
L2:
  locals.sum = _sh_ljs_add_rjs(shr, &locals.sum, &locals.from);
  locals.from = _sh_ljs_double(_sh_ljs_to_double_rjs(shr, &locals.from) + 1);
  if (_sh_ljs_less_equal_rjs(shr, &locals.from, &locals.to))
    goto L2;
L1:
  _sh_leave(shr, &locals.head, frame);
  return locals.sum;
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

  _sh_ljs_declare_global_var(shr, s_symbols[0]);
  _sh_ljs_create_environment(shr, frame, &locals.t0, 0);
  locals.t1 = _sh_ljs_create_closure(shr, &locals.t0, sum, s_symbols[0], 2);
  locals.t0 = _sh_ljs_get_global_object(shr);
  _sh_ljs_put_by_id_loose_rjs(
      shr,
      &locals.t0,
      s_symbols[0],
      &locals.t1,
      s_prop_cache + SH_PROPERTY_CACHE_ENTRY_SIZE * 0);
  locals.t2 = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      s_symbols[1],
      s_prop_cache + SH_PROPERTY_CACHE_ENTRY_SIZE * 1);
  locals.t3 = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      s_symbols[0],
      s_prop_cache + SH_PROPERTY_CACHE_ENTRY_SIZE * 0);
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
  SHRuntime *shr = _sh_init();
  bool success = _sh_initialize_units(shr, 1, &s_this_unit);
  _sh_done(shr);
  return !success;
}
