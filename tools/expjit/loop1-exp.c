/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"

#include <stdlib.h>

static uint32_t unit_index;
static inline SHSymbolID *get_symbols(SHUnit *);
static inline SHPropertyCacheEntry *get_prop_cache(SHUnit *);
static const SHSrcLoc s_source_locations[];
static SHNativeFuncInfo s_function_info_table[];
static SHLegacyValue _0_global(SHRuntime *shr);
SHLegacyValue _1_bench(SHRuntime *shr);
SHLegacyValue _1_bench_jit(SHRuntime *shr);
SHLegacyValue _1_bench_jit_exp(SHRuntime *shr);

// loop1.js:8:1
static SHLegacyValue _0_global(SHRuntime *shr) {
  _SH_MODEL();
  struct {
    SHLocals head;
    SHLegacyValue t0;
    SHLegacyValue t1;
  } locals;
  _sh_check_native_stack_overflow(shr);
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 11);
  locals.head.count = 2;
  SHUnit *shUnit = shr->units[unit_index];
  locals.t0 = _sh_ljs_undefined();
  locals.t1 = _sh_ljs_undefined();
  SHLegacyValue np2 = _sh_ljs_undefined();
  SHLegacyValue np3 = _sh_ljs_undefined();
  SHLegacyValue np4 = _sh_ljs_undefined();
  SHLegacyValue np5 = _sh_ljs_undefined();

  _sh_ljs_declare_global_var(shr, get_symbols(shUnit)[1] /*bench*/);
  locals.t0 = _sh_ljs_create_environment(shr, NULL, 0);
  locals.t1 = _sh_ljs_create_closure(
      shr, &locals.t0, _1_bench_jit, &s_function_info_table[1], shUnit);
  locals.t0 = _sh_ljs_get_global_object(shr);
  _sh_ljs_put_by_id_strict_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[1] /*bench*/,
      &locals.t1,
      get_prop_cache(shUnit) + 0);
  np4 = _sh_ljs_double(1);
  np3 = _sh_ljs_double(1000);
  np5 = _sh_ljs_double(0);
  goto L1;
L1:;
  // PhiInst
  frame[4] = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[1] /*bench*/,
      get_prop_cache(shUnit) + 1);
  frame[5] = _sh_ljs_undefined();
  frame[3] = _sh_ljs_undefined();
  frame[2] = _sh_ljs_double(100);
  locals.t1 = _sh_ljs_call(shr, frame, 1);
  np5 = _sh_ljs_double(_sh_ljs_get_double(np5) + _sh_ljs_get_double(np4));
  np2 = _sh_ljs_bool(_sh_ljs_get_double(np5) != _sh_ljs_get_double(np3));
  if (_sh_ljs_get_bool(np2))
    goto L1;
  goto L2;

L2:;
  locals.t1 = _sh_ljs_try_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[2] /*print*/,
      get_prop_cache(shUnit) + 2);
  frame[4] = _sh_ljs_get_by_id_rjs(
      shr,
      &locals.t0,
      get_symbols(shUnit)[1] /*bench*/,
      get_prop_cache(shUnit) + 3);
  frame[5] = _sh_ljs_undefined();
  frame[3] = _sh_ljs_undefined();
  frame[2] = _sh_ljs_double(100);
  frame[2] = _sh_ljs_call(shr, frame, 1);
  frame[5] = _sh_ljs_undefined();
  frame[4] = locals.t1;
  frame[3] = _sh_ljs_undefined();
  locals.t0 = _sh_ljs_call(shr, frame, 1);
  _sh_leave(shr, &locals.head, frame);
  return locals.t0;
}

// Manually compiled to C.
SHLegacyValue _1_bench_jit_exp(SHRuntime *shr) {
  struct {
    SHLocals head;
  } locals;
  _sh_check_native_stack_overflow(shr);
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 6);

  //   LoadParam         r0, 1
  frame[0] = _sh_ljs_param(frame, 1);
  //   Dec               r3, r0
  frame[3] = _sh_ljs_dec_rjs(shr, frame + 0);
  //   LoadConstUInt8    r2, 1
  frame[2] = _sh_ljs_double(1);
  //   Mov               r1, r0
  frame[1] = frame[0];
  //   Mov               r0, r1
  frame[0] = frame[1];
  //   JNotGreater       L1, r3, r2
  if (!_sh_ljs_greater_rjs(shr, frame + 3, frame + 2))
    goto L1;
