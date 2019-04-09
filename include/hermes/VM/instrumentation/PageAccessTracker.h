/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#if defined(HERMES_FACEBOOK_BUILD) && !defined(_WINDOWS)
#include "hermes/VM/instrumentation/PageAccessTrackerPosix.h"
#else
#include "hermes/VM/instrumentation/PageAccessTrackerEmpty.h"
#endif
