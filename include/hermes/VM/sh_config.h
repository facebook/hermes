/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SH_CONFIG_H
#define HERMES_SH_CONFIG_H

#include "libhermesvm-config.h"

#ifndef __cplusplus
// uchar.h is not universally available, so just define our own.
typedef uint16_t char16_t;
#endif

#ifndef SHERMES_EXPORT
#ifdef _MSC_VER
#define SHERMES_EXPORT __declspec(dllexport)
#else // _MSC_VER
#define SHERMES_EXPORT __attribute__((visibility("default")))
#endif // _MSC_VER
#endif // !defined(HERMES_EXPORT)

#endif
