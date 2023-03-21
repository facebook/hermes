/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/SH/SH.h"

#include "LowerCalls.h"
#include "RecreateCheapValues.h"
#include "SHRegAlloc.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/BCGen/LowerBuiltinCalls.h"
#include "hermes/BCGen/LowerStoreInstrs.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/HashString.h"
#include "hermes/Support/UTF8.h"

#include "llvh/ADT/BitVector.h"

using namespace hermes;

namespace {
/// Generates the correct label for BasicBlock \p B based on \p bbMap and
/// outputs it through \p OS.
void generateBasicBlockLabel(
    BasicBlock *B,
    llvh::raw_ostream &OS,
    const llvh::DenseMap<BasicBlock *, unsigned> &bbMap) {
  OS << "L" << bbMap.find(B)->second;
}

const char *boolStr(bool b) {
  return b ? "true" : "false";
}

/// Helper to unique and store the contents of strings.
class SHStringTable {
  StringSetVector strings_;

  /// Append a printable representation of the character \p c, that when
  /// enclosed in single quotes in a C file, will produce the same character.
  /// NOTE: We deliberately print each character separately, since this emits a
  /// mix of printable characters and hexadecimal escapes, which consume all
  /// subsequent hexadecimal characters. For example, consider emitting the
  /// escape '\x0' followed by the character '1', if we emitted them in a single
  /// string literal, the compiler would parse them as the single escape '\x01'.
  static void appendEscaped(std::string &out, char16_t c) {
    // To improve the readability of the generated source, print certain
    // characters directly. We consider the character printable if it is ASCII,
    // returns true for isprint, and is not a ' or a \.
    bool canPrint = c <= 127 && std::isprint(c) && c != u'\'' && c != u'\\';
    if (canPrint) {
      out += c;
    } else {
      // If the character is not printable, emit a hexadecimal escape sequence
      // for it.
      out += "\\x";
      const char *hexdig = "0123456789ABCDEF";
      out += hexdig[c >> 12];
      out += hexdig[(c >> 8) & 0xF];
      out += hexdig[(c >> 4) & 0xF];
      out += hexdig[c & 0xF];
    }
  }

 public:
  /// \return the current number of strings in the table.
  uint32_t size() const {
    return strings_.size();
  }

  /// Add the string \p str to the table if it is not already in it.
  /// \return the index associated with str.
  uint32_t add(llvh::StringRef str) {
    return strings_.insert(str);
  }

  /// Turn the table of strings into the SH C data structures that are necessary
  /// to provide strings for the module.
  void generate(llvh::raw_ostream &os) const {
    struct StringEntry {
      uint32_t offset, length, hash;
    };
    std::vector<StringEntry> stringEntries;
    stringEntries.reserve(strings_.size());

    // Buffers containing the escaped strings, which can be emitted directly
    // into the C code.
    std::string asciiStr, u16Str;
    // The number of characters emitted so far in each of the buffers.
    uint32_t asciiOffset = 0, u16Offset = 0;

    for (const auto &str : strings_) {
      StringEntry entry;
      if (isAllASCII(str.begin(), str.end())) {
        asciiStr += "  ";
        // The given string is entirely ASCII. Emit the sequence of ASCII
        // characters to asciiStr.
        for (char c : str) {
          asciiStr += '\'';
          appendEscaped(asciiStr, c);
          asciiStr += "', ";
        }
        // Strings are null terminated to make them easier to debug.
        asciiStr += "'\\0',\n";
        entry = {
            asciiOffset,
            (uint32_t)str.size(),
            hashString(llvh::ArrayRef<char>{str.data(), str.size()})};
        asciiOffset += str.size() + 1;
      } else {
        u16Str += "  ";
        // If the string is non-ASCII, convert it to UTF-16, and then perform
        // the same steps as the ASCII string above.
        std::u16string strStorage;
        convertUTF8WithSurrogatesToUTF16(
            std::back_inserter(strStorage),
            str.data(),
            str.data() + str.size());
        for (char16_t c : strStorage) {
          u16Str += "u'";
          appendEscaped(u16Str, c);
          u16Str += "', ";
        }
        u16Str += "u'\\0',\n";
        entry = {
            u16Offset | (1 << 31),
            (uint32_t)strStorage.size(),
            hashString(llvh::ArrayRef<char16_t>{
                strStorage.data(), strStorage.size()})};
        u16Offset += strStorage.size() + 1;
      }
      stringEntries.push_back(entry);
    }

    // Example:
    // static const char s_ascii_pool[] = {
    //   'H', 'e', 'l', 'l', 'o', '\0',
    //   'W', 'o', 'r', 'l', 'd', '\0',
    // };
    // static const char16_t s_u16_pool[] = {
    // };
    // static SHSymbolID s_symbols[2];
    // static const uint32_t s_strings[] = {
    //     0, 5, 0, 6, 5, 0,
    // };

    os << "static const char s_ascii_pool[] = {\n"
       << asciiStr << "};\n"
       << "static const char16_t s_u16_pool[] = {\n"
       << u16Str << "};\n"
       << "static SHSymbolID s_symbols[" << size() << "];\n"
       << "static const uint32_t s_strings[] = {";
    for (const auto &entry : stringEntries)
      os << entry.offset << "," << entry.length << "," << entry.hash << ",";
    os << "};\n";
  }
};

class SHLiteralBuffers {
  hbc::SerializedLiteralGenerator literalGenerator_;

 public:
  /// Table of constants used to initialize constant arrays.
  /// They are stored as chars in order to shorten bytecode size.
  std::vector<unsigned char> arrayBuffer{};

  /// Table of constants used to initialize object keys.
  /// They are stored as chars in order to shorten bytecode size
  std::vector<unsigned char> objKeyBuffer{};

  /// Table of constants used to initialize object values.
  /// They are stored as chars in order to shorten bytecode size
  std::vector<unsigned char> objValBuffer{};

  explicit SHLiteralBuffers(SHStringTable &stringTable)
      : literalGenerator_(
            [&stringTable](llvh::StringRef str) {
              return stringTable.add(str);
            },
            [&stringTable](llvh::StringRef str) {
              return stringTable.add(str);
            },
            true) {}

  /// Returns the starting offset of the elements.
  uint32_t addArrayBuffer(ArrayRef<Literal *> elements) {
    return literalGenerator_.serializeBuffer(elements, arrayBuffer, false);
  }

  /// Add to the the object buffer using \keys as the array of keys, and
  /// \vals as the array of values.
  /// Returns a pair where the first value is the object's offset into the
  /// key buffer, and the second value is its offset into the value buffer.
  std::pair<uint32_t, uint32_t> addObjectBuffer(
      ArrayRef<Literal *> keys,
      ArrayRef<Literal *> vals) {
    return std::pair<uint32_t, uint32_t>{
        literalGenerator_.serializeBuffer(keys, objKeyBuffer, true),
        literalGenerator_.serializeBuffer(vals, objValBuffer, false)};
  }

  void generate(llvh::raw_ostream &os) const {
    generateBuffer(os, "s_obj_key_buffer", objKeyBuffer);
    generateBuffer(os, "s_obj_val_buffer", objValBuffer);
    generateBuffer(os, "s_array_buffer", arrayBuffer);
  }

 private:
  void generateBuffer(
      llvh::raw_ostream &os,
      llvh::StringRef name,
      llvh::ArrayRef<unsigned char> buf) const {
    os << "static unsigned char " << name << "[" << buf.size() << "] = {";
    for (unsigned char c : buf) {
      os << (int)c << ",";
    }
    os << "};\n";
  }
};

struct ModuleGen {
  /// Table containing uniqued strings for the current module.
  SHStringTable stringTable{};

  /// A map from functions to unique numbers for identificatio
  llvh::DenseMap<Function *, unsigned> funcMap{};

  /// Literal buffers for objects and arrays.
  SHLiteralBuffers literalBuffers;

  explicit ModuleGen() : literalBuffers{stringTable} {}

