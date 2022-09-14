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

static SHLegacyValue unit_main(SHRuntime *shr) {
  return _sh_ljs_undefined();
}

int main(int argc, char **argv) {
  SHRuntime *shr = _sh_init();
  bool success = _sh_initialize_units(shr, 1, &s_this_unit);
  _sh_done(shr);
  return !success;
}
