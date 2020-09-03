//===---- Watchdog.cpp - Implement Watchdog ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Watchdog class.
//
//===----------------------------------------------------------------------===//

#include "llvh/Support/Watchdog.h"
#include "llvh/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Watchdog.inc"
#endif
#ifdef _WIN32
#include "Windows/Watchdog.inc"
#endif
