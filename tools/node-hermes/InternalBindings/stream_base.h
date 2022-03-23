/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../RuntimeState.h"
#include "hermes/hermes.h"

namespace facebook {
/// Given the data argument, initializes a uv_buf and
/// writes to the stream associated with the object this
/// function is defined on.
int streamBaseWriteUtf8String(
    RuntimeState &rs,
    const jsi::Value &arg,
    uv_stream_t *stream);
} // namespace facebook
