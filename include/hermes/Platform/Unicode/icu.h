/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_ICU_H
#define HERMES_SUPPORT_ICU_H

/// This header includes the correct ICU headers: system headers on Linux or
/// externally-supplied on Apple platforms.

#ifdef __APPLE__
/// Prevent appending the ICU version to the namespace, so that we can link
/// properly against libicucore.dylib.
#define U_DISABLE_RENAMING 1
#include "unicode/ucnv.h"
#include "unicode/ucol.h"
#include "unicode/udat.h"
#include "unicode/uloc.h"
#include "unicode/uniset.h"
#include "unicode/unorm2.h"
#include "unicode/ustring.h"
#elif defined(USE_WIN10_ICU)
#define U_DISABLE_RENAMING 1
#define U_SHOW_CPLUSPLUS_API 0
#define U_DEFAULT_SHOW_DRAFT 0
#define U_HIDE_DRAFT_API 1
#define U_HIDE_DEPRECATED_API 1
#define U_HIDE_OBSOLETE_API 1
#define U_HIDE_INTERNAL_API 1
#define U_NO_DEFAULT_INCLUDE_UTF_HEADERS 1
#include "unicode/ucnv.h"
#include "unicode/ucol.h"
#include "unicode/udat.h"
#include "unicode/uloc.h"
#include "unicode/uniset.h"
#include "unicode/unorm2.h"
#include "unicode/ustring.h"
#elif defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
#include <unicode/ucnv.h>
#include <unicode/uloc.h>
#include <unicode/uniset.h>
#include <unicode/unorm2.h>
#include <unicode/ustring.h>
#else
#include <unicode/ucnv.h>
#include <unicode/ucol.h>
#include <unicode/udat.h>
#include <unicode/uloc.h>
#include <unicode/uniset.h>
#include <unicode/unorm2.h>
#include <unicode/ustring.h>
#endif

#endif // HERMES_SUPPORT_ICU_H
