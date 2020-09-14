//===- Memory.cpp - Memory Handling Support ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines some helpful functions for allocating memory and dealing
// with memory mapped files
//
//===----------------------------------------------------------------------===//

#include "llvh/Support/Memory.h"
#include "llvh/Config/llvm-config.h"
#include "llvh/Support/Valgrind.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Memory.inc"
#endif
#ifdef _WIN32
#include "Windows/Memory.inc"
#endif
