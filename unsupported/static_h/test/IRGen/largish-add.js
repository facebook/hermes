/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( echo "0" "+"{1..29999} | %hermesc -O0 -dump-ir - >/dev/null )
