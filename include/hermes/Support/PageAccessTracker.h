/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_HAVE_LIBSIGMUX
#define HERMES_HAS_REAL_PAGE_TRACKER
#include "hermes/Support/PageAccessTrackerPosix.h"
#else
#include "hermes/Support/PageAccessTrackerEmpty.h"
#endif
