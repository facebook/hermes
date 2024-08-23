/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"

#include <stdio.h>
#include <stdlib.h>

#include "loop1-jit.h"

static uint32_t unit_index;
static inline SHSymbolID *get_symbols(SHUnit *);
static inline SHPropertyCacheEntry *get_prop_cache(SHUnit *);
static const SHSrcLoc s_source_locations[];
static SHNativeFuncInfo s_function_info_table[];
static SHLegacyValue _0_global(SHRuntime *shr);
static SHLegacyValue _1_bench(SHRuntime *shr);
// idisp.js:8:1
static SHLegacyValue _0_global(SHRuntime *shr) {
  _SH_MODEL();
  struct {
    SHLocals head;
    SHLegacyValue t0;
    SHLegacyValue t1;
    SHLegacyValue t2;
  } locals;
  _sh_check_native_stack_overflow(shr);
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 12);
  locals.head.count = 3;
  SHUnit *shUnit = shr->units[unit_index];
  locals.t0 = _sh_ljs_undefined();
  locals.t1 = _sh_ljs_undefined();
  locals.t2 = _sh_ljs_undefined();
  SHLegacyValue np0 = _sh_ljs_undefined();
  SHLegacyValue np1 = _sh_ljs_undefined();
  SHLegacyValue np2 = _sh_ljs_undefined();
  SHLegacyValue np3 = _sh_ljs_undefined();
  SHLegacyValue np4 = _sh_ljs_undefined();
  SHLegacyValue np5 = _sh_ljs_undefined();

L0:;
  _sh_ljs_declare_global_var(shr, get_symbols(shUnit)[1] /*bench*/);
  _sh_ljs_declare_global_var(shr, get_symbols(shUnit)[2] /*start*/);
  _sh_ljs_declare_global_var(shr, get_symbols(shUnit)[3] /*end*/);
  locals.t0 = _sh_ljs_create_environment(shr, NULL, 0);
  locals.t1 = _sh_ljs_create_closure(
      shr, &locals.t0, _1_bench, &s_function_info_table[1], shUnit);
  locals.t0 = _sh_ljs_get_global_object(shr);
  _sh_ljs_put_by_id_strict_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[1] /*bench*/,
      &locals.t1,
      get_prop_cache(shUnit) + 0);
  np0 = _sh_ljs_undefined();
  np3 = _sh_ljs_double(1);
  np2 = _sh_ljs_double(1000);
  np5 = _sh_ljs_double(0);
  goto L1;
L1:;
  // PhiInst
  frame[5] = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[1] /*bench*/,
      get_prop_cache(shUnit) + 1);
  frame[6] = _sh_ljs_undefined();
  frame[4] = _sh_ljs_undefined();
  frame[3] = _sh_ljs_double(10);
  frame[2] = _sh_ljs_double(10);
  locals.t1 = _sh_ljs_call(shr, frame, 2);
  np5 = _sh_ljs_double(_sh_ljs_get_double(np5) + _sh_ljs_get_double(np3));
  np1 = _sh_ljs_bool(_sh_ljs_get_double(np5) != _sh_ljs_get_double(np2));
  if (_sh_ljs_get_bool(np1))
    goto L1;
  goto L2;

