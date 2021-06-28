/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../RuntimeState.h"
#include "hermes/hermes.h"

namespace facebook {
jsi::Value bufferBinding(jsi::Runtime &rt, RuntimeState &rs);
jsi::Value constantsBinding(jsi::Runtime &rt, RuntimeState &rs);
} // namespace facebook
