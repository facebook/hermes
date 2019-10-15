/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(HERMES_FACEBOOK_BUILD) && defined(__linux__)
#include "hermes/Support/PageAccessTrackerPosix.h"
#else
#include "hermes/Support/PageAccessTrackerEmpty.h"
#endif