  /// Generates the correct label for BasicBlock \p B based on \p bbMap and
  /// outputs it through \p OS. If the JS function name contains characters
  /// that aren't allowed in C identifiers, they will be replaced by '_'.
  void generateFunctionLabel(Function *F, llvh::raw_ostream &OS) {
    OS << '_' << funcMap.find(F)->second << '_';

    auto name = F->getInternalNameStr();
    for (auto c : name) {
      if (('0' <= c && c <= '9') || ('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z')) {
        OS << c;
      } else {
        // Replace illegal identifier characters with '_'.
        OS << '_';
      }
    }
  }
};

class InstrGen {
 public:
  /// \p os is the output stream
  /// \p ra is the pre-ran register allocator for the current function
  /// \p envSize is the environment size of the current function
  InstrGen(
      llvh::raw_ostream &os,
      sh::SHRegisterAllocator &ra,
      const llvh::DenseMap<BasicBlock *, unsigned> &bbMap,
      Function &F,
      ModuleGen &moduleGen,
      FunctionScopeAnalysis &scopeAnalysis,
      unsigned envSize,
      uint32_t &nextCacheIdx)
      : os_(os),
        ra_(ra),
        bbMap_(bbMap),
        F_(F),
        moduleGen_(moduleGen),
        scopeAnalysis_(scopeAnalysis),
        envSize_(envSize),
        nextCacheIdx_(nextCacheIdx) {
    registerIsPointer_.resize(ra_.getMaxRegisterUsage());
    if (F_.getContext().getOptimizationSettings().promoteNonPtr) {
      // Default every register to not be a pointer.
      registerIsPointer_.reset();
      // Check each register to see if it's a non-pointer.
      for (auto &BB : F_) {
        for (auto &I : BB) {
          if (ra_.isAllocated(&I) && I.hasOutput()) {
            sh::Register reg = ra_.getRegister(&I);
            if (reg.getClass() != hermes::sh::RegClass::Local)
              continue;
            if (!I.getType().isNonPtr()) {
              // Set the bit if I sets the register to a pointer type.
              // Numbers might be pointers, but the stack doesn't use HV32.
              registerIsPointer_.set(reg.getIndex());
            }
          }
        }
      }
    } else {
      // Optimization is not enabled, everything is a pointer.
      registerIsPointer_.set();
    }
  }

  /// Converts Instruction \p I into valid C code and outputs it through the
  /// ostream.
  void generate(Instruction &I) {
    switch (I.getKind()) {
#define INCLUDE_HBC_INSTRS
#define DEF_VALUE(CLASS, PARENT) \
  case ValueKind::CLASS##Kind:   \
    return generate##CLASS(*cast<CLASS>(&I));
#define DEF_TAG(NAME, PARENT) \
  case ValueKind::NAME##Kind: \
    return generate##PARENT(*cast<PARENT>(&I));
#include "hermes/IR/Instrs.def"
      default:
        llvm_unreachable("Invalid kind");
    }
  }

  bool registerIsPointer(uint32_t r) const {
    return registerIsPointer_[r];
  }

 private:
  /// The ostream used to output C code
  llvh::raw_ostream &os_;

  /// The register allocator that was created for the current function
  sh::SHRegisterAllocator &ra_;

  /// A map from basic blocks to unique numbers for identification
  const llvh::DenseMap<BasicBlock *, unsigned> &bbMap_;

  // The function being compiled.
  Function &F_;

  /// The state for the module currently being emitted.
  ModuleGen &moduleGen_;

  /// Function scope analysis of the current module
  FunctionScopeAnalysis &scopeAnalysis_;

  /// The size of this functions environment
  unsigned envSize_;

  /// Starts out at 0 and increments every time a cache index is used
  uint32_t &nextCacheIdx_;

  /// Indices used to generate unique names for the jump buffers.
  uint32_t nextJBufIdx_{0};

  /// Entry \c i is true if a pointer is ever written to register \c i.
  llvh::BitVector registerIsPointer_{};

  void unimplemented(Instruction &inst) {
    std::string err{"Unimplemented "};
    err += inst.getName();
    F_.getParent()->getContext().getSourceErrorManager().error(
        inst.getLocation(), err);
    // This can optionally be disabled.
    hermes_fatal(err);
  }

  /// Helper to generate a value in a register,
  void generateRegister(sh::Register reg) {
    if (reg.getClass() == sh::RegClass::Local) {
      if (registerIsPointer(reg.getIndex()))
        os_ << "locals.t" << reg.getIndex();
      else
        os_ << "r" << reg.getIndex();
    } else {
      os_ << "frame[" << (hbc::StackFrameLayout::FirstLocal + reg.getIndex())
          << ']';
    }
  }

  /// Helper to generate a value that must always have an allocated register,
  /// for instance because we need to assign to it or take its address.
  void generateRegister(Value &val) {
    generateRegister(ra_.getRegister(&val));
  }

  /// Helper to generate a pointer to a value that must always have an allocated
  /// register, for instance because we need to assign to it or take its
  /// address.
  /// The pointer can be used to pass to API functions that will use it
  /// as a PinnedHermesValue.
  void generateRegisterPtr(Value &val) {
    os_ << "&";
    generateRegister(val);
  }

  /// Helper to generate an SHValue from a Value.
  void generateValue(Value &val) {
    if (llvh::isa<LiteralUndefined>(&val)) {
      os_ << "_sh_ljs_undefined()";
    } else if (llvh::isa<LiteralNull>(&val)) {
      os_ << "_sh_ljs_null()";
    } else if (llvh::isa<LiteralEmpty>(&val)) {
      os_ << "_sh_ljs_empty()";
    } else if (auto B = llvh::dyn_cast<LiteralBool>(&val)) {
      os_ << "_sh_ljs_bool(" << boolStr(B->getValue()) << ")";
    } else if (auto LN = llvh::dyn_cast<LiteralNumber>(&val)) {
      os_ << "_sh_ljs_double(";
      if (!LN->isNegativeZero() &&
          LN->getValue() == unsafeTruncateDouble<int>(LN->getValue())) {
        os_ << static_cast<int>(LN->getValue());
      } else {
        os_ << "((struct HermesValueBase){.raw = "
            << llvh::DoubleToBits(LN->getValue()) << "u}).f64";
      }
      os_ << ")";
    } else if (auto S = llvh::dyn_cast<LiteralString>(&val)) {
      os_ << "_sh_ljs_get_string(shr, s_symbols["
          << moduleGen_.stringTable.add(S->getValue().str()) << "])";
    } else if (auto *LBI = llvh::dyn_cast<LiteralBigInt>(&val)) {
      auto parsedBigInt = bigint::ParsedBigInt::parsedBigIntFromNumericValue(
          LBI->getValue()->str());
      llvh::ArrayRef<uint8_t> bytes = parsedBigInt->getBytes();
      assert(parsedBigInt && "should be valid");
      os_ << "_sh_ljs_create_bigint(shr, ";
      // Print a string literal using hex escapes for each byte.
      // Cast it to `uint8_t*` for use with the API signature.
      os_ << "(const uint8_t *)\"";
      for (const uint8_t c : bytes) {
        os_ << llvh::format("\\x%x", c);
      }
      os_ << "\", ";
      os_ << bytes.size();
      os_ << ")";
    } else if (llvh::isa<Instruction>(&val)) {
      generateRegister(val);
    } else {
      hermes_fatal("Unknown value");
    }
  }

