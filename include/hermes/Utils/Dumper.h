/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UTILS_DUMPER_H
#define HERMES_UTILS_DUMPER_H

#include <map>

#include "hermes/IR/IRVisitor.h"

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace hermes {
class Value;
class Argument;
class Instruction;
class BasicBlock;
class Function;
class Module;

class CondBranchInst;
class AllocaInst;
class ReturnInst;
class Parameter;
class BranchInst;

/// Display a nice dotty graph that depicts the function.
void viewGraph(Function *F);

/// A utility class for naming instructions. This should only be used for
/// pretty-printing instructions.
struct InstructionNamer {
  InstructionNamer() = default;
  std::map<Value *, unsigned> InstrMap;
  unsigned Counter{0};
  void clear();
  unsigned getNumber(Value *);
};

using llvh::raw_ostream;

struct IRPrinter : public IRVisitor<IRPrinter, void> {
  /// Indentation level.
  unsigned Indent;

  SourceErrorManager &sm_;
  /// Output stream.
  llvh::raw_ostream &os;
  /// If set to true then we need to escape the quote mark because the output of
  /// this printer may be printed as a quoted label.
  bool needEscape;

  InstructionNamer InstNamer;
  InstructionNamer BBNamer;

  explicit IRPrinter(Context &ctx, llvh::raw_ostream &ost, bool escape = false)
      : Indent(0),
        sm_(ctx.getSourceErrorManager()),
        os(ost),
        needEscape(escape) {}

  virtual ~IRPrinter() = default;

  virtual void printFunctionHeader(Function *F);
  virtual void printFunctionVariables(Function *F);
  virtual void printValueLabel(Instruction *I, Value *V, unsigned opIndex);
  virtual void printTypeLabel(Type T);
  virtual void printInstruction(Instruction *I);
  virtual void printInstructionDestination(Instruction *I);
  virtual void printSourceLocation(SMLoc loc);
  virtual void printSourceLocation(SMRange rng);

  std::string getQuoteSign() {
    return needEscape ? R"(\")" : R"(")";
  }

  /// Quote the string if it has spaces.
  std::string quoteStr(StringRef name);

  /// Escapes the string if it has non-printable characters.
  std::string escapeStr(StringRef name);

  /// Declare the functions we are going to reimplement.
  void visitInstruction(const Instruction &I);
  void visitBasicBlock(const BasicBlock &BB);
  void visitFunction(const Function &F);
  void visitModule(const Module &M);
};

} // namespace hermes

#endif