L2:;
  locals.t1 = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[4] /*Date*/,
      get_prop_cache(shUnit) + 2);
  locals.t2 = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t1,
      get_symbols(shUnit)[5] /*prototype*/,
      get_prop_cache(shUnit) + 3);
  locals.t2 = _sh_ljs_create_this(shr, &locals.t2, &locals.t1);
  frame[6] = locals.t1;
  frame[5] = locals.t1;
  frame[4] = locals.t2;
  locals.t1 = _sh_ljs_call(shr, frame, 0);
  locals.t1 = _sh_ljs_is_object(locals.t1) ? locals.t1 : locals.t2;
  _sh_ljs_put_by_id_strict_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[2] /*start*/,
      &locals.t1,
      get_prop_cache(shUnit) + 4);
  locals.t2 = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[6] /*print*/,
      get_prop_cache(shUnit) + 5);
  frame[5] = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[1] /*bench*/,
      get_prop_cache(shUnit) + 6);
  frame[3] = _sh_ljs_double(4000000);
  frame[2] = _sh_ljs_double(100);
  frame[6] = _sh_ljs_undefined();
  frame[4] = _sh_ljs_undefined();
  frame[3] = _sh_ljs_call(shr, frame, 2);
  frame[6] = _sh_ljs_undefined();
  frame[5] = locals.t2;
  frame[4] = _sh_ljs_undefined();
  locals.t1 = _sh_ljs_call(shr, frame, 1);
  locals.t1 = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[4] /*Date*/,
      get_prop_cache(shUnit) + 7);
  locals.t2 = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t1,
      get_symbols(shUnit)[5] /*prototype*/,
      get_prop_cache(shUnit) + 8);
  locals.t2 = _sh_ljs_create_this(shr, &locals.t2, &locals.t1);
  frame[6] = locals.t1;
  frame[5] = locals.t1;
  frame[4] = locals.t2;
  locals.t1 = _sh_ljs_call(shr, frame, 0);
  locals.t1 = _sh_ljs_is_object(locals.t1) ? locals.t1 : locals.t2;
  _sh_ljs_put_by_id_strict_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[3] /*end*/,
      &locals.t1,
      get_prop_cache(shUnit) + 9);
  frame[5] = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[6] /*print*/,
      get_prop_cache(shUnit) + 10);
  locals.t2 = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[3] /*end*/,
      get_prop_cache(shUnit) + 11);
  locals.t0 = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[2] /*start*/,
      get_prop_cache(shUnit) + 12);
  locals.t2 = _sh_ljs_sub_rjs(shr, &locals.t2, &locals.t0);
  locals.t0 = _sh_ljs_get_string(shr, get_symbols(shUnit)[7] /*Time: */);
  frame[3] = _sh_ljs_add_rjs(shr, &locals.t0, &locals.t2);
  frame[6] = _sh_ljs_undefined();
  frame[4] = _sh_ljs_undefined();
  locals.t0 = _sh_ljs_call(shr, frame, 1);
  _sh_leave(shr, &locals.head, frame);
  return locals.t0;
}
// idisp.js:10:1
static SHLegacyValue _1_bench(SHRuntime *shr) {
  static JitFn s_fn = NULL;
  static unsigned s_count = 0;
  if (s_fn)
    return (*s_fn)(shr);
  if (++s_count >= 10) {
    s_fn = compile_loop1();
    //    exit(1);
    return (*s_fn)(shr);
  }

  struct {
    SHLocals head;
    SHLegacyValue t0;
    SHLegacyValue t1;
    SHLegacyValue t2;
    SHLegacyValue t3;
    SHLegacyValue t4;
    SHLegacyValue t5;
    SHLegacyValue t6;
    SHLegacyValue t7;
  } locals;
  _sh_check_native_stack_overflow(shr);
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 2);
  locals.head.count = 8;
  SHUnit *shUnit = shr->units[unit_index];
  locals.t0 = _sh_ljs_undefined();
  locals.t1 = _sh_ljs_undefined();
  locals.t2 = _sh_ljs_undefined();
  locals.t3 = _sh_ljs_undefined();
  locals.t4 = _sh_ljs_undefined();
  locals.t5 = _sh_ljs_undefined();
  locals.t6 = _sh_ljs_undefined();
  locals.t7 = _sh_ljs_undefined();
  SHLegacyValue np0 = _sh_ljs_undefined();
  SHLegacyValue np1 = _sh_ljs_undefined();

L0:;
  locals.t3 = _sh_ljs_param(frame, 2);
  locals.t0 = _sh_ljs_param(frame, 1);
  locals.t2 = _sh_ljs_dec_rjs(shr, &locals.t0);
  np1 = _sh_ljs_double(0);
  np0 = _sh_ljs_double(1);
  locals.t1 = _sh_ljs_double(0);
  locals.t0 = _sh_ljs_double(0);
  if (_sh_ljs_greater_equal_rjs(shr, &locals.t2, &locals.t0))
    goto L1;
  goto L4;

L1:;
  // PhiInst
  // PhiInst
  locals.t6 = _sh_ljs_dec_rjs(shr, &locals.t3);
  locals.t5 = locals.t3;
  locals.t4 = locals.t5;
  if (_sh_ljs_greater_rjs(shr, &locals.t6, &np0))
    goto L2;
  goto L3;

