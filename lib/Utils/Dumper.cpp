/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cctype>
#include <string>

#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/Casting.h"
#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/GraphWriter.h"
#include "llvh/Support/raw_ostream.h"

#include "hermes/AST/Context.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRVisitor.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/Statistic.h"
#include "hermes/Utils/Dumper.h"

using namespace hermes;

using llvh::dyn_cast;
using llvh::isa;

namespace hermes {

std::string IRPrinter::escapeStr(llvh::StringRef name) {
  std::string s = name.str();
  std::string out;
  out += getQuoteSign();
  for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {
    unsigned char c = *i;
    if (std::isprint(c) && c != '\\' && c != '"') {
      out += c;
    } else {
      out += "\\\\";
      switch (c) {
        case '"':
          out += "\\\"";
          break;
        case '\\':
          out += "\\\\";
          break;
        case '\t':
          out += 't';
          break;
        case '\r':
          out += 'r';
          break;
        case '\n':
          out += 'n';
          break;
        default:
          char const *const hexdig = "0123456789ABCDEF";
          out += 'x';
          out += hexdig[c >> 4];
          out += hexdig[c & 0xF];
      }
    }
  }
  out += getQuoteSign();
  return out;
}

std::string IRPrinter::quoteStr(llvh::StringRef name) {
  if (name.count(" ") || name.empty()) {
    return getQuoteSign() + name.str() + getQuoteSign();
  }
  return name.str();
}

void InstructionNamer::clear() {
  Counter = 0;
  InstrMap.clear();
}
unsigned InstructionNamer::getNumber(Value *T) {
  auto It = InstrMap.find(T);
  if (It != InstrMap.end()) {
    return It->second;
  }
  InstrMap[T] = Counter;
  return Counter++;
}

VariableNamer::Name VariableNamer::getName(Variable *var) {
  auto [varIt, newVar] = varMap_.try_emplace(var, 0);
  if (newVar) {
    varIt->second = ++namesCounts_[std::make_pair(
        var->getParent()->getFunction(), var->getName())];
  }

  return Name{var->getName(), varIt->second - 1};
}

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const VariableNamer::Name &n) {
  // If the name is empty or contains spaces, quote it.
  if (n.name.str().count(' ') || n.name.str().empty())
    os << '"' << n.name << '"';
  else
    os << n.name;
  if (n.suffix)
    os << "#" << n.suffix;
  return os;
}

void IRPrinter::printTypeLabel(Value *v) {
  // Don't print the type of basic blocks.
  if (llvh::isa<BasicBlock>(v))
    return;
  os << ": " << v->getType();
}

void IRPrinter::printValueLabel(Instruction *I, Value *V, unsigned opIndex) {
  auto &ctx = I->getContext();
  if (isa<CallBuiltinInst>(I) && opIndex == CallInst::CalleeIdx) {
    os << "["
       << getBuiltinMethodName(cast<CallBuiltinInst>(I)->getBuiltinIndex())
       << "]";
  } else if (isa<GetBuiltinClosureInst>(I) && opIndex == 0) {
    os << "["
       << getBuiltinMethodName(
              cast<GetBuiltinClosureInst>(I)->getBuiltinIndex())
       << "]";
  } else if (auto LBI = dyn_cast<LiteralBigInt>(V)) {
    os << LBI->getValue()->str();
  } else if (auto LS = dyn_cast<LiteralString>(V)) {
    os << escapeStr(ctx.toString(LS->getValue()));
  } else if (auto LB = dyn_cast<LiteralBool>(V)) {
    os << (LB->getValue() ? "true" : "false");
  } else if (auto LN = dyn_cast<LiteralNumber>(V)) {
    const auto Num = LN->getValue();
    if (Num == 0 && std::signbit(Num)) {
      // Ensure we output -0 correctly
      os << "-0";
    } else {
      char buf[NUMBER_TO_STRING_BUF_SIZE];
      numberToString(LN->getValue(), buf, sizeof(buf));
      os << buf;
    }
  } else if (isa<LiteralEmpty>(V)) {
    os << "empty";
  } else if (isa<LiteralNull>(V)) {
    os << "null";
  } else if (isa<LiteralUndefined>(V)) {
    os << "undefined";
  } else if (isa<GlobalObject>(V)) {
    os << "globalObject";
  } else if (isa<EmptySentinel>(V)) {
    os << "empty";
  } else if (isa<Instruction>(V)) {
    os << "%" << InstNamer.getNumber(V);
  } else if (isa<BasicBlock>(V)) {
    os << "%BB" << BBNamer.getNumber(V);
  } else if (auto L = dyn_cast<Label>(V)) {
    auto Name = L->get();
    os << "$" << quoteStr(ctx.toString(Name));
  } else if (auto P = dyn_cast<JSDynamicParam>(V)) {
    auto Name = P->getName();
    os << "%" << ctx.toString(Name);
  } else if (auto F = dyn_cast<Function>(V)) {
    os << "%" << quoteStr(ctx.toString(F->getInternalName())) << "()";
  } else if (auto VS = dyn_cast<VariableScope>(V)) {
    os << "%" << quoteStr(ctx.toString(VS->getFunction()->getInternalName()))
       << "()";
  } else if (auto VR = dyn_cast<Variable>(V)) {
    os << "[" << varNamer_.getName(VR);
    if (I->getParent()->getParent() != VR->getParent()->getFunction()) {
      llvh::StringRef scopeName =
          VR->getParent()->getFunction()->getInternalNameStr();
      os << "@" << quoteStr(scopeName);
    }
    os << "]";
  } else if (auto *NS = llvh::dyn_cast<LiteralNativeSignature>(V)) {
    os << '"';
    NS->getData()->format(os);
    os << '"';
  } else if (auto *NE = llvh::dyn_cast<LiteralNativeExtern>(V)) {
    NativeExtern *ne = NE->getData();
    os << "extern_c(";
    ne->signature()->format(os, ne->name()->c_str());
    if (ne->declared())
      os << " /*declared*/";
    os << ")";
  } else if (auto *shLocal = llvh::dyn_cast<sh::SHLocal>(V)) {
    os << '$' << shLocal->reg();
  } else {
    llvm_unreachable("Invalid value");
  }

  printTypeLabel(V);
}

