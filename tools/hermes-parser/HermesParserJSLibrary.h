/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HERMESPARSER_HERMESPARSERJSLIBRARY_H
#define HERMES_TOOLS_HERMESPARSER_HERMESPARSERJSLIBRARY_H

using JSReference = int;

using NodeLabel = const char *;
using NodeBoolean = bool;
using NodeNumber = double;
using NodePtr = JSReference;
using NodeList = JSReference;

/// Definitions of JS Library functions that can be called from WASM

extern "C" {
JSReference buildArray();
void appendToArray(JSReference, JSReference);
JSReference buildSourceLocation(int, int, int, int, int, int);
JSReference buildComment(JSReference, int, const char *, size_t);
JSReference buildProgramWithComments(JSReference, JSReference, JSReference);
}

/// Builder function for each node in the AST

#define ESTREE_NODE_0_ARGS(NAME, ...) JSReference build_##NAME(JSReference);
#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  JSReference build_##NAME(JSReference, ARG0TY);
#define ESTREE_NODE_2_ARGS(                                      \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1PT) \
  JSReference build_##NAME(JSReference, ARG0TY, ARG1TY);
#define ESTREE_NODE_3_ARGS( \
    NAME,                   \
    BASE,                   \
    ARG0TY,                 \
    ARG0NM,                 \
    ARG0OPT,                \
    ARG1TY,                 \
    ARG1NM,                 \
    ARG1PT,                 \
    ARG2TY,                 \
    ARG2NM,                 \
    ARG2PT)                 \
  JSReference build_##NAME(JSReference, ARG0TY, ARG1TY, ARG2TY);
#define ESTREE_NODE_4_ARGS( \
    NAME,                   \
    BASE,                   \
    ARG0TY,                 \
    ARG0NM,                 \
    ARG0OPT,                \
    ARG1TY,                 \
    ARG1NM,                 \
    ARG1PT,                 \
    ARG2TY,                 \
    ARG2NM,                 \
    ARG2PT,                 \
    ARG3TY,                 \
    ARG3NM,                 \
    ARG3PT)                 \
  JSReference build_##NAME(JSReference, ARG0TY, ARG1TY, ARG2TY, ARG3TY);
#define ESTREE_NODE_5_ARGS( \
    NAME,                   \
    BASE,                   \
    ARG0TY,                 \
    ARG0NM,                 \
    ARG0OPT,                \
    ARG1TY,                 \
    ARG1NM,                 \
    ARG1PT,                 \
    ARG2TY,                 \
    ARG2NM,                 \
    ARG2PT,                 \
    ARG3TY,                 \
    ARG3NM,                 \
    ARG3PT,                 \
    ARG4TY,                 \
    ARG4NM,                 \
    ARG4PT)                 \
  JSReference build_##NAME(JSReference, ARG0TY, ARG1TY, ARG2TY, ARG3TY, ARG4TY);
#define ESTREE_NODE_6_ARGS( \
    NAME,                   \
    BASE,                   \
    ARG0TY,                 \
    ARG0NM,                 \
    ARG0OPT,                \
    ARG1TY,                 \
    ARG1NM,                 \
    ARG1PT,                 \
    ARG2TY,                 \
    ARG2NM,                 \
    ARG2PT,                 \
    ARG3TY,                 \
    ARG3NM,                 \
    ARG3PT,                 \
    ARG4TY,                 \
    ARG4NM,                 \
    ARG4PT,                 \
    ARG5TY,                 \
    ARG5NM,                 \
    ARG5PT)                 \
  JSReference build_##NAME( \
      JSReference, ARG0TY, ARG1TY, ARG2TY, ARG3TY, ARG4TY, ARG5TY);
#define ESTREE_NODE_7_ARGS( \
    NAME,                   \
    BASE,                   \
    ARG0TY,                 \
    ARG0NM,                 \
    ARG0OPT,                \
    ARG1TY,                 \
    ARG1NM,                 \
    ARG1PT,                 \
    ARG2TY,                 \
    ARG2NM,                 \
    ARG2PT,                 \
    ARG3TY,                 \
    ARG3NM,                 \
    ARG3PT,                 \
    ARG4TY,                 \
    ARG4NM,                 \
    ARG4PT,                 \
    ARG5TY,                 \
    ARG5NM,                 \
    ARG5PT,                 \
    ARG6TY,                 \
    ARG6NM,                 \
    ARG6PT)                 \
  JSReference build_##NAME( \
      JSReference, ARG0TY, ARG1TY, ARG2TY, ARG3TY, ARG4TY, ARG5TY, ARG6TY);
#define ESTREE_NODE_8_ARGS( \
    NAME,                   \
    BASE,                   \
    ARG0TY,                 \
    ARG0NM,                 \
    ARG0OPT,                \
    ARG1TY,                 \
    ARG1NM,                 \
    ARG1PT,                 \
    ARG2TY,                 \
    ARG2NM,                 \
    ARG2PT,                 \
    ARG3TY,                 \
    ARG3NM,                 \
    ARG3PT,                 \
    ARG4TY,                 \
    ARG4NM,                 \
    ARG4PT,                 \
    ARG5TY,                 \
    ARG5NM,                 \
    ARG5PT,                 \
    ARG6TY,                 \
    ARG6NM,                 \
    ARG6PT,                 \
    ARG7TY,                 \
    ARG7NM,                 \
    ARG7PT)                 \
  JSReference build_##NAME( \
      JSReference,          \
      ARG0TY,               \
      ARG1TY,               \
      ARG2TY,               \
      ARG3TY,               \
      ARG4TY,               \
      ARG5TY,               \
      ARG6TY,               \
      ARG7TY);

extern "C" {
#include "hermes/AST/ESTree.def"
}

#endif // HERMES_TOOLS_HBCDUMP_PROFILEANALYZER_H