L2:;
  // PhiInst
  // PhiInst
  locals.t5 = _sh_ljs_mul_rjs(shr, &locals.t5, &locals.t6);
  locals.t6 = _sh_ljs_dec_rjs(shr, &locals.t6);
  locals.t4 = locals.t5;
  if (_sh_ljs_greater_rjs(shr, &locals.t6, &np0))
    goto L2;
  goto L3;

L3:;
  // PhiInst
  locals.t1 = _sh_ljs_add_rjs(shr, &locals.t1, &locals.t4);
  locals.t2 = _sh_ljs_dec_rjs(shr, &locals.t2);
  locals.t0 = locals.t1;
  if (_sh_ljs_greater_equal_rjs(shr, &locals.t2, &np1))
    goto L1;
  goto L4;

L4:;
  // PhiInst
  _sh_leave(shr, &locals.head, frame);
  return locals.t0;
}
static unsigned char s_literal_val_buffer[0] = {};
static unsigned char s_obj_key_buffer[0] = {};
static const SHShapeTableEntry s_obj_shape_table[] = {};

static const SHSrcLoc s_source_locations[] = {
    {.filename_idx = 0, .line = 0, .column = 0},
};

static SHNativeFuncInfo s_function_info_table[] = {
    {.name_index = 8, .arg_count = 0, .prohibit_invoke = 2, .kind = 0},
    {.name_index = 1, .arg_count = 2, .prohibit_invoke = 2, .kind = 0},
};
static const char s_ascii_pool[] = {
    '\0', 'b', 'e', 'n',  'c', 'h',  '\0', 's', 't',  'a', 'r', 't',  '\0',
    'e',  'n', 'd', '\0', 'D', 'a',  't',  'e', '\0', 'p', 'r', 'o',  't',
    'o',  't', 'y', 'p',  'e', '\0', 'p',  'r', 'i',  'n', 't', '\0', 'T',
    'i',  'm', 'e', ':',  ' ', '\0', 'g',  'l', 'o',  'b', 'a', 'l',  '\0',
};
static const char16_t s_u16_pool[] = {};
static const uint32_t s_strings[] = {
    0,  0, 0,          1,  5, 540019094,  7,  5, 2477978462,
    13, 3, 1517149248, 17, 4, 3442766438, 22, 9, 2155634493,
    32, 5, 2794059355, 38, 6, 3934850720, 45, 6, 615793799,
};
#define CREATE_THIS_UNIT sh_export_this_unit
struct UnitData {
  SHUnit unit;
  SHSymbolID symbol_data[9];
  SHPropertyCacheEntry prop_cache_data[13];
  ;
  SHCompressedPointer object_literal_class_cache[0];
};
SHUnit *CREATE_THIS_UNIT(SHRuntime *shr) {
  struct UnitData *unit_data = calloc(sizeof(struct UnitData), 1);
  *unit_data = (struct UnitData){
      .unit = {
          .index = &unit_index,
          .num_symbols = 9,
          .num_prop_cache_entries = 13,
          .ascii_pool = s_ascii_pool,
          .u16_pool = s_u16_pool,
          .strings = s_strings,
          .symbols = unit_data->symbol_data,
          .prop_cache = unit_data->prop_cache_data,
          .obj_key_buffer = s_obj_key_buffer,
          .obj_key_buffer_size = 0,
          .literal_val_buffer = s_literal_val_buffer,
          .literal_val_buffer_size = 0,
          .obj_shape_table = s_obj_shape_table,
          .obj_shape_table_count = 0,
          .object_literal_class_cache = unit_data->object_literal_class_cache,
          .source_locations = s_source_locations,
          .source_locations_size = 1,
          .unit_main = _0_global,
          .unit_main_info = &s_function_info_table[0],
          .unit_name = "sh_compiled"}};
  return (SHUnit *)unit_data;
}

SHSymbolID *get_symbols(SHUnit *unit) {
  return ((struct UnitData *)unit)->symbol_data;
}

SHPropertyCacheEntry *get_prop_cache(SHUnit *unit) {
  return ((struct UnitData *)unit)->prop_cache_data;
}

void init_console_bindings(SHRuntime *shr);

int main(int argc, char **argv) {
  SHRuntime *shr = _sh_init(argc, argv);
  init_console_bindings(shr);
  bool success = _sh_initialize_units(shr, 1, CREATE_THIS_UNIT);
  _sh_done(shr);
  return success ? 0 : 1;
}
