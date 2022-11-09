//===- TableGen.cpp - Top-Level TableGen implementation for LLVM ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the main function for LLVM's TableGen.
//
//===----------------------------------------------------------------------===//

#include "TableGenBackends.h" // Declares all backends.
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/ManagedStatic.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Signals.h"
#include "llvh/TableGen/Main.h"
#include "llvh/TableGen/Record.h"
#include "llvh/TableGen/SetTheory.h"

using namespace llvh;

enum ActionType {
  PrintRecords,
  DumpJSON,
  PrintEnums,
  PrintSets,
  GenExample,
};

namespace llvh {
/// Storage for TimeRegionsOpt as a global so that backends aren't required to
/// include CommandLine.h
bool TimeRegions = false;
} // end namespace llvh

namespace {
  cl::opt<ActionType>
  Action(cl::desc("Action to perform:"),
         cl::values(clEnumValN(PrintRecords, "print-records",
                               "Print all records to stdout (default)"),
                    clEnumValN(DumpJSON, "dump-json",
                               "Dump all records as machine-readable JSON"),
                    clEnumValN(PrintEnums, "print-enums",
                               "Print enum values for a class"),
                    clEnumValN(PrintSets, "print-sets",
                               "Print expanded sets for testing DAG exprs"),
                    clEnumValN(GenExample, "gen-example",
                               "Generate example")));

  cl::OptionCategory PrintEnumsCat("Options for -print-enums");
  cl::opt<std::string>
  Class("class", cl::desc("Print Enum list for this class"),
        cl::value_desc("class name"), cl::cat(PrintEnumsCat));

cl::opt<bool, true>
    TimeRegionsOpt("time-regions",
                   cl::desc("Time regions of tablegens execution"),
                   cl::location(TimeRegions));

bool LLVMTableGenMain(raw_ostream &OS, RecordKeeper &Records) {
  switch (Action) {
  case PrintRecords:
    OS << Records;           // No argument, dump all contents
    break;
  case DumpJSON:
    EmitJSON(Records, OS);
    break;
  case PrintEnums:
  {
    for (Record *Rec : Records.getAllDerivedDefinitions(Class))
      OS << Rec->getName() << ", ";
    OS << "\n";
    break;
  }
  case PrintSets:
  {
    SetTheory Sets;
    Sets.addFieldExpander("Set", "Elements");
    for (Record *Rec : Records.getAllDerivedDefinitions("Set")) {
      OS << Rec->getName() << " = [";
      const std::vector<Record*> *Elts = Sets.expand(Rec);
      assert(Elts && "Couldn't expand Set instance");
      for (Record *Elt : *Elts)
        OS << ' ' << Elt->getName();
      OS << " ]\n";
    }
    break;
  }
  case GenExample:
    EmitExample(Records, OS);
    break;
  }

  return false;
}
}

int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  llvm_shutdown_obj Y;

  return TableGenMain(argv[0], &LLVMTableGenMain);
}

#ifdef __has_feature
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
// Disable LeakSanitizer for this binary as it has too many leaks that are not
// very interesting to fix. See compiler-rt/include/sanitizer/lsan_interface.h .
LLVM_ATTRIBUTE_USED int __lsan_is_turned_off() { return 1; }
#endif  // __has_feature(address_sanitizer)
#endif  // defined(__has_feature)