void IRPrinter::printFunctionHeader(Function *F) {
  auto &Ctx = F->getContext();
  std::string defKindStr = F->getDefinitionKindStr(false);

  os << defKindStr << " " << quoteStr(Ctx.toString(F->getInternalName()))
     << "(";
  uint32_t idx = 0u - 1;
  for (auto *P : F->getJSDynamicParams()) {
    ++idx;
    // Skip "this".
    if (idx == 0)
      continue;
    // Comma before the second param
    if (idx > 1) {
      os << ", ";
    }
    os << P->getName().str();
    printTypeLabel(P);
  }
  os << ")";
  printTypeLabel(F);
  os << " " << F->getAttributes(F->getParent()).getDescriptionStr();
}

void IRPrinter::printFunctionVariables(Function *F) {
  bool first = true;
  os << "frame = [";
  for (auto V : F->getFunctionScope()->getVariables()) {
    if (!first) {
      os << ", ";
    }
    os << varNamer_.getName(V);
    printTypeLabel(V);
    first = false;
  }
  os << "]";
}

void IRPrinter::printInstructionDestination(Instruction *I) {
  os << "%" << InstNamer.getNumber(I);
}

void IRPrinter::printInstruction(Instruction *I) {
  printInstructionDestination(I);
  os << " = ";
  os << I->getName();

  if (!I->getAttributes(I->getModule()).isEmpty()) {
    os << " " << I->getAttributes(I->getModule()).getDescriptionStr();
  }

  // Don't print the type of instructions without output.
  if (I->hasOutput()) {
    os << " (:" << I->getType() << ")";
  }

  bool first = true;

  for (int i = 0, e = I->getNumOperands(); i < e; i++) {
    os << (first ? " " : ", ");
    printValueLabel(I, I->getOperand(i), i);
    first = false;
  }

  auto codeGenOpts = I->getContext().getCodeGenerationSettings();
  // Print the use list if there is any user for the instruction.
  if (!codeGenOpts.dumpUseList || I->getUsers().empty())
    return;

  llvh::DenseSet<Instruction *> Visited;
  os << " // users:";
  for (auto &U : I->getUsers()) {
    auto *II = cast<Instruction>(U);
    assert(II && "Expecting user to be an Instruction");
    if (Visited.find(II) != Visited.end())
      continue;
    Visited.insert(II);
    os << " %" << InstNamer.getNumber(II);
  }
}

void IRPrinter::printSourceLocation(SMLoc loc) {
  SourceErrorManager::SourceCoords coords;
  if (!sm_.findBufferLineAndLoc(loc, coords))
    return;

  os << sm_.getSourceUrl(coords.bufId) << ":" << coords.line << ":"
     << coords.col;
}

void IRPrinter::printSourceLocation(SMRange rng) {
  SourceErrorManager::SourceCoords start, end;
  if (!sm_.findBufferLineAndLoc(rng.Start, start) ||
      !sm_.findBufferLineAndLoc(rng.End, end))
    return;

  os << "[" << sm_.getSourceUrl(start.bufId) << ":" << start.line << ":"
     << start.col << " ... " << sm_.getSourceUrl(end.bufId) << ":" << end.line
     << ":" << end.col << ")";
}

void IRPrinter::visitModule(const Module &M) {
  // Use IRVisitor dispatch to visit each individual function.
  for (auto &F : M)
    visit(F);
}

void IRPrinter::visitFunction(const Function &F) {
  auto *UF = const_cast<Function *>(&F);
  os.indent(Indent);
  BBNamer.clear();
  InstNamer.clear();
  // Number all instructions sequentially.
  for (auto &BB : *UF)
    for (auto &I : BB)
      InstNamer.getNumber(&I);

  printFunctionHeader(UF);
  os << "\n";
  printFunctionVariables(UF);
  os << "\n";

  auto codeGenOpts = F.getContext().getCodeGenerationSettings();
  if (codeGenOpts.dumpSourceLocation) {
    os << "source location: ";
    printSourceLocation(F.getSourceRange());
    os << "\n";
  }

  // Use IRVisitor dispatch to visit the basic blocks.
  for (auto &BB : F) {
    visit(BB);
  }

  os.indent(Indent);
  os << "function_end"
     << "\n";
  os << "\n";
}