  void generateInstruction(Instruction &inst) {
    hermes_fatal("Unimplemented instruction Instruction");
  }
  void generateAddEmptyStringInst(AddEmptyStringInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    os_ << "_sh_ljs_add_empty_string_rjs(shr, ";
    generateRegisterPtr(*inst.getSingleOperand());
    os_ << ");\n";
  }
  void generateAsNumberInst(AsNumberInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    os_ << "_sh_ljs_double(_sh_ljs_to_double_rjs(shr, ";
    generateRegisterPtr(*inst.getSingleOperand());
    os_ << "));\n";
  }
  void generateAsNumericInst(AsNumericInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    os_ << "_sh_ljs_to_numeric_rjs(shr, ";
    generateRegisterPtr(*inst.getSingleOperand());
    os_ << ");\n";
  }
  void generateAsInt32Inst(AsInt32Inst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    if (inst.getSingleOperand()->getType().isNumberType()) {
      os_ << "_sh_ljs_double((double)_sh_to_int32_double(_sh_ljs_get_double(";
      generateRegister(*inst.getSingleOperand());
      os_ << ")));\n";
    } else {
      os_ << "_sh_ljs_to_int32_rjs(shr, ";
      generateRegisterPtr(*inst.getSingleOperand());
      os_ << ");\n";
    }
  }
  void generateLoadStackInst(LoadStackInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    generateValue(*inst.getSingleOperand());
    os_ << ";\n";
  }
  void generateMovInst(MovInst &inst) {
    sh::Register dstReg = ra_.getRegister(&inst);
    if (ra_.isAllocated(inst.getSingleOperand()) &&
        dstReg == ra_.getRegister(inst.getSingleOperand())) {
      return;
    }
    os_.indent(2);
    generateRegister(dstReg);
    os_ << " = ";
    generateValue(*inst.getSingleOperand());
    os_ << ";\n";
  }
  void generateImplicitMovInst(ImplicitMovInst &inst) {
    // ImplicitMovs produce no bytecode, they only express that a subsequent
    // instruction will perform the equivalent of a 'Mov'.
  }
  void generateUnaryOperatorInst(UnaryOperatorInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    bool isDouble = inst.getSingleOperand()->getType().isNumberType();
    switch (inst.getKind()) {
      case (ValueKind::UnaryIncInstKind):
        if (isDouble) {
          os_ << "_sh_ljs_double(_sh_ljs_get_double(";
          generateRegister(*inst.getSingleOperand());
          os_ << ") + 1);\n";
        } else {
          os_ << "_sh_ljs_inc_rjs(shr, &";
          generateRegister(*inst.getSingleOperand());
          os_ << ");\n";
        }
        break;
      case (ValueKind::UnaryDecInstKind):
        if (isDouble) {
          os_ << "_sh_ljs_double(_sh_ljs_get_double(";
          generateRegister(*inst.getSingleOperand());
          os_ << ") - 1);\n";
        } else {
          os_ << "_sh_ljs_dec_rjs(shr, &";
          generateRegister(*inst.getSingleOperand());
          os_ << ");\n";
        }
        break;
      case (ValueKind::UnaryTildeInstKind):
        os_ << "_sh_ljs_bit_not_rjs(shr, &";
        generateRegister(*inst.getSingleOperand());
        os_ << ");\n";
        break;
      case (ValueKind::UnaryTypeofInstKind):
        os_ << "_sh_ljs_typeof(shr, &";
        generateRegister(*inst.getSingleOperand());
        os_ << ");\n";
        break;
      case ValueKind::UnaryMinusInstKind: { // -
        os_ << "_sh_ljs_minus_rjs(shr, &";
        generateRegister(*inst.getSingleOperand());
        os_ << ");\n";
        break;
      }
      case ValueKind::UnaryBangInstKind: { // !
        os_ << "_sh_ljs_bool(!_sh_ljs_to_boolean(";
        generateRegister(*inst.getSingleOperand());
        os_ << "));\n";
        break;
      }
      case ValueKind::UnaryVoidInstKind: { // Void operator.
        os_ << "_sh_ljs_undefined();\n";
        break;
      }
      default:
        unimplemented(inst);
        break;
    }
  }
  void generateDirectEvalInst(DirectEvalInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_direct_eval(shr, ";
    generateRegisterPtr(*inst.getEvalText());
    os_ << ", ";
    os_ << boolStr(inst.getStrictCaller());
    os_ << ");\n";
  }
  void generateDeclareGlobalVarInst(DeclareGlobalVarInst &inst) {
    os_ << "  _sh_ljs_declare_global_var(shr, s_symbols["
        << moduleGen_.stringTable.add(inst.getName()->getValue().str())
        << "]);\n";
  }
  void generateLoadFrameInst(LoadFrameInst &inst) {
    hermes_fatal("LoadFrameInst should have been lowered.");
  }
  void generateHBCLoadConstInst(HBCLoadConstInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    generateValue(*inst.getConst());
    os_ << ";\n";
  }
  void generateLoadParamInst(LoadParamInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    uint32_t index = inst.getParam()->getIndexInParamList();
    // Special case "this": it is always available, so we don't need a call.
    if (index == 0) {
      os_ << " = frame[" << hbc::StackFrameLayout::ThisArg << "];\n";
    } else {
      os_ << " = _sh_ljs_param(frame, " << index << ");\n";
    }
  }
  void generateHBCResolveEnvironment(HBCResolveEnvironment &inst) {
    llvh::Optional<int32_t> instScopeDepth =
        scopeAnalysis_.getScopeDepth(inst.getScope());
    llvh::Optional<int32_t> curScopeDepth = scopeAnalysis_.getScopeDepth(
        inst.getParent()->getParent()->getFunctionScope());
    if (!instScopeDepth || !curScopeDepth) {
      // This function did not have any CreateFunctionInst, it is dead.
      return;
    }
    int32_t delta = curScopeDepth.getValue() - instScopeDepth.getValue() - 1;
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_env(shr, frame, " << delta << ");\n";
  }
  void generateHBCGetArgumentsLengthInst(HBCGetArgumentsLengthInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_arguments_length(shr, frame, ";
    generateRegisterPtr(*inst.getLazyRegister());
    os_ << ");\n";
  }
  void generateHBCReifyArgumentsLooseInst(HBCReifyArgumentsLooseInst &inst) {
    os_ << "  _sh_ljs_reify_arguments_loose(shr, frame, ";
    generateRegisterPtr(*inst.getLazyRegister());
    os_ << ");\n";
  }
  void generateHBCReifyArgumentsStrictInst(HBCReifyArgumentsStrictInst &inst) {
    os_ << "  _sh_ljs_reify_arguments_strict(shr, frame, ";
    generateRegisterPtr(*inst.getLazyRegister());
    os_ << ");\n";
  }
  void generateHBCSpillMovInst(HBCSpillMovInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    generateValue(*inst.getSingleOperand());
    os_ << ";\n";
  }
  void generatePhiInst(PhiInst &inst) {
    // PhiInst has been translated into a sequence of MOVs in RegAlloc
    // Nothing to do here.
  }
  void generateBinaryOperatorInst(BinaryOperatorInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";

    // If the final result is a bool instead of a double.
    bool boolConv = false;

    // Whether to pass the arguments to a function by value.
    bool passByValue = false;

    // Function to use if we can't specialize the types.
    const char *funcUntypedOp = nullptr;

    // Infix operator for doubles.
    const char *infixDoubleOp = nullptr;
    // Function call for operator for doubles.
    const char *funcDoubleOp = nullptr;
    // Function call for operator for int32.
    const char *funcInt32Op = nullptr;

    bool bothDouble = inst.getLeftHandSide()->getType().isNumberType() &&
        inst.getRightHandSide()->getType().isNumberType();

    bool bothInt32 = inst.getLeftHandSide()->getType().isInt32Type() &&
        inst.getRightHandSide()->getType().isInt32Type();

    switch (inst.getKind()) {
      case ValueKind::BinaryAddInstKind: // +   (+=)
        if (bothDouble) {
          infixDoubleOp = "+";
        } else {
          funcUntypedOp = "_sh_ljs_add_rjs";
        }
        break;
      case ValueKind::BinarySubtractInstKind: // -   (-=)
        if (bothDouble) {
          infixDoubleOp = "-";
        } else {
          funcUntypedOp = "_sh_ljs_sub_rjs";
        }
        break;
      case ValueKind::BinaryMultiplyInstKind: // *   (*=)
        if (bothDouble) {
          infixDoubleOp = "*";
        } else {
          funcUntypedOp = "_sh_ljs_mul_rjs";
        }
        break;
      case ValueKind::BinaryDivideInstKind: // /   (/=)
        if (bothDouble)
          infixDoubleOp = "/";
        else
          funcUntypedOp = "_sh_ljs_div_rjs";
        break;
      case ValueKind::BinaryModuloInstKind: // %   (%=)
        if (bothInt32)
          funcInt32Op = "_sh_mod_int32";
        else if (bothDouble)
          funcDoubleOp = "_sh_mod_double";
        else
          funcUntypedOp = "_sh_ljs_mod_rjs";
        break;
      case ValueKind::BinaryOrInstKind: // |   (|=)
        funcUntypedOp = "_sh_ljs_bit_or_rjs";
        break;
      case ValueKind::BinaryAndInstKind: // &   (&=)
        funcUntypedOp = "_sh_ljs_bit_and_rjs";
        break;
      case ValueKind::BinaryXorInstKind: // ^   (^=)
        funcUntypedOp = "_sh_ljs_bit_xor_rjs";
        break;
      case ValueKind::BinaryRightShiftInstKind: // >>  (>>=)
        funcUntypedOp = "_sh_ljs_right_shift_rjs";
        break;
      case ValueKind::BinaryUnsignedRightShiftInstKind: // >>> (>>>=)
        funcUntypedOp = "_sh_ljs_unsigned_right_shift_rjs";
        break;
      case ValueKind::BinaryLeftShiftInstKind: // <<  (<<=)
        funcUntypedOp = "_sh_ljs_left_shift_rjs";
        break;
      case ValueKind::BinaryNotEqualInstKind: // !=
        funcUntypedOp = "!_sh_ljs_equal_rjs";
        boolConv = true;
        break;
      case ValueKind::BinaryEqualInstKind: // ==
        if (bothDouble) {
          infixDoubleOp = "==";
        } else {
          funcUntypedOp = "_sh_ljs_equal_rjs";
        }
        boolConv = true;
        break;
      case ValueKind::BinaryStrictlyNotEqualInstKind: // !==
        if (bothDouble) {
          infixDoubleOp = "!=";
        } else {
          funcUntypedOp = "!_sh_ljs_strict_equal";
          passByValue = true;
        }
        boolConv = true;
        break;
      case ValueKind::BinaryStrictlyEqualInstKind: // ===
        if (bothDouble) {
          infixDoubleOp = "==";
        } else {
          funcUntypedOp = "_sh_ljs_strict_equal";
          passByValue = true;
        }
        boolConv = true;
        break;
      case ValueKind::BinaryInInstKind: // in
        funcUntypedOp = "_sh_ljs_is_in_rjs";
        break;
      case ValueKind::BinaryInstanceOfInstKind:
        funcUntypedOp = "_sh_ljs_instance_of_rjs";
        break;
      case ValueKind::BinaryLessThanInstKind:
        if (bothDouble) {
          infixDoubleOp = "<";
        } else {
          funcUntypedOp = "_sh_ljs_less_rjs";
        }
        boolConv = true;
        break;
      case ValueKind::BinaryLessThanOrEqualInstKind:
        if (bothDouble) {
          infixDoubleOp = "<=";
        } else {
          funcUntypedOp = "_sh_ljs_less_equal_rjs";
        }
        boolConv = true;
        break;
      case ValueKind::BinaryGreaterThanInstKind:
        if (bothDouble) {
          infixDoubleOp = ">";
        } else {
          funcUntypedOp = "_sh_ljs_greater_rjs";
        }
        boolConv = true;
        break;
      case ValueKind::BinaryGreaterThanOrEqualInstKind:
        if (bothDouble) {
          infixDoubleOp = ">=";
        } else {
          funcUntypedOp = "_sh_ljs_greater_equal_rjs";
        }
        boolConv = true;
        break;
      case ValueKind::BinaryExponentiationInstKind:
      default:
        unimplemented(inst);
        return;
    }
    if (boolConv)
      os_ << "_sh_ljs_bool(";
    else if (infixDoubleOp || funcDoubleOp || funcInt32Op)
      os_ << "_sh_ljs_double(";
    if (passByValue) {
      assert(funcUntypedOp);
      os_ << funcUntypedOp << "(";
      generateRegister(*inst.getLeftHandSide());
      os_ << ", ";
      generateRegister(*inst.getRightHandSide());
      os_ << ")";
    } else if (infixDoubleOp) {
      assert(bothDouble);
      os_ << "_sh_ljs_get_double(";
      generateRegister(*inst.getLeftHandSide());
      os_ << ") " << infixDoubleOp << " _sh_ljs_get_double(";
      generateRegister(*inst.getRightHandSide());
      os_ << ")";
    } else if (funcInt32Op) {
      assert(bothInt32);
      os_ << funcInt32Op << "(";
      os_ << "_sh_ljs_get_double(";
      generateRegister(*inst.getLeftHandSide());
      os_ << "), _sh_ljs_get_double(";
      generateRegister(*inst.getRightHandSide());
      os_ << "))";
    } else if (funcDoubleOp) {
      assert(bothDouble);
      os_ << funcDoubleOp << "(";
      os_ << "_sh_ljs_get_double(";
      generateRegister(*inst.getLeftHandSide());
      os_ << "), _sh_ljs_get_double(";
      generateRegister(*inst.getRightHandSide());
      os_ << "))";
    } else {
      assert(funcUntypedOp);
      os_ << funcUntypedOp << "(shr, &";
      generateRegister(*inst.getLeftHandSide());
      os_ << ", &";
      generateRegister(*inst.getRightHandSide());
      os_ << ")";
    }
    if (boolConv)
      os_ << ")";
    else if (infixDoubleOp || funcDoubleOp || funcInt32Op)
      os_ << ")";
    os_ << ";\n";
  }

  void generateStorePropertyInstImpl(StorePropertyInst &inst, bool strictMode) {
    os_.indent(2);
    if (auto *LS = llvh::dyn_cast<LiteralString>(inst.getProperty())) {
      if (strictMode)
        os_ << "_sh_ljs_put_by_id_strict_rjs";
      else
        os_ << "_sh_ljs_put_by_id_loose_rjs";
      os_ << "(shr,&";
      generateRegister(*inst.getObject());
      os_ << ",s_symbols[" << moduleGen_.stringTable.add(LS->getValue().str())
          << "], &";
      generateRegister(*inst.getStoredValue());
      os_ << ", s_prop_cache + " << nextCacheIdx_++ << ");\n";
      return;
    }

    if (strictMode)
      os_ << "_sh_ljs_put_by_val_strict_rjs";
    else
      os_ << "_sh_ljs_put_by_val_loose_rjs";
    os_ << "(shr,&";
    generateRegister(*inst.getObject());
    os_ << ", &";
    generateRegister(*inst.getProperty());
    os_ << ", &";
    generateRegister(*inst.getStoredValue());
    os_ << ");\n";
  }
  void generateStorePropertyLooseInst(StorePropertyLooseInst &inst) {
    generateStorePropertyInstImpl(inst, false);
  }
  void generateStorePropertyStrictInst(StorePropertyStrictInst &inst) {
    generateStorePropertyInstImpl(inst, true);
  }

  void generateTryStoreGlobalPropertyInstImpl(
      TryStoreGlobalPropertyInst &inst,
      bool strictMode) {
    os_.indent(2);
    if (strictMode)
      os_ << "_sh_ljs_try_put_by_id_strict_rjs(";
    else
      os_ << "_sh_ljs_try_put_by_id_loose_rjs(";

    os_ << "shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ", ";
    auto prop = inst.getProperty();
    auto *propStr = cast<LiteralString>(prop);
    os_ << llvh::format(
               "s_symbols[%u]",
               moduleGen_.stringTable.add(propStr->getValue().str()))
        << ", ";
    generateRegisterPtr(*inst.getStoredValue());
    os_ << ", s_prop_cache + " << nextCacheIdx_++;

    os_ << ");\n";
  }
  void generateTryStoreGlobalPropertyLooseInst(
      TryStoreGlobalPropertyLooseInst &inst) {
    generateTryStoreGlobalPropertyInstImpl(inst, false);
  }
  void generateTryStoreGlobalPropertyStrictInst(
      TryStoreGlobalPropertyStrictInst &inst) {
    generateTryStoreGlobalPropertyInstImpl(inst, true);
  }

  void generateStoreOwnPropertyInst(StoreOwnPropertyInst &inst) {
    os_.indent(2);
    auto prop = inst.getProperty();
    bool isEnumerable = inst.getIsEnumerable();

    // If the property is a LiteralNumber, the property is enumerable, and it is
    // a valid array index, it is coming from an array initialization and we
    // will emit it as PutByIndex.
    auto *numProp = llvh::dyn_cast<LiteralNumber>(prop);
    if (numProp && isEnumerable) {
      if (auto arrayIndex = numProp->convertToArrayIndex()) {
        uint32_t index = arrayIndex.getValue();
        os_ << "_sh_ljs_put_own_by_index(";
        os_ << "shr, ";
        generateRegisterPtr(*inst.getObject());
        os_ << ", ";
        os_ << index << ", ";
        generateRegisterPtr(*inst.getStoredValue());
        os_ << ");\n";
        return;
      }
    }

    if (isEnumerable)
      os_ << "_sh_ljs_put_own_by_val(";
    else
      os_ << "_sh_ljs_put_own_ne_by_val(";

    os_ << "shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ", ";
    generateRegisterPtr(*prop);
    os_ << ", ";
    generateRegisterPtr(*inst.getStoredValue());
    os_ << ");\n";
  }
  void generateStoreNewOwnPropertyInst(StoreNewOwnPropertyInst &inst) {
    os_.indent(2);
    auto prop = inst.getProperty();
    bool isEnumerable = inst.getIsEnumerable();

    if (auto *numProp = llvh::dyn_cast<LiteralNumber>(prop)) {
      assert(
          isEnumerable &&
          "No way to generate non-enumerable indexed StoreNewOwnPropertyInst.");
      uint32_t index = *numProp->convertToArrayIndex();
      os_ << "_sh_ljs_put_own_by_index(";
      os_ << "shr, ";
      generateRegisterPtr(*inst.getObject());
      os_ << ", ";
      os_ << index << ", ";
      generateRegisterPtr(*inst.getStoredValue());
      os_ << ");\n";
      return;
    }

    if (isEnumerable)
      os_ << "_sh_ljs_put_new_own_by_id(";
    else
      os_ << "_sh_ljs_put_new_own_ne_by_id(";

    os_ << "shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ", ";
    auto *propStr = cast<LiteralString>(prop);
    os_ << llvh::format(
               "s_symbols[%u]",
               moduleGen_.stringTable.add(propStr->getValue().str()))
        << ", &";
    generateRegister(*inst.getStoredValue());
    os_ << ");\n";
  }
  void generateStoreGetterSetterInst(StoreGetterSetterInst &inst) {
    os_.indent(2);
    os_ << "_sh_ljs_put_own_getter_setter_by_val(";
    os_ << "shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ", ";
    generateRegisterPtr(*inst.getProperty());
    os_ << ", ";
    generateRegisterPtr(*inst.getStoredGetter());
    os_ << ", ";
    generateRegisterPtr(*inst.getStoredSetter());
    os_ << ", " << inst.getIsEnumerable();
    os_ << ");\n";
  }
  void generateDeletePropertyInstImpl(
      DeletePropertyInst &inst,
      bool strictMode) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    auto prop = inst.getProperty();
    if (auto *propStr = llvh::dyn_cast<LiteralString>(prop)) {
      if (strictMode)
        os_ << "_sh_ljs_del_by_id_strict(";
      else
        os_ << "_sh_ljs_del_by_id_loose(";

      os_ << "shr, ";
      generateRegisterPtr(*inst.getObject());
      os_ << ", ";
      os_ << llvh::format(
          "s_symbols[%u]",
          moduleGen_.stringTable.add(propStr->getValue().str()));
      os_ << ");\n";
      return;
    }

    if (strictMode)
      os_ << "_sh_ljs_del_by_val_strict(";
    else
      os_ << "_sh_ljs_del_by_val_loose(";

    os_ << "shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ", ";
    generateRegisterPtr(*prop);
    os_ << ");\n";
  }
  void generateDeletePropertyLooseInst(DeletePropertyLooseInst &inst) {
    generateDeletePropertyInstImpl(inst, false);
  }
  void generateDeletePropertyStrictInst(DeletePropertyStrictInst &inst) {
    generateDeletePropertyInstImpl(inst, true);
  }
  void generateLoadPropertyInst(LoadPropertyInst &inst) {
    os_.indent(2);
    generateValue(inst);
    os_ << " = ";
    if (auto *LS = llvh::dyn_cast<LiteralString>(inst.getProperty())) {
      os_ << "_sh_ljs_get_by_id_rjs(shr,&";
      generateRegister(*inst.getObject());
      os_ << ",s_symbols[" << moduleGen_.stringTable.add(LS->getValue().str())
          << "], s_prop_cache + " << nextCacheIdx_++ << ");\n";
      return;
    }
    os_ << "_sh_ljs_get_by_val_rjs(shr,&";
    generateRegister(*inst.getObject());
    os_ << ", &";
    generateRegister(*inst.getProperty());
    os_ << ");\n";
  }
  void generateTryLoadGlobalPropertyInst(TryLoadGlobalPropertyInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    LiteralString *LS = inst.getProperty();
    os_ << "_sh_ljs_try_get_by_id_rjs(shr,&";
    generateRegister(*inst.getObject());
    os_ << ",s_symbols[" << moduleGen_.stringTable.add(LS->getValue().str())
        << "], s_prop_cache + " << nextCacheIdx_++ << ");\n";
  }
  void generateLoadParentInst(LoadParentInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    os_ << "_sh_load_parent(shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ");\n";
  }
  void generateStoreParentInst(StoreParentInst &inst) {
    os_.indent(2);
    os_ << "_sh_store_parent(shr, ";
    generateRegisterPtr(*inst.getStoredValue());
    os_ << ", ";
    generateRegisterPtr(*inst.getObject());
    os_ << ");\n";
  }
  void generateStoreStackInst(StoreStackInst &inst) {
    hermes_fatal("StoreStackInst should have been lowered.");
  }
  void generateStoreFrameInst(StoreFrameInst &inst) {
    hermes_fatal("StoreFrameInst should have been lowered.");
  }
  void generateAllocStackInst(AllocStackInst &inst) {
    // This is a no-op.
  }
  void generateAllocObjectInst(AllocObjectInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    // TODO: Utilize sizeHint.
    if (llvh::isa<EmptySentinel>(inst.getParentObject())) {
      os_ << "_sh_ljs_new_object(shr)";
    } else {
      os_ << "_sh_ljs_new_object_with_parent(shr, &";
      generateValue(*inst.getParentObject());
      os_ << ")";
    }
    os_ << ";\n";
  }
  void generateAllocArrayInst(AllocArrayInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    auto elementCount = inst.getElementCount();
    uint32_t sizeHint = inst.getSizeHint()->asUInt32();

    if (elementCount == 0) {
      os_ << "_sh_ljs_new_array(shr, " << sizeHint << ")";
    } else {
      llvh::SmallVector<Literal *, 8> elements;
      for (unsigned i = 0, e = inst.getElementCount(); i < e; ++i) {
        elements.push_back(cast<Literal>(inst.getArrayElement(i)));
      }
      auto bufIndex = moduleGen_.literalBuffers.addArrayBuffer(
          ArrayRef<Literal *>{elements});

      os_ << "_sh_ljs_new_array_with_buffer(shr, &THIS_UNIT, ";
      os_ << sizeHint << ", ";
      os_ << elementCount << ", ";
      os_ << bufIndex << ")";
    }
    os_ << ";\n";
  }
  void generateGetTemplateObjectInst(GetTemplateObjectInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    // If not dup, argCount also includes cooked strings.
    uint32_t numStrings = inst.getNumStrings();
    uint32_t argCount = inst.isDup() ? numStrings : numStrings * 2;
    // Can't lower to calling the HermesBuiltin because that depends on
    // RuntimeModule, so we have a _sh_get_template_object function instead,
    // which can read from the SHUnit templateMap.
    os_ << "_sh_get_template_object(shr, &THIS_UNIT, "
        << inst.getTemplateObjID() << ", " << boolStr(inst.isDup()) << ", "
        << argCount;
    for (unsigned i = 0; i < numStrings; ++i) {
      os_ << ", ";
      generateRegisterPtr(*inst.getRawString(i));
    }
    if (!inst.isDup()) {
      for (unsigned i = 0; i < numStrings; ++i) {
        os_ << ", ";
        generateRegisterPtr(*inst.getCookedString(i));
      }
    }
    os_ << ");\n";
  }
  void generateAllocObjectLiteralInst(AllocObjectLiteralInst &inst) {
    // This instruction should not have reached this far.
    hermes_fatal("AllocObjectLiteralInst should have been lowered.");
  }
  void generateCreateArgumentsInst(CreateArgumentsInst &inst) {
    hermes_fatal("CreateArgumentsInst should have been lowered.");
  }
  void generateCatchInst(CatchInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_catch(shr, (SHLocals*)&locals, frame, "
        << (ra_.getMaxArgumentRegisters() + hbc::StackFrameLayout::FirstLocal)
        << ");\n";
  }
  void generateDebuggerInst(DebuggerInst &inst) {
    unimplemented(inst);
  }
  void generateCreateRegExpInst(CreateRegExpInst &inst) {
    os_.indent(2);

    uint32_t patternStrID =
        moduleGen_.stringTable.add(inst.getPattern()->getValue().str());
    uint32_t flagsStrID =
        moduleGen_.stringTable.add(inst.getFlags()->getValue().str());

    // Compile the regexp. We expect this to succeed because the AST went
    // through the SemanticValidator. This is a bit of a hack: what we would
    // really like to do is have the Parser emit a CompiledRegExp that can be
    // threaded through the AST and then through the IR to this instruction
    // selection, but that is too awkward, so we compile again here.
    // uint32_t reBytecodeID = UINT32_MAX;
    // if (auto regexp = CompiledRegExp::tryCompile(
    //         inst.getPattern()->getValue().str(),
    //         inst.getFlags()->getValue().str())) {
    //   reBytecodeID = moduleGen_.regexpTable.addRegExp(std::move(*regexp));
    // }

    // TODO(T132343328): Compile the regexp bytecode ahead of time.
    generateValue(inst);
    os_ << " = ";
    os_ << "_sh_ljs_create_regexp(shr, ";
    os_ << llvh::format("s_symbols[%u]", patternStrID);
    os_ << ", ";
    os_ << llvh::format("s_symbols[%u]", flagsStrID);
    os_ << ");\n";
  }
  void generateTryEndInst(TryEndInst &inst) {
    // TODO(T132354002): Properly understand the nesting of try blocks so we can
    // directly pass in the corresponding SHJmpBuf here instead of retrieving it
    // from shr.
    os_ << "  _sh_end_try(shr);\n";
  }
  void generateGetNewTargetInst(GetNewTargetInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = frame[" << hbc::StackFrameLayout::NewTarget << "];\n";
  }
  void generateThrowIfEmptyInst(ThrowIfEmptyInst &inst) {
    os_.indent(2);
    os_ << "if (_sh_ljs_is_empty(";
    generateRegister(inst);
    os_ << " = ";
    generateValue(*inst.getCheckedValue());
    os_ << ")) _sh_throw_empty(shr);\n";
  }
  void generateIteratorBeginInst(IteratorBeginInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_iterator_begin_rjs(shr, ";
    generateRegisterPtr(*inst.getSourceOrNext());
    os_ << ");\n";
  }
  void generateIteratorNextInst(IteratorNextInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_iterator_next_rjs(shr, ";
    generateRegisterPtr(*inst.getIterator());
    os_ << ", ";
    generateRegisterPtr(*inst.getSourceOrNext());
    os_ << ");\n";
  }
  void generateIteratorCloseInst(IteratorCloseInst &inst) {
    os_ << "  _sh_ljs_iterator_close_rjs(shr, ";
    generateRegisterPtr(*inst.getIterator());
    os_ << ", " << boolStr(inst.getIgnoreInnerException());
    os_ << ");\n";
  }
  void generateHBCStoreToEnvironmentInst(HBCStoreToEnvironmentInst &inst) {
    os_ << "  _sh_ljs_store_to_env(shr, ";
    generateValue(*inst.getEnvironment());
    os_ << ",";
    generateValue(*inst.getStoredValue());
    os_ << ", " << inst.getResolvedName()->getIndexInVariableList() << ");\n";
  }
  void generateHBCLoadFromEnvironmentInst(HBCLoadFromEnvironmentInst &inst) {
    os_.indent(2);
    generateValue(inst);
    os_ << " = _sh_ljs_load_from_env(";
    generateValue(*inst.getEnvironment());
    os_ << ", " << inst.getResolvedName()->getIndexInVariableList() << ");\n";
  }
  void generateUnreachableInst(UnreachableInst &inst) {
    unimplemented(inst);
  }
  void generateCreateFunctionInst(CreateFunctionInst &inst) {
    hermes_fatal("CreateFunctionInst should have been lowered.");
  }
  void generateCreateGeneratorInst(CreateGeneratorInst &inst) {
    unimplemented(inst);
  }
  void generateHBCCreateFunctionInst(HBCCreateFunctionInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_create_closure_"
        << (inst.getFunctionCode()->isStrictMode() ? "strict" : "loose")
        << "(shr, &";
    generateRegister(*inst.getEnvironment());
    os_ << ", ";
    moduleGen_.generateFunctionLabel(inst.getFunctionCode(), os_);
    os_ << ", s_symbols["
        << moduleGen_.stringTable.add(
               inst.getFunctionCode()->getOriginalOrInferredName().str())
        << "], "
        << inst.getFunctionCode()->getExpectedParamCountIncludingThis() - 1
        << ");\n";
  }
  void generateHBCCreateGeneratorInst(HBCCreateGeneratorInst &inst) {
    unimplemented(inst);
  }
  void generateBranchInst(BranchInst &inst) {
    os_ << "  goto ";
    generateBasicBlockLabel(inst.getBranchDest(), os_, bbMap_);
    os_ << ";";
  }
  void generateReturnInst(ReturnInst &inst) {
    os_ << "  _sh_leave(shr, &locals.head, frame);\n  return ";
    generateValue(*inst.getValue());
    os_ << ";\n";
  }
  void generateThrowInst(ThrowInst &inst) {
    os_ << "  _sh_throw(shr, ";
    generateValue(*inst.getThrownValue());
    os_ << ");\n";
  }
  void generateSwitchInst(SwitchInst &inst) {
    hermes_fatal("SwitchInst should have been lowered");
  }
  void generateCondBranchInst(CondBranchInst &inst) {
    os_.indent(2);
    os_ << "if(";
    if (inst.getCondition()->getType().isBooleanType()) {
      os_ << "_sh_ljs_get_bool(";
    } else {
      os_ << "_sh_ljs_to_boolean(";
    }
    generateRegister(*inst.getCondition());
    os_ << ")) ";
    os_ << "goto ";
    generateBasicBlockLabel(inst.getTrueDest(), os_, bbMap_);
    os_ << ";\n  goto ";
    generateBasicBlockLabel(inst.getFalseDest(), os_, bbMap_);
    os_ << ";\n";
  }
  void generateGetPNamesInst(GetPNamesInst &inst) {
    os_.indent(2);
    generateValue(*inst.getIterator());
    os_ << " = _sh_ljs_get_pname_list_rjs(shr, ";
    generateRegisterPtr(*inst.getBase());
    os_ << ", ";
    generateRegisterPtr(*inst.getIndex());
    os_ << ", ";
    generateRegisterPtr(*inst.getSize());
    os_ << ");\n";

    os_.indent(2) << "if (_sh_ljs_is_undefined(";
    generateValue(*inst.getIterator());
    os_ << ")) goto ";
    generateBasicBlockLabel(inst.getOnEmptyDest(), os_, bbMap_);
    os_ << ";\n";

    os_.indent(2) << "goto ";
    generateBasicBlockLabel(inst.getOnSomeDest(), os_, bbMap_);
    os_ << ";\n";
  }
  void generateGetNextPNameInst(GetNextPNameInst &inst) {
    os_.indent(2);
    generateValue(*inst.getPropertyAddr());
    os_ << " = _sh_ljs_get_next_pname_rjs(shr, ";
    generateRegisterPtr(*inst.getIteratorAddr());
    os_ << ", ";
    generateRegisterPtr(*inst.getBaseAddr());
    os_ << ", ";
    generateRegisterPtr(*inst.getIndexAddr());
    os_ << ", ";
    generateRegisterPtr(*inst.getSizeAddr());
    os_ << ");\n";

    os_.indent(2) << "if (_sh_ljs_is_undefined(";
    generateValue(*inst.getPropertyAddr());
    os_ << ")) goto ";
    generateBasicBlockLabel(inst.getOnLastDest(), os_, bbMap_);
    os_ << ";\n";

    os_.indent(2) << "goto ";
    generateBasicBlockLabel(inst.getOnSomeDest(), os_, bbMap_);
    os_ << ";\n";
  }
  void generateTryStartInst(TryStartInst &inst) {
    // TODO(T132354002): Properly understand the nesting of try blocks so we can
    // reuse SHJmpBufs.
    os_ << "  SHJmpBuf jBuf" << nextJBufIdx_ << ";\n";
    os_ << "  if(_sh_try(shr, &jBuf" << nextJBufIdx_ << ") == 0) goto ";
    generateBasicBlockLabel(inst.getTryBody(), os_, bbMap_);
    os_ << ";\n";
    os_ << "  goto ";
    generateBasicBlockLabel(inst.getCatchTarget(), os_, bbMap_);
    os_ << ";\n";
    nextJBufIdx_++;
  }
  void generateCompareBranchInst(CompareBranchInst &inst) {
    os_ << "  if(";

    // Whether to pass the arguments to a function by value.
    bool passByValue = false;

    // Function to use if we can't specialize the types.
    const char *funcUntypedOp = nullptr;

    // Infix operator for doubles.
    const char *infixDoubleOp = nullptr;

    bool bothDouble = inst.getLeftHandSide()->getType().isNumberType() &&
        inst.getRightHandSide()->getType().isNumberType();

    switch (inst.getKind()) {
      case ValueKind::CmpBrLessThanInstKind: // <
        if (bothDouble) {
          infixDoubleOp = "<";
        } else {
          funcUntypedOp = "_sh_ljs_less_rjs";
        }
        break;
      case ValueKind::CmpBrLessThanOrEqualInstKind: // <=
        if (bothDouble) {
          infixDoubleOp = "<=";
        } else {
          funcUntypedOp = "_sh_ljs_less_equal_rjs";
        }
        break;
      case ValueKind::CmpBrGreaterThanInstKind: // >
        if (bothDouble) {
          infixDoubleOp = ">";
        } else {
          funcUntypedOp = "_sh_ljs_greater_rjs";
        }
        break;
      case ValueKind::CmpBrGreaterThanOrEqualInstKind: // >=
        if (bothDouble) {
          infixDoubleOp = ">=";
        } else {
          funcUntypedOp = "_sh_ljs_greater_equal_rjs";
        }
        break;
      case ValueKind::CmpBrEqualInstKind: // ==
        funcUntypedOp = "_sh_ljs_equal_rjs";
        break;
      case ValueKind::CmpBrNotEqualInstKind: // !=
        funcUntypedOp = "!_sh_ljs_equal_rjs";
        break;
      case ValueKind::CmpBrStrictlyEqualInstKind: // ===
        if (bothDouble) {
          infixDoubleOp = "==";
        } else {
          funcUntypedOp = "_sh_ljs_strict_equal";
          passByValue = true;
        }
        break;
      case ValueKind::CmpBrStrictlyNotEqualInstKind: // !==
        if (bothDouble) {
          infixDoubleOp = "!=";
        } else {
          funcUntypedOp = "!_sh_ljs_strict_equal";
          passByValue = true;
        }
        break;
      default:
        hermes_fatal("Invalid operator for CompareBranchInst");
    }
    if (passByValue) {
      assert(funcUntypedOp);
      os_ << funcUntypedOp << "(";
      generateRegister(*inst.getLeftHandSide());
      os_ << ", ";
      generateRegister(*inst.getRightHandSide());
      os_ << ")";
    } else if (infixDoubleOp) {
      assert(bothDouble);
      os_ << "_sh_ljs_get_double(";
      generateRegister(*inst.getLeftHandSide());
      os_ << ") " << infixDoubleOp << " _sh_ljs_get_double(";
      generateRegister(*inst.getRightHandSide());
      os_ << ")";
    } else {
      assert(funcUntypedOp);
      os_ << funcUntypedOp << "(shr, &";
      generateRegister(*inst.getLeftHandSide());
      os_ << ", &";
      generateRegister(*inst.getRightHandSide());
      os_ << ")";
    }
    os_ << ") goto ";
    generateBasicBlockLabel(inst.getTrueDest(), os_, bbMap_);
    os_ << ";\n  goto ";
    generateBasicBlockLabel(inst.getFalseDest(), os_, bbMap_);
    os_ << ";\n";
  }
  void generateSwitchImmInst(SwitchImmInst &inst) {
    unimplemented(inst);
  }
  void generateSaveAndYieldInst(SaveAndYieldInst &inst) {
    unimplemented(inst);
  }
  void generateCallInst(CallInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_call(shr, frame, " << inst.getNumArguments() - 1
        << ");\n";
  }
  void generateGetBuiltinClosureInst(GetBuiltinClosureInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_builtin_closure(shr, "
        << (uint32_t)inst.getBuiltinIndex() << ");\n";
  }
  void generateCallBuiltinInst(CallBuiltinInst &inst) {
    if (inst.getBuiltinIndex() == BuiltinMethod::Math_sqrt) {
      if (inst.getNumArguments() == 2 &&
          inst.getArgument(1)->getType().isNumberType()) {
        os_.indent(2);
        generateRegister(inst);
        os_ << " = _sh_ljs_double(sqrt(_sh_ljs_get_double(";
        generateValue(*inst.getArgument(1));
        os_ << ")));\n";
        return;
      }
    }
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_call_builtin(shr, frame, " << inst.getNumArguments() - 1
        << ", " << (uint32_t)inst.getBuiltinIndex() << ");\n";
  }
  void generateConstructInst(ConstructInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_construct(shr, frame, " << inst.getNumArguments() - 1
        << ");\n";
  }
  void generateHBCCallNInst(HBCCallNInst &inst) {
    unimplemented(inst);
  }
  void generateStartGeneratorInst(StartGeneratorInst &inst) {
    unimplemented(inst);
  }
  void generateResumeGeneratorInst(ResumeGeneratorInst &inst) {
    unimplemented(inst);
  }
  void generateHBCGetGlobalObjectInst(HBCGetGlobalObjectInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_global_object(shr);\n";
  }
  void generateHBCCreateEnvironmentInst(HBCCreateEnvironmentInst &inst) {
    os_ << "  _sh_ljs_create_environment(shr, frame, &";
    generateRegister(inst);
    os_ << ", " << envSize_ << ");\n";
  }
  void generateCoerceThisNSInst(CoerceThisNSInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_coerce_this_ns(shr, ";
    generateRegister(*inst.getSingleOperand());
    os_ << ");\n";
  }
  void generateLIRGetThisNSInst(LIRGetThisNSInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_coerce_this_ns(shr, frame["
        << hbc::StackFrameLayout::ThisArg << "]);\n";
  }
  void generateCreateThisInst(CreateThisInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_create_this(shr, &";
    generateRegister(*inst.getPrototype());
    os_ << ", &";
    generateRegister(*inst.getClosure());
    os_ << ");\n";
  }
  void generateHBCGetArgumentsPropByValLooseInst(
      HBCGetArgumentsPropByValLooseInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_arguments_prop_by_val_loose(shr, frame, ";
    generateRegisterPtr(*inst.getIndex());
    os_ << ", ";
    generateRegisterPtr(*inst.getLazyRegister());
    os_ << ");\n";
  }
  void generateHBCGetArgumentsPropByValStrictInst(
      HBCGetArgumentsPropByValStrictInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_arguments_prop_by_val_strict(shr, frame, ";
    generateRegisterPtr(*inst.getIndex());
    os_ << ", ";
    generateRegisterPtr(*inst.getLazyRegister());
    os_ << ");\n";
  }
  void generateGetConstructedObjectInst(GetConstructedObjectInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_is_object(";
    generateRegister(*inst.getConstructorReturnValue());
    os_ << ") ? ";
    generateRegister(*inst.getConstructorReturnValue());
    os_ << " : ";
    generateRegister(*inst.getThisValue());
    os_ << ";\n";
  }
  void generateHBCAllocObjectFromBufferInst(
      HBCAllocObjectFromBufferInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    int numLiterals = inst.getKeyValuePairCount();
    llvh::SmallVector<Literal *, 8> objKeys;
    llvh::SmallVector<Literal *, 8> objVals;
    for (int ind = 0; ind < numLiterals; ind++) {
      auto keyValuePair = inst.getKeyValuePair(ind);
      objKeys.push_back(cast<Literal>(keyValuePair.first));
      objVals.push_back(cast<Literal>(keyValuePair.second));
    }

    // size hint operand of NewObjectWithBuffer opcode is 16-bit.
    uint32_t sizeHint =
        std::min((uint32_t)UINT16_MAX, inst.getSizeHint()->asUInt32());

    auto buffIdxs = moduleGen_.literalBuffers.addObjectBuffer(
        llvh::ArrayRef<Literal *>{objKeys}, llvh::ArrayRef<Literal *>{objVals});
    os_ << " = ";
    os_ << "_sh_ljs_new_object_with_buffer(shr, &THIS_UNIT, ";
    os_ << sizeHint << ", ";
    os_ << numLiterals << ", ";
    os_ << buffIdxs.first << ", ";
    os_ << buffIdxs.second << ")";
    os_ << ";\n";
  }
  void generateHBCProfilePointInst(HBCProfilePointInst &inst) {
    unimplemented(inst);
  }
  void generatePrLoadInst(PrLoadInst &inst) {
    os_.indent(2);
    generateValue(inst);
    os_ << " = ";
    os_ << "_sh_prload(shr, ";
    generateRegister(*inst.getObject());
    os_ << ", " << inst.getPropIndex() << ");\n";
  }
  void generatePrStoreInst(PrStoreInst &inst) {
    os_.indent(2);
    const char *suffix = "";
    Type propType = inst.getStoredValue()->getType();
    if (propType.isNumberType()) {
      suffix = "_number";
    } else if (propType.isBooleanType()) {
      suffix = "_boolean";
    } else if (propType.isObjectType()) {
      suffix = "_object";
    } else if (propType.isStringType()) {
      suffix = "_string";
    }
    os_ << "_sh_prstore" << suffix << "(shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ", " << inst.getPropIndex() << ", ";
    generateRegisterPtr(*inst.getStoredValue());
    os_ << ");\n";
  }
};

/// Converts Function \p F into valid C code and outputs it through \p OS.
void generateFunction(
    Function &F,
    llvh::raw_ostream &OS,
    ModuleGen &moduleGen,
    FunctionScopeAnalysis &scopeAnalysis,
    uint32_t &nextCacheIdx,
    BytecodeGenerationOptions options) {
  PostOrderAnalysis PO(&F);

  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());

  sh::SHRegisterAllocator RA(&F);

  RA.allocate(order);

  if (options.format == DumpRA) {
    RA.dump();
    return;
  }

  PassManager PM;
  PM.addPass(new LowerStoreInstrs(RA));
  PM.addPass(new sh::LowerCalls(RA));
  if (options.optimizationEnabled) {
    PM.addPass(new MovElimination(RA));
    PM.addPass(new sh::RecreateCheapValues(RA));
  }
  PM.run(&F);

  if (options.format == DumpLRA) {
    RA.dump();
    return;
  }

  if (options.format == DumpPostRA) {
    F.dump();
    return;
  }

  assert(
      (options.format == DumpBytecode || options.format == EmitBundle) &&
      "All other dump formats must be handled before this.");

  // Set up the entry to the function to look like:
  // static SHLegacyValue _1_myFunc(SHRuntime *shr) {
  //   struct {
  //     SHLocals head;
  //     SHLegacyValue t0;
  //     SHLegacyValue t1;
  //     SHLegacyValue t2;
  //   } locals;
  //   SHLegacyValue *frame = _sh_enter(shr, &locals.head, 3);
  //   locals.head.count = 3;
  //   locals.t0 = _sh_ljs_undefined();
  //   locals.t1 = _sh_ljs_undefined();
  //   locals.t2 = _sh_ljs_undefined();

  unsigned bbCounter = 0;
  llvh::DenseMap<BasicBlock *, unsigned> bbMap;
  for (auto &B : order) {
    bbMap[B] = bbCounter;
    bbCounter++;
  }

  OS << "// ";
  F.getContext().getSourceErrorManager().dumpCoords(
      OS, F.getSourceRange().Start);
  OS << '\n';

  unsigned envSize = F.getFunctionScope()->getVariables().size();

  InstrGen instrGen(
      OS, RA, bbMap, F, moduleGen, scopeAnalysis, envSize, nextCacheIdx);

  // Number of registers stored in the `locals` struct below.
  uint32_t localsSize = 0;

  OS << "static SHLegacyValue ";
  moduleGen.generateFunctionLabel(&F, OS);
  OS << "(SHRuntime *shr) {\n";

  // In the global function, ensure that we are linking to the correct
  // library.
  if (F.isGlobalScope())
    OS << "  _SH_MODEL();\n";

  OS << "  struct {\n    SHLocals head;\n";

  for (size_t i = 0; i < RA.getMaxRegisterUsage(); ++i) {
    if (instrGen.registerIsPointer(i)) {
      OS << "    SHLegacyValue t" << i << ";\n";
      ++localsSize;
    }
  }

  OS << "  } locals;\n"
     << "  SHLegacyValue *frame = _sh_enter(shr, &locals.head, "
     << (RA.getMaxArgumentRegisters() + hbc::StackFrameLayout::FirstLocal)
     << ");\n"
     << "  locals.head.count =" << localsSize << ";\n";

  // Initialize all registers to undefined.
  for (size_t i = 0; i < RA.getMaxRegisterUsage(); ++i) {
    if (instrGen.registerIsPointer(i)) {
      OS << "  locals.t";
    } else {
      OS << "  SHLegacyValue r";
    }
    OS << i << " = _sh_ljs_undefined();\n";
  }

  for (auto &B : order) {
    OS << "\n";
    generateBasicBlockLabel(B, OS, bbMap);
    OS << ":\n"
       // Print a no-op expression, since the line after a label must be an
       // expression (as opposed to a declaration).
       << "  ;\n";

    for (auto &I : *B) {
      instrGen.generate(I);
    }
  }

  OS << "}\n";
}

/// Converts Module \p M into valid C code and outputs it through \p OS.
/// Returns the cache size necessary to store all the cache indexes used.
void generateModule(
    Module *M,
    llvh::raw_ostream &OS,
    const BytecodeGenerationOptions &options) {
  PassManager PM;
  PM.addPass(createLIRPeephole());
  // LowerExponentiationOperator needs to run before LowerBuiltinCalls because
  // it introduces calls to HermesInternal.
  PM.addPass(new LowerExponentiationOperator());
  // LowerBuiltinCalls needs to run before the rest of the lowering.
  PM.addPass(new LowerBuiltinCalls());
  PM.addPass(new LowerNumericProperties());
  PM.addPass(new LowerAllocObjectLiteral());
  PM.addPass(new hbc::LowerArgumentsArray());
  PM.addPass(new LimitAllocArray(UINT16_MAX));
  PM.addPass(new hbc::DedupReifyArguments());
  // TODO Consider supporting LowerSwitchIntoJumpTables for optimization
  PM.addPass(new SwitchLowering());
  PM.addPass(new hbc::LoadConstants(true));
  PM.addPass(new hbc::LowerLoadStoreFrameInst());
  if (options.optimizationEnabled) {
    PM.addTypeInference();
    // Lowers AllocObjects and its sequential literal properties into a single
    // Reduce comparison and conditional jump to single comparison jump
    PM.addPass(new LowerCondBranch());
    // Move loads to child blocks if possible.
    PM.addCodeMotion();
    // Eliminate common HBCLoadConstInsts.
    // TODO(T140823187): Run before CodeMotion too.
    // Avoid pushing HBCLoadConstInsts down into individual blocks,
    // preventing their elimination.
    PM.addCSE();
    // Drop unused LoadParamInsts.
    PM.addDCE();
  }
  PM.run(M);

  if (options.format == DumpLIR) {
    M->dump();
    return;
  }

  // TODO: Share cache indices where the property name is the same and
  // -reuse-prop-cache is passed in.
  uint32_t nextCacheIdx = 0;
  ModuleGen moduleGen;

  auto topLevelFunc = M->getTopLevelFunction();
  moduleGen.funcMap[topLevelFunc] = 0;
  unsigned funcCounter = 1;
  for (auto &F : *M) {
    if (&F != topLevelFunc) {
      moduleGen.funcMap[&F] = funcCounter;
      funcCounter++;
    }
  }

  FunctionScopeAnalysis scopeAnalysis{topLevelFunc};

  if (options.format == DumpBytecode || options.format == EmitBundle) {
    if (!isValidSHUnitName(options.unitName))
      hermes_fatal("Invalid unit name passed to SH backend.");
    // Note that we prefix the unit name with sh_export_ to avoid potential
    // conflicts.
    OS << "#define THIS_UNIT sh_export_" << options.unitName << R"(
#include "hermes/VM/static_h.h"

SHUnit THIS_UNIT;

static SHSymbolID s_symbols[];
static SHPropertyCacheEntry s_prop_cache[];
)";

    // Forward declare every JS function.
    for (auto &F : *M) {
      OS << "static SHLegacyValue ";
      moduleGen.generateFunctionLabel(&F, OS);
      OS << "(SHRuntime *shr);\n";
    }
  }

  for (auto &F : *M)
    generateFunction(F, OS, moduleGen, scopeAnalysis, nextCacheIdx, options);

  if (options.format == DumpBytecode || options.format == EmitBundle) {
    moduleGen.stringTable.generate(OS);
    moduleGen.literalBuffers.generate(OS);

    OS << "static SHPropertyCacheEntry s_prop_cache[" << nextCacheIdx << "];\n"
       << "SHUnit THIS_UNIT = { .num_symbols = " << moduleGen.stringTable.size()
       << ", .num_prop_cache_entries = " << nextCacheIdx
       << ", .ascii_pool = s_ascii_pool, .u16_pool = s_u16_pool,"
       << ".strings = s_strings, .symbols = s_symbols, .prop_cache = s_prop_cache,"
       << ".obj_key_buffer = s_obj_key_buffer, .obj_key_buffer_size = "
       << moduleGen.literalBuffers.objKeyBuffer.size() << ", "
       << ".obj_val_buffer = s_obj_val_buffer, .obj_val_buffer_size = "
       << moduleGen.literalBuffers.objValBuffer.size() << ", "
       << ".array_buffer = s_array_buffer, .array_buffer_size = "
       << moduleGen.literalBuffers.arrayBuffer.size() << ", "
       << ".unit_main = _0_global, .unit_main_strict = "
       << boolStr(M->getTopLevelFunction()->isStrictMode()) << ", "
       << ".unit_name = \"sh_compiled\" };\n";
    if (options.emitMain) {
      OS << R"(
int main(int argc, char **argv) {
  SHRuntime *shr = _sh_init(argc, argv);
  bool success = _sh_initialize_units(shr, 1, &THIS_UNIT);
  _sh_done(shr);
  return success ? 0 : 1;
}
)";
    }
  }
}
} // namespace

/// Converts Module \p M into valid C code and outputs it through \p OS
void sh::generateSH(
    Module *M,
    llvh::raw_ostream &OS,
    const BytecodeGenerationOptions &options) {
  generateModule(M, OS, options);
}