L2:
  //   Mul               r1, r1, r3
  frame[1] = _sh_ljs_mul_rjs(shr, frame + 1, frame + 3);
  //   Dec               r3, r3
  frame[3] = _sh_ljs_dec_rjs(shr, frame + 3);
  //   Mov               r0, r1
  frame[0] = frame[1];
  //   JGreater          L2, r3, r2
  if (_sh_ljs_greater_rjs(shr, frame + 3, frame + 2))
    goto L2;
L1:;
  //   Ret               r0
  SHLegacyValue tmp = frame[0];
  _sh_leave(shr, &locals.head, frame);
  return tmp;
}

// Function<bench>(2 params, 6 registers):
// Offset in debug table: source 0x001c, lexical 0x0000
//   LoadParam         r0, 1
//   Dec               r3, r0
//   LoadConstUInt8    r2, 1
//   Mov               r1, r0
//   Mov               r0, r1
//   JNotGreater       L1, r3, r2
// L2:
//   Mul               r1, r1, r3
//   Dec               r3, r3
//   Mov               r0, r1
//   JGreater          L2, r3, r2
// L1:
//   Ret               r0

// loop1.js:10:1
SHLegacyValue _1_bench(SHRuntime *shr) {
  struct {
    SHLocals head;
    SHLegacyValue t0;
    SHLegacyValue t1;
    SHLegacyValue t2;
    SHLegacyValue t3;
  } locals;
  _sh_check_native_stack_overflow(shr);
  SHLegacyValue *frame = _sh_enter(shr, &locals.head, 2);
  locals.head.count = 4;
  locals.t0 = _sh_ljs_undefined();
  locals.t1 = _sh_ljs_undefined();
  locals.t2 = _sh_ljs_undefined();
  locals.t3 = _sh_ljs_undefined();
  SHLegacyValue np0 = _sh_ljs_undefined();

  locals.t0 = _sh_ljs_param(frame, 1);
  locals.t2 = _sh_ljs_dec_rjs(shr, &locals.t0);
  np0 = _sh_ljs_double(1);
  locals.t1 = locals.t0;
  locals.t0 = locals.t1;
  if (_sh_ljs_greater_rjs(shr, &locals.t2, &np0))
    goto L1;
  goto L2;

L1:;
  // PhiInst
  // PhiInst
  locals.t1 = _sh_ljs_mul_rjs(shr, &locals.t1, &locals.t2);
  locals.t2 = _sh_ljs_dec_rjs(shr, &locals.t2);
  locals.t0 = locals.t1;
  if (_sh_ljs_greater_rjs(shr, &locals.t2, &np0))
    goto L1;
  goto L2;

L2:;
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
    {.name_index = 3, .arg_count = 0, .prohibit_invoke = 2, .kind = 0},
    {.name_index = 1, .arg_count = 1, .prohibit_invoke = 2, .kind = 0},
};
static const char s_ascii_pool[] = {
    '\0', 'b', 'e',  'n', 'c', 'h', '\0', 'p', 'r', 'i',
    'n',  't', '\0', 'g', 'l', 'o', 'b',  'a', 'l', '\0',
};
static const char16_t s_u16_pool[] = {};
static const uint32_t s_strings[] = {
    0,
    0,
    0,
    1,
    5,
    540019094,
    7,
    5,
    2794059355,
    13,
    6,
    615793799,
};
#define CREATE_THIS_UNIT sh_export_this_unit
struct UnitData {
  SHUnit unit;
  SHSymbolID symbol_data[4];
  SHPropertyCacheEntry prop_cache_data[4];
  ;
  SHCompressedPointer object_literal_class_cache[0];
};
SHUnit *CREATE_THIS_UNIT(SHRuntime *shr) {
  struct UnitData *unit_data = calloc(sizeof(struct UnitData), 1);
  *unit_data = (struct UnitData){
      .unit = {
          .index = &unit_index,
          .num_symbols = 4,
          .num_prop_cache_entries = 4,
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
