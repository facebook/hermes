/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../RuntimeState.h"
#include "hermes/hermes.h"

namespace facebook {
jsi::Value bufferBinding(RuntimeState &rs);
jsi::Value constantsBinding(RuntimeState &rs);
jsi::Value fsBinding(RuntimeState &rs);
jsi::Value utilBinding(RuntimeState &rs);
jsi::Value ttyBinding(RuntimeState &rs);
jsi::Value pipeBinding(RuntimeState &rs);
} // namespace facebook
