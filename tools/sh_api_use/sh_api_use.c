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

int main(int argc, char **argv) {
  SHRuntime *shr = _sh_init();
  _sh_done(shr);
  return 0;
}