void IRPrinter::visitBasicBlock(const BasicBlock &BB) {
  auto *UBB = const_cast<BasicBlock *>(&BB);
  os.indent(Indent);
  os << "%BB" << BBNamer.getNumber(UBB) << ":\n";
  Indent += 2;

  // Use IRVisitor dispatch to visit the instructions.
  for (auto &I : BB)
    visit(I);

  Indent -= 2;
}

void IRPrinter::visitInstruction(const Instruction &I) {
  auto *UII = const_cast<Instruction *>(&I);
  auto codeGenOpts = I.getContext().getCodeGenerationSettings();
  if (codeGenOpts.dumpSourceLocation) {
    os << "; ";
    printSourceLocation(UII->getLocation());
    os << "\n";
  }
  os.indent(Indent);
  printInstruction(UII);
  os << "\n";
}

} // namespace hermes

namespace {

/// This class prints Functions into dotty graphs. This struct inherits the
/// IRVisitor and reimplement the visitFunction and visitBasicBlock function.
struct DottyPrinter : public IRVisitor<DottyPrinter, void> {
  llvh::raw_ostream &os;
  llvh::SmallVector<std::pair<std::string, std::string>, 4> Edges;
  IRPrinter Printer;

  explicit DottyPrinter(
      Context &ctx,
      llvh::raw_ostream &ost,
      llvh::StringRef Title)
      : os(ost), Printer(ctx, ost, /* escape output */ true) {
    os << " digraph g {\n graph [ rankdir = \"TD\" ];\n";
    os << "labelloc=\"t\"; ";
    os << " node [ fontsize = \"16\" shape = \"record\" ]; edge [ ];\n";
  }

  ~DottyPrinter() {
    os << "\n";

    // Print the edges between the blocks.
    for (auto T : Edges) {
      os << "" << T.first << " ->";
      os << "" << T.second << ";\n";
    }
    os << "\n}\n";
  }

  /// Convert pointers into unique textual ids.
  static std::string toString(BasicBlock *ptr) {
    auto Num = (size_t)ptr;
    return std::to_string(Num);
  }

  /// Reimplement the visitFunction in IRVisitor.
  void visitFunction(const Function &F) {
    os << "label=\"";
    Printer.printFunctionHeader(const_cast<Function *>(&F));
    os << "\";\n";

    // Pre-assign every instruction a number, by doing this we avoid
    // assigning non-consecutive numbers to consecutive instructions if we
    // are dumping user list. Its also not a bad practice to initialize the
    // InstNamer table before we dump all the instructions.
    for (auto &BB : F) {
      for (auto &II : BB) {
        (void)Printer.InstNamer.getNumber(const_cast<Instruction *>(&II));
      }
    }

    // Use IRVisitor dispatch to visit the basic blocks.
    for (auto &BB : F) {
      visit(BB);
    }
  }

  /// Reimplement the visitBasicBlock in the IRVisitor.
  /// Visit all of the basic blocks in the program and render them into dotty
  /// records. Save edges between basic blocks and print them when we finish
  /// visiting all of the blocks in the program.
  void visitBasicBlock(const BasicBlock &V) {
    auto *BB = const_cast<BasicBlock *>(&V);
    os << "\"" << toString(BB) << "\"";
    os << "[ label = \"{ ";

    os << " { <self> | <head> \\<\\<%BB" << Printer.BBNamer.getNumber(BB)
       << "\\>\\> | } ";

    int counter = 0;

    // For each instruction in the basic block:
    for (auto &I : *BB) {
      counter++;
      os << " | ";

      // Print the instruction.
      os << "<L" << counter << ">";
      Printer.printInstruction(&I);
    }

    for (auto I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
      auto From = toString(BB) + ":L" + std::to_string(counter);
      Edges.push_back({From, toString(*I) + (*I == BB ? ":self" : ":head")});
    }

    // Emit the record:
    os << "}\" shape = \"record\" ]; \n";
  }
};

} // anonymous namespace

void hermes::viewGraph(Function *F) {
#ifndef NDEBUG
  auto &Ctx = F->getContext();
  llvh::StringRef Name = Ctx.toString(F->getInternalName());
  int FD;
  // Windows can't always handle long paths, so limit the length of the name.
  std::string N = Name.str();
  N = N.substr(0, std::min<std::size_t>(N.size(), 140));
  std::string Filename = llvh::createGraphFilename(N, FD);
  {
    llvh::raw_fd_ostream O(FD, /*shouldClose=*/true);

    if (FD == -1) {
      llvh::errs() << "error opening file '" << Filename << "' for writing!\n";
      return;
    }

    DottyPrinter D(F->getContext(), O, Name);
    D.visitFunction(*F);
  }

  llvh::DisplayGraph(Filename);
  llvh::errs() << " done. \n";
#endif
}
