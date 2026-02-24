/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Patch to avoid Compiler Warning (level 2) C4275
#ifndef HERMES_EXPORT
#ifdef CREATE_SHARED_LIBRARY
#ifdef _MSC_VER
#define HERMES_EXPORT __declspec(dllexport)
#else // _MSC_VER
#define HERMES_EXPORT __attribute__((visibility("default")))
#endif // _MSC_VER
#else // CREATE_SHARED_LIBRARY
#define HERMES_EXPORT
#endif // CREATE_SHARED_LIBRARY
#endif // !defined(HERMES_EXPORT)
