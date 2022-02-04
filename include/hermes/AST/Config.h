/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_CONFIG_H
#define HERMES_PARSER_CONFIG_H

#if !defined(HERMES_PARSE_JSX)
#if defined(HERMES_IS_MOBILE_BUILD)
#define HERMES_PARSE_JSX 0
#else
#define HERMES_PARSE_JSX 1
#endif
#endif

#if !defined(HERMES_PARSE_FLOW)
#if defined(HERMES_IS_MOBILE_BUILD)
#define HERMES_PARSE_FLOW 0
#else
#define HERMES_PARSE_FLOW 1
#endif
#endif

#if !defined(HERMES_PARSE_TS)
#if defined(HERMES_IS_MOBILE_BUILD)
#define HERMES_PARSE_TS 0
#else
#define HERMES_PARSE_TS 1
#endif
#endif

#endif
