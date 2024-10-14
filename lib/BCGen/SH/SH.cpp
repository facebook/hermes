/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/SH/SH.h"

#include "LineDirectiveEmitter.h"
#include "LoweringPasses.h"
#include "SHRegAlloc.h"
#include "hermes/AST/NativeContext.h"
#include "hermes/BCGen/FunctionInfo.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/StackFrameLayout.h"
#include "hermes/BCGen/LiteralBufferBuilder.h"
#include "hermes/BCGen/LowerBuiltinCalls.h"
#include "hermes/BCGen/LowerScopes.h"
#include "hermes/BCGen/LowerStoreInstrs.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/BCGen/RemoveMovs.h"
#include "hermes/BCGen/SerializedLiteralGenerator.h"
#include "hermes/BCGen/ShapeTableEntry.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/DenseMapInfoSpecializations.h"
#include "hermes/Support/HashString.h"
#include "hermes/Support/UTF8.h"
#include "llvh/ADT/MapVector.h"

#include "llvh/ADT/BitVector.h"
#include "llvh/ADT/SetVector.h"

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
      if (isAllASCII(str)) {
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
    // static const uint32_t s_strings[] = {
    //     0, 5, 0, 6, 5, 0,
    // };

    os << "static const char s_ascii_pool[] = {\n"
       << asciiStr << "};\n"
       << "static const char16_t s_u16_pool[] = {\n"
       << u16Str << "};\n"
       << "static const uint32_t s_strings[] = {";
    for (const auto &entry : stringEntries)
      os << entry.offset << "," << entry.length << "," << entry.hash << ",";
    os << "};\n";
  }
};

class SHLiteralBuffers {
 public:
  /// Table of constants used to initialize constant arrays/object values.
  /// They are stored as chars in order to shorten bytecode size.
  std::vector<unsigned char> literalValueBuffer{};

  /// Table of constants used to initialize object keys.
  /// They are stored as chars in order to shorten bytecode size
  std::vector<unsigned char> objKeyBuffer{};

  /// Table of object literal shapes.
  std::vector<ShapeTableEntry> objShapeTable{};

  /// A map from instruction to literal offset in the corresponding buffers.
  /// \c arrayBuffer, \c objKeyBuffer, \c objliteralValBuffer.
  LiteralBufferBuilder::LiteralOffsetMapTy literalOffsetMap{};

  explicit SHLiteralBuffers(
      Module *M,
      SHStringTable &table,
      bool optimizationEnabled) {
    LiteralBufferBuilder::Result bufs = LiteralBufferBuilder::generate(
        M,
        [](const Function *) { return true; },
        [&table](llvh::StringRef str) { return table.add(str); },
        [&table](llvh::StringRef str) { return table.add(str); },
        optimizationEnabled);
    literalValueBuffer = std::move(bufs.literalValBuffer);
    objKeyBuffer = std::move(bufs.keyBuffer);
    objShapeTable = std::move(bufs.shapeTable);
    literalOffsetMap = std::move(bufs.offsetMap);
  }

  /// For a given instruction \p inst that has an associated serialized literal,
  /// obtain the offset of the literal in the associated buffer. In case of
  /// an object literal, it is a pair of offsets (key and value). In case of
  /// array literal, only the first offset is used.
  LiteralBufferBuilder::LiteralOffset serializedLiteralOffsetFor(
      const Instruction *inst) const {
    assert(
        literalOffsetMap.count(inst) &&
        "instruction has no serialized literal");
    return literalOffsetMap.find(inst)->second;
  }

  void generate(llvh::raw_ostream &os) const {
    generateBuffer(os, "s_literal_val_buffer", literalValueBuffer);
    generateBuffer(os, "s_obj_key_buffer", objKeyBuffer);
    // Generate the shape table.
    os << "static const SHShapeTableEntry s_obj_shape_table[] = {\n";
    for (auto &entry : objShapeTable) {
      os.indent(2);
      os << "{ .key_buffer_offset = " << entry.keyBufferOffset
         << ", .num_props = " << entry.numProps << " },\n";
    }
    os << "};\n";
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

/// For each unique SourceCoords, maintain a corresponding index, which can be
/// used to retrieve that source location information at runtime. Invalid
/// coordinates have an index of 0.
class SHSrcLocationTable {
 public:
  /// The type of the index.
  using IdxTy = uint32_t;
  /// Index representing an invalid or unknown location.
  static constexpr IdxTy kInvalidLocIdx = 0;

  SHSrcLocationTable(SHStringTable &stringTable) : stringTable_(stringTable) {
    // This first element in the table is reserved for invalid locations.
    insert({stringTable_.add(""), 0, 0});
  }

  /// Get the source location index for the given \p coords, creating one if it
  /// does not exist.
  IdxTy getIndex(
      const SourceErrorManager &sm,
      const hermes::SourceErrorManager::SourceCoords &coords) {
    if (!coords.isValid())
      return kInvalidLocIdx;
    // If this source location has not been seen before, use the next
    // index. Otherwise, use the existing entry.
    uint32_t fileIdx = getFileIdx(sm, coords.bufId);
    return insert({fileIdx, coords.line, coords.col});
  }

  /// \return how many unique source locations exist in the table.
  uint32_t size() const {
    return indicesMap_.size();
  }

  /// Turn the table of source locations into the corresponding SH C data
  /// structures.
  void generate(llvh::raw_ostream &OS, const SourceErrorManager &sm) const {
    OS << "\nstatic const SHSrcLoc s_source_locations[] = {\n";
    for (const auto &loc : locations_) {
      auto [fileIdx, line, col] = loc;
      OS.indent(2);
      OS << "{ .filename_idx = " << fileIdx << ", .line = " << line
         << ", .column = " << col << " },\n";
    }
    OS << "};\n";
  }

 private:
  /// Tuple type of <filenameStringID, line, column>. The filenameStringID is
  /// the index into the global string table to find the name of the file of a
  /// location.
  using LocationTy = std::tuple<uint32_t, uint32_t, uint32_t>;
  /// A type for a cache that maps from source coordinate buffer id to index in
  /// the global string table.
  using FileIdxCacheTy = llvh::DenseMap<uint32_t, uint32_t>;
  /// Map a tuple type to a unique index into the source location table.
  llvh::DenseMap<LocationTy, IdxTy> indicesMap_;
  /// This contains all the keys of indicesMap_, in insertion order.
  std::vector<LocationTy> locations_;
  /// A cache mapping buffer id to filename index into the global string table.
  FileIdxCacheTy fileIdxCache_{};
  /// To avoid performing a hash lookup in most cases, cache the last found
  /// entry in the file index cache.
  FileIdxCacheTy::value_type *lastFileIdxEntry_ = nullptr;

  /// A reference to the global string table. Used for filenames.
  SHStringTable &stringTable_;

  /// Insert a source location and \return its index into this location table.
  IdxTy insert(LocationTy loc) {
    uint32_t nextID = indicesMap_.size();
    const auto [iter, success] = indicesMap_.insert({loc, nextID});
    // If we successfully inserted, that means the element hasn't been seen
    // before. We should also append to the sorted vector.
    if (success) {
      locations_.push_back(loc);
    }
    return iter->second;
  }

  /// \return the index into the global string table for the corresponding name
  /// of \p bufId. The \p bufId *must* come from a valid source coordinate
  /// location.
  uint32_t getFileIdx(const SourceErrorManager &sm, uint32_t bufId) {
    // Fast path- we are probably comparing the same bufId repeatedly.
    if (LLVM_LIKELY(lastFileIdxEntry_ && lastFileIdxEntry_->first == bufId)) {
      return lastFileIdxEntry_->second;
    }

    // Slower path- check our cache to see if we already have this bufId's
    // string table index.
    auto it = fileIdxCache_.find(bufId);
    if (LLVM_LIKELY(it != fileIdxCache_.end())) {
      lastFileIdxEntry_ = &*it;
      return it->second;
    }

    // Slowest path- ask the global string table for the ID of the buffer
    // filename. This is potentially a very expensive operation since looking up
    // the filename in the string table potentially requires scanning it three
    // times.
    llvh::StringRef filename = sm.getSourceUrl(bufId);
    auto stringTableIdx = stringTable_.add(filename);
    it = fileIdxCache_.try_emplace(bufId, stringTableIdx).first;
    lastFileIdxEntry_ = &*it;
    return it->second;
  }
};

class SHNativeJSFunctionTable {
  /// A map of function pointer -> function id
  llvh::DenseMap<Function *, unsigned> funcMap_{};
  /// A reference to the global string table. Used for function names.
  SHStringTable &stringTable_;

 public:
  explicit SHNativeJSFunctionTable(Module *M, SHStringTable &stringTable)
      : stringTable_(stringTable) {
    // Ensure that the top level function has an id of 0.
    auto topLevelFunc = M->getTopLevelFunction();
    funcMap_[topLevelFunc] = 0;
    unsigned funcCounter = 1;
    // Iterate through all the functions in the module, assigning them unique
    // incrementing IDs.
    for (auto &F : *M) {
      if (&F != topLevelFunc) {
        funcMap_[&F] = funcCounter++;
      }
    }
  }

  /// \return the unique index for the given \p F.
  uint32_t getIndex(Function *F) const {
    return funcMap_.find(F)->second;
  }

  /// \return size of the function table.
  size_t size() const {
    return funcMap_.size();
  }

  /// Generates the correct label for Function \p F, and outputs it to \p OS. If
  /// the JS function name contains characters that aren't allowed in C
  /// identifiers, they will be replaced by '_'.
  void generateFunctionLabel(Function *F, llvh::raw_ostream &OS) const {
    OS << '_' << getIndex(F) << '_';

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

  /// Turn the table of function information into the corresponding SH C data
  /// structures.
  void generate(llvh::raw_ostream &OS) const {
    // Sort the keys by function index.
    std::vector<const Function *> sortedKeys{funcMap_.size()};
    for (auto &entry : funcMap_)
      sortedKeys[entry.second] = entry.first;

    OS << "\nstatic SHNativeFuncInfo s_function_info_table[] = {\n";
    for (const Function *F : sortedKeys) {
      uint32_t nameIdx = stringTable_.add(F->getOriginalOrInferredName().str());
      uint32_t argCount = F->getExpectedParamCountIncludingThis() - 1;
      auto kindVal = F->getKind();
      OS.indent(2);
      OS << "{ .name_index = " << nameIdx << ", .arg_count = " << argCount
         << ", .prohibit_invoke = "
         << computeProhibitInvoke(F->getProhibitInvoke())
         << ", .kind = " << computeFuncKind(kindVal) << " },\n";
    }
    OS << "};\n";
  }
};

struct ModuleGen {
  /// Table containing uniqued strings for the current module.
  SHStringTable stringTable{};

  /// Literal buffers for objects and arrays.
  SHLiteralBuffers literalBuffers;

  /// Maintain a table of all unique source file locations used by throwing
  /// instructions.
  SHSrcLocationTable srcLocationTable;

  /// Table of JS native functions
  SHNativeJSFunctionTable nativeFunctionTable;

  explicit ModuleGen(Module *M, bool optimizationEnabled)
      : literalBuffers{M, stringTable, optimizationEnabled},
        srcLocationTable{stringTable},
        nativeFunctionTable{M, stringTable} {}
};

/// \return true if the SHLegacyValue representations of values \p a and \p b
/// can be compared directly (bitwise).
static bool canCompareStrictEqualityRaw(Value *a, Value *b) {
  Type aType = a->getType();
  Type bType = b->getType();

  // If both can be numbers, then we can't compare because `-0` and `0` have
  // different bitwise representations.
  if (aType.canBeNumber() && bType.canBeNumber())
    return false;

  // If both can be bigint, then we can't compare because BigInts are compared
  // by values which are stored on the heap.
  if (aType.canBeBigInt() && bType.canBeBigInt())
    return false;

  // If both can be strings, then we can't compare because strings are compared
  // by their contents which are stored on the heap.
  if (aType.canBeString() && bType.canBeString())
    return false;

  // Otherwise, we can compare.
  return true;
}

class InstrGen {
 public:
  /// \p os is the output stream
  /// \p ra is the pre-ran register allocator for the current function
  InstrGen(
      llvh::raw_ostream &os,
      sh::SHRegisterAllocator &ra,
      const llvh::DenseMap<BasicBlock *, unsigned> &bbMap,
      Function &F,
      ModuleGen &moduleGen,
      uint32_t &nextCacheIdx,
      const llvh::DenseMap<BasicBlock *, size_t> &bbTryDepths)
      : os_(os),
        ra_(ra),
        bbMap_(bbMap),
        F_(F),
        nativeContext_(F.getContext().getNativeContext()),
        moduleGen_(moduleGen),
        nextCacheIdx_(nextCacheIdx),
        bbTryDepths_(bbTryDepths) {}

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

 private:
  /// The ostream used to output C code
  llvh::raw_ostream &os_;

  /// The register allocator that was created for the current function
  sh::SHRegisterAllocator &ra_;

  /// A map from basic blocks to unique numbers for identification
  const llvh::DenseMap<BasicBlock *, unsigned> &bbMap_;

  // The function being compiled.
  Function &F_;

  // Info related to native compilation.
  NativeContext &nativeContext_;

  /// The state for the module currently being emitted.
  ModuleGen &moduleGen_;

  /// Starts out at 0 and increments every time a cache index is used
  uint32_t &nextCacheIdx_;

  /// A map from basic blocks to their number of enclosing try statements
  /// (if non-zero).
  const llvh::DenseMap<BasicBlock *, size_t> &bbTryDepths_;

  void unimplemented(Instruction &inst) {
    std::string err{"Unimplemented "};
    err += inst.getName();
    F_.getParent()->getContext().getSourceErrorManager().error(
        inst.getLocation(), err);
    // This can optionally be disabled.
    hermes_fatal(err);
  }

  /// Generate a comment containing the string \p str escaped and truncated
  /// to 20 characters.
  llvh::raw_ostream &genStringComment(llvh::StringRef str) {
    os_ << " /*";

    // Escape */ specially, since we are in a comment.
    auto escape = [this](llvh::StringRef str) {
      size_t from = 0;
      size_t pos;
      while ((pos = str.find("*/", from)) != llvh::StringRef::npos) {
        // Write the part of the string including the "*" followed by "\/".
        os_.write_escaped(str.slice(from, pos + 1)) << "\\057";
        from = pos + 2;
      }
      os_.write_escaped(str.substr(from));
    };
    if (str.size() > 20) {
      escape(str.take_front(20));
      os_ << "...";
    } else {
      escape(str);
    }
    return os_ << "*/";
  }

  /// Generate a string constant by referencing the global string table.
  llvh::raw_ostream &genStringConst(LiteralString *LS) {
    auto str = LS->getValue().str();
    os_ << "get_symbols(shUnit)[" << moduleGen_.stringTable.add(str) << ']';
    return genStringComment(str);
  }
  /// Generate a string constant, followed by an optional value (if non-null),
  /// and a cache index. This must be used when
  /// passing parameters to API functions that will use the cache index.
  llvh::raw_ostream &genStringConstIC(
      LiteralString *LS,
      Value *optValue = nullptr) {
    genStringConst(LS);
    if (optValue) {
      os_ << ", ";
      generateRegisterPtr(*optValue);
    }
    os_ << ", ";
    return genIC(LS);
  }

  /// Generate a cache index. This must be used when passing parameters to API
  /// functions that will use the cache index.
  /// \p LS is currently unused. In the future, it might be used in the
  /// optimization controlled by OptimizationSettings::reusePropCache: that
  /// different instructions accessing the same property probably are for
  /// objects of the same hidden class, and should get the same offset.
  llvh::raw_ostream &genIC(LiteralString *LS) {
    return os_ << "get_prop_cache(shUnit) + " << nextCacheIdx_++;
  }

  /// Helper to generate a value in a register,
  void generateRegister(sh::Register reg) {
    switch (reg.getClass()) {
      case sh::RegClass::LocalPtr:
        os_ << "locals.t" << reg.getIndex();
        break;

      case sh::RegClass::LocalNonPtr:
        os_ << "np" << reg.getIndex();
        break;

      case sh::RegClass::RegStack:
        os_ << "frame[" << (hbc::StackFrameLayout::FirstLocal + reg.getIndex())
            << ']';
        break;

      default:
        hermes_fatal("unimplemented reg class");
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
    if (llvh::isa<LiteralUndefined>(&val) || llvh::isa<LiteralUninit>(&val)) {
      os_ << "_sh_ljs_undefined()";
    } else if (llvh::isa<LiteralNull>(&val)) {
      os_ << "_sh_ljs_null()";
    } else if (llvh::isa<LiteralEmpty>(&val)) {
      os_ << "_sh_ljs_empty()";
    } else if (auto B = llvh::dyn_cast<LiteralBool>(&val)) {
      os_ << "_sh_ljs_bool(" << boolStr(B->getValue()) << ")";
    } else if (auto LN = llvh::dyn_cast<LiteralNumber>(&val)) {
      os_ << "_sh_ljs_double(";
      int32_t intval;
      if (!LN->isNegativeZero() &&
          sh_tryfast_f64_to_i32(LN->getValue(), intval)) {
        os_ << intval;
      } else {
        os_ << "((struct HermesValueBase){.raw = "
            << llvh::DoubleToBits(LN->getValue()) << "u}).f64";
      }
      os_ << ")";
    } else if (auto S = llvh::dyn_cast<LiteralString>(&val)) {
      os_ << "_sh_ljs_get_string(shr, ";
      genStringConst(S) << ")";
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
    } else if (auto *NE = llvh::dyn_cast<LiteralNativeExtern>(&val)) {
      os_ << "_sh_ljs_native_pointer(" << NE->getData()->name()->str() << ")";
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
      os_ << "_sh_ljs_double(_sh_ljs_to_int32_rjs(shr, ";
      generateRegisterPtr(*inst.getSingleOperand());
      os_ << "));\n";
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
    os_.indent(2);
    generateRegister(dstReg);
    os_ << " = ";
    generateValue(*inst.getSingleOperand());
    os_ << ";\n";
  }
  void generateImplicitMovInst(ImplicitMovInst &inst) {
    // ImplicitMovs produce no bytecode, they only express that a subsequent
    // instruction will perform the equivalent of a 'Mov'.
    os_ << "  // ImplicitMovInst\n";
  }
  void generateTypeOfInst(TypeOfInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_typeof(shr, ";
    generateRegisterPtr(*inst.getArgument());
    os_ << ");\n";
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
    os_ << "  _sh_ljs_declare_global_var(shr, ";
    genStringConst(inst.getName()) << ");\n";
  }
  void generateLoadFrameInst(LoadFrameInst &inst) {
    os_.indent(2);
    generateValue(inst);
    os_ << " = _sh_ljs_load_from_env(";
    generateValue(*inst.getScope());
    os_ << ", " << inst.getLoadVariable()->getIndexInVariableList() << ");\n";
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
  void generateGetParentScopeInst(GetParentScopeInst &inst) {
    os_.indent(2);
    generateRegister(inst);

    os_ << " = _sh_ljs_get_env_from_closure(shr, frame["
        << hbc::StackFrameLayout::CalleeClosureOrCB << "]);";
  }
  void generateResolveScopeInst(ResolveScopeInst &inst) {
    hermes_fatal("ResolveScopeInst should have been lowered.");
  }
  void generateLIRResolveScopeInst(LIRResolveScopeInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_env(shr, ";
    generateRegister(*inst.getStartScope());
    os_ << ", " << inst.getNumLevels()->asUInt32() << ");\n";
  }
  void generateHBCResolveParentEnvironmentInst(
      HBCResolveParentEnvironmentInst &inst) {
    hermes_fatal("HBCResolveEnvironment is not used by SH.");
  }
  void generateGetClosureScopeInst(GetClosureScopeInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_get_env_from_closure(shr, ";
    generateRegister(*inst.getClosure());
    os_ << ");";
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
    os_ << "  // PhiInst\n";
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
    // Infix operator for raw bitwise comparison.
    const char *infixRawOp = nullptr;
    // Function call for operator for doubles.
    const char *funcDoubleOp = nullptr;
    // Function call for operator for int32.
    const char *funcInt32Op = nullptr;

    bool bothDouble = inst.getLeftHandSide()->getType().isNumberType() &&
        inst.getRightHandSide()->getType().isNumberType();

    // NOTE: this used to check whether we know that both operands are numbers
    // in the int32 range. We no longer track that information, so for now this
    // is hardcoded to false.
    const bool bothInt32 = false;

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
        } else if (canCompareStrictEqualityRaw(
                       inst.getLeftHandSide(), inst.getRightHandSide())) {
          infixRawOp = "!=";
        } else {
          funcUntypedOp = "!_sh_ljs_strict_equal";
          passByValue = true;
        }
        boolConv = true;
        break;
      case ValueKind::BinaryStrictlyEqualInstKind: // ===
        if (bothDouble) {
          infixDoubleOp = "==";
        } else if (canCompareStrictEqualityRaw(
                       inst.getLeftHandSide(), inst.getRightHandSide())) {
          infixRawOp = "==";
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
    } else if (infixRawOp) {
      os_ << "";
      generateRegister(*inst.getLeftHandSide());
      os_ << ".raw " << infixRawOp << " ";
      generateRegister(*inst.getRightHandSide());
      os_ << ".raw";
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
      os_ << ", ";
      genStringConstIC(LS, inst.getStoredValue()) << ");\n";
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
    genStringConstIC(propStr, inst.getStoredValue()) << ");\n";
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
    genStringConst(propStr) << ", ";
    generateRegisterPtr(*inst.getStoredValue());
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
      genStringConst(propStr) << ");\n";
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
      os_ << ",";
      genStringConstIC(LS) << ");\n";
      return;
    }
    // If the prop is an index-like constant, generate the special bytecode.
    if (auto *litNum = llvh::dyn_cast<LiteralNumber>(inst.getProperty())) {
      if (auto idxOpt = doubleToArrayIndex(litNum->getValue())) {
        os_ << "_sh_ljs_get_by_index_rjs(shr,&";
        generateRegister(*inst.getObject());
        os_ << ", ";
        os_ << *idxOpt << ");\n";
        return;
      }
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
    os_ << ", ";
    genStringConstIC(LS) << ");\n";
  }
  void generateLoadParentNoTrapsInst(LoadParentNoTrapsInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_load_parent_no_traps(shr, ";
    generateRegister(*inst.getObject());
    os_ << ");\n";
  }
  void generateTypedLoadParentInst(TypedLoadParentInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    os_ << "_sh_typed_load_parent(shr, ";
    generateRegisterPtr(*inst.getObject());
    os_ << ");\n";
  }
  void generateTypedStoreParentInst(TypedStoreParentInst &inst) {
    os_.indent(2);
    os_ << "_sh_typed_store_parent(shr, ";
    generateRegisterPtr(*inst.getStoredValue());
    os_ << ", ";
    generateRegisterPtr(*inst.getObject());
    os_ << ");\n";
  }
  void generateFUnaryMathInst(FUnaryMathInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_double(";
    switch (inst.getKind()) {
      case ValueKind::FNegateKind:
        os_ << "-_sh_ljs_get_double(";
        generateValue(*inst.getArg());
        os_ << ")";
        break;
      default:
        llvm_unreachable("invalid FUnaryMath");
    }
    os_ << ");\n";
  }
  void generateFBinaryMathInst(FBinaryMathInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";

    // Handle FModuloInst separately because it needs a function call
    // and doesn't fit neatly into a C binary operator.
    if (inst.getKind() == ValueKind::FModuloInstKind) {
      os_ << "_sh_ljs_double(_sh_mod_double(_sh_ljs_get_double(";
      generateValue(*inst.getLeft());
      os_ << "), _sh_ljs_get_double(";
      generateValue(*inst.getRight());
      os_ << ")));\n";
      return;
    }

    os_ << "_sh_ljs_double(_sh_ljs_get_double(";
    generateValue(*inst.getLeft());
    os_ << ") ";
    switch (inst.getKind()) {
      case ValueKind::FAddInstKind:
        os_ << "+";
        break;
      case ValueKind::FSubtractInstKind:
        os_ << "-";
        break;
      case ValueKind::FMultiplyInstKind:
        os_ << "*";
        break;
      case ValueKind::FDivideInstKind:
        os_ << "/";
        break;
      default:
        llvm_unreachable("invalid FBinaryMath");
    }
    os_ << " _sh_ljs_get_double(";
    generateValue(*inst.getRight());
    os_ << "));\n";
  }
  void generateFCompareInst(FCompareInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";

    os_ << "_sh_ljs_bool(_sh_ljs_get_double(";
    generateValue(*inst.getLeft());
    os_ << ") ";
    switch (inst.getKind()) {
      case ValueKind::FEqualInstKind:
        os_ << "==";
        break;
      case ValueKind::FNotEqualInstKind:
        os_ << "!=";
        break;
      case ValueKind::FLessThanInstKind:
        os_ << "<";
        break;
      case ValueKind::FLessThanOrEqualInstKind:
        os_ << "<=";
        break;
      case ValueKind::FGreaterThanInstKind:
        os_ << ">";
        break;
      case ValueKind::FGreaterThanOrEqualInstKind:
        os_ << ">=";
        break;
      default:
        llvm_unreachable("invalid FBinaryMath");
    }
    os_ << " _sh_ljs_get_double(";
    generateValue(*inst.getRight());
    os_ << "));\n";
  }
  void generateHBCFCompareBranchInst(HBCFCompareBranchInst &inst) {
    hermes_fatal("HBCFCompareBranchInst not generated in native backend");
  }
  void generateStringConcatInst(StringConcatInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = ";
    os_ << "_sh_string_concat(shr, " << inst.getNumOperands() << ", ";
    for (int i = 0, e = inst.getNumOperands(); i < e; ++i) {
      if (i != 0)
        os_ << ", ";
      generateRegisterPtr(*inst.getOperand(i));
    }
    os_ << ");\n";
  }
  void generateHBCStringConcatInst(HBCStringConcatInst &inst) {
    hermes_fatal("HBCStringConcatInst not generated in native backend");
  }
  void generateStoreStackInst(StoreStackInst &inst) {
    hermes_fatal("StoreStackInst should have been lowered.");
  }
  void generateStoreFrameInst(StoreFrameInst &inst) {
    os_ << "  _sh_ljs_store_to_env(shr, ";
    generateValue(*inst.getScope());
    os_ << ",";
    generateValue(*inst.getValue());
    os_ << ", " << inst.getVariable()->getIndexInVariableList() << ");\n";
  }
  void generateAllocStackInst(AllocStackInst &inst) {
    // This is a no-op.
    os_ << "  // AllocStackInst\n";
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
      auto bufIndex =
          moduleGen_.literalBuffers.serializedLiteralOffsetFor(&inst);

      os_ << "_sh_ljs_new_array_with_buffer(shr, shUnit, ";
      os_ << sizeHint << ", ";
      os_ << elementCount << ", ";
      os_ << bufIndex.valueBufferOffset << ")";
    }
    os_ << ";\n";
  }
  void generateAllocFastArrayInst(AllocFastArrayInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_new_fastarray(shr, " << inst.getCapacity()->asUInt32()
        << ");\n";
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
    os_ << "_sh_get_template_object(shr, shUnit, " << inst.getTemplateObjID()
        << ", " << boolStr(inst.isDup()) << ", " << argCount;
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
    assert(
        inst.getKeyValuePairCount() == 0 &&
        "AllocObjectLiteralInst with properties should be lowered to HBCAllocObjectFromBufferInst");
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
  void generateCreateArgumentsLooseInst(CreateArgumentsLooseInst &inst) {
    hermes_fatal("CreateArgumentsLooseInst should have been lowered.");
  }
  void generateCreateArgumentsStrictInst(CreateArgumentsStrictInst &inst) {
    hermes_fatal("CreateArgumentsStrictInst should have been lowered.");
  }
  void generateCatchInst(CatchInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_catch(shr, (SHLocals*)&locals, frame, "
        << (ra_.getMaxArgumentRegisters() + hbc::StackFrameLayout::FirstLocal)
        << ");\n";
  }
  void generateDebuggerInst(DebuggerInst &inst) {
    hermes_fatal("DebuggerInst should have been deleted.");
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
    os_ << llvh::format("get_symbols(shUnit)[%u]", patternStrID);
    os_ << ", ";
    os_ << llvh::format("get_symbols(shUnit)[%u]", flagsStrID);
    os_ << ");\n";
  }
  void generateTryEndInst(TryEndInst &inst) {
    os_ << "  _sh_end_try(shr, &jmpBuf"
        << bbTryDepths_.lookup(inst.getBranchDest()) << ");\n";
    os_ << "  goto ";
    generateBasicBlockLabel(inst.getBranchDest(), os_, bbMap_);
    os_ << ";";
  }
  void generateGetNewTargetInst(GetNewTargetInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = frame[" << hbc::StackFrameLayout::NewTarget << "];\n";
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
  void generateUnreachableInst(UnreachableInst &inst) {
    os_.indent(2);
    os_ << "_sh_unreachable();\n";
  }
  void generateCreateFunctionInst(CreateFunctionInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_create_closure" << "(shr, &";
    generateRegister(*inst.getScope());
    os_ << ", ";
    moduleGen_.nativeFunctionTable.generateFunctionLabel(
        inst.getFunctionCode(), os_);
    os_ << ", ";
    os_ << "&s_function_info_table["
        << moduleGen_.nativeFunctionTable.getIndex(inst.getFunctionCode())
        << "]" << ", shUnit);\n";
  }
  void generateCreateGeneratorInst(CreateGeneratorInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_create_generator_object" << "(shr, &";
    generateRegister(*inst.getScope());
    os_ << ", ";
    moduleGen_.nativeFunctionTable.generateFunctionLabel(
        inst.getFunctionCode(), os_);
    os_ << ", ";
    os_ << "&s_function_info_table["
        << moduleGen_.nativeFunctionTable.getIndex(inst.getFunctionCode())
        << "]" << ", shUnit);\n";
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
  void generateThrowTypeErrorInst(ThrowTypeErrorInst &inst) {
    os_ << "  _sh_throw_type_error(shr, ";
    generateRegisterPtr(*inst.getMessage());
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
    os_ << "  if(_sh_try(shr, &jmpBuf" << bbTryDepths_.lookup(inst.getParent())
        << ") == 0) goto ";
    generateBasicBlockLabel(inst.getTryBody(), os_, bbMap_);
    os_ << ";\n";
    os_ << "  goto ";
    generateBasicBlockLabel(inst.getCatchTarget(), os_, bbMap_);
    os_ << ";\n";
  }
  void generateHBCCompareBranchInst(HBCCompareBranchInst &inst) {
    unimplemented(inst);
  }
  void generateSwitchImmInst(SwitchImmInst &inst) {
    unimplemented(inst);
  }
  void generateSaveAndYieldInst(SaveAndYieldInst &inst) {
    unimplemented(inst);
  }
  /// Populate the outgoing registers which are constants.
  /// These aren't added by LowerCalls, they're not representable in lowered IR.
  /// We want to set them up to be able to do the call without going through
  /// doCall.
  void setupCallInline(BaseCallInst &inst) {
    auto stackReg = [maxArgsRegs = ra_.getMaxArgumentRegisters()](
                        int32_t index) -> sh::Register {
      return sh::Register(
          sh::RegClass::RegStack, (sh::RegIndex)((int32_t)maxArgsRegs + index));
    };

    os_.indent(2);
    generateRegister(stackReg(hbc::StackFrameLayout::PreviousFrame));
    os_ << " = _sh_ljs_native_pointer(frame);\n";

    os_.indent(2);
    generateRegister(stackReg(hbc::StackFrameLayout::SavedIP));
    os_ << " = _sh_ljs_native_pointer((void*)0);\n";

    os_.indent(2);
    generateRegister(stackReg(hbc::StackFrameLayout::SavedCodeBlock));
    os_ << " = _sh_ljs_native_pointer((void*)0);\n";

    os_.indent(2);
    generateRegister(stackReg(hbc::StackFrameLayout::SHLocals));
    os_ << " = _sh_ljs_native_pointer((void*)0);\n";

    os_.indent(2);
    generateRegister(stackReg(hbc::StackFrameLayout::ArgCount));
    os_ << " = _sh_ljs_native_uint32(" << (inst.getNumArguments() - 1)
        << ");\n";

    // new.target is set up in LowerCalls.
  }
  void generateCallInst(CallInst &inst) {
    if (inst.getAttributes(inst.getModule()).isNativeJSFunction) {
      // Fast paths for calling NativeJSFunction.
      setupCallInline(inst);
      os_.indent(2);
      generateRegister(inst);
      os_ << " = ";
      if (auto *targetFunc = llvh::dyn_cast<Function>(inst.getTarget())) {
        // Fast path, avoid all indirection and just call the function.
        moduleGen_.nativeFunctionTable.generateFunctionLabel(targetFunc, os_);
        os_ << "(shr);\n";
      } else {
        // Avoid doCall and perform a legacy call on the pointer.
        os_ << "((SHNativeJSFunction *)_sh_ljs_get_pointer(";
        generateValue(*inst.getCallee());
        os_ << "))->functionPtr(shr);\n";
      }
    } else {
      os_.indent(2);
      generateRegister(inst);
      os_ << " = _sh_ljs_call(shr, frame, " << inst.getNumArguments() - 1
          << ");\n";
    }
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
  void generateHBCCallWithArgCountInst(HBCCallWithArgCountInst &inst) {
    unimplemented(inst);
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
  void generateCreateScopeInst(CreateScopeInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_create_environment(shr, ";
    if (llvh::isa<EmptySentinel>(inst.getParentScope()))
      os_ << "NULL";
    else
      generateRegisterPtr(*inst.getParentScope());
    os_ << ", " << inst.getVariableScope()->getVariables().size() << ");\n";
  }
  void generateHBCCreateFunctionEnvironmentInst(
      HBCCreateFunctionEnvironmentInst &inst) {
    hermes_fatal("HBCCreateEnvironmentInst is not used by SH.");
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
    assert(
        llvh::isa<EmptySentinel>(inst.getNewTarget()) &&
        "CreateThis currently only supported for `new`");
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_create_this(shr, &";
    generateRegister(*inst.getClosure());
    os_ << ", &";
    generateRegister(*inst.getClosure());
    os_ << ", ";
    Module *M = F_.getParent();
    auto *protoStr =
        M->getLiteralString(M->getContext().getIdentifier("prototype"));
    genIC(protoStr);
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

    auto [shapeIdx, valIdx] =
        moduleGen_.literalBuffers.serializedLiteralOffsetFor(&inst);

    os_ << " = ";
    os_ << "_sh_ljs_new_object_with_buffer(shr, shUnit, ";
    os_ << shapeIdx << ", ";
    os_ << valIdx << ")";
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
      suffix = "_bool";
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
  void generateUnionNarrowTrustedInst(UnionNarrowTrustedInst &inst) {
    // Since all values are currently NaN-boxed, narrowing is just a move.
    // TODO(T155912625): Revisit this once union narrow lowering is fixed.
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

  /// \return the name of the SH function for checking whether a value is of
  /// a specific type.
  static llvh::StringLiteral nameOfFunctionCheckingForType(Type type) {
    assert(!type.isNoType() && "type must be non-zero");
    switch (type.getFirstTypeKind()) {
      case Type::Empty:
        return "_sh_ljs_is_empty";
      case Type::Uninit:
      case Type::Undefined:
        return "_sh_ljs_is_undefined";
      case Type::Null:
        return "_sh_ljs_is_null";
      case Type::Boolean:
        return "_sh_ljs_is_bool";
      case Type::String:
        return "_sh_ljs_is_string";
      case Type::Number:
        return "_sh_ljs_is_double";
      case Type::BigInt:
        return "_sh_ljs_is_bigint";
      case Type::Environment:
        hermes_fatal("cannot check for environment type");
      case Type::FunctionCode:
        hermes_fatal("cannot check for functionCode type");
      case Type::Object:
        return "_sh_ljs_is_object";
      case Type::LAST_TYPE:
        break;
    }
    hermes_fatal("invalid type for checking");
  }

  /// \param badTypes the types which cause an exception if they are the input.
  void _typeCastHelper(
      Type resultType,
      Type badTypes,
      sh::Register dstReg,
      sh::Register srcReg,
      const char *errorExpr) {
    os_.indent(2);
    os_ << "if (";

    // Are there fewer "bad" types than "good" types? That determines which we
    // check.
    auto [checkTypes, negativeCheck] =
        badTypes.countTypes() < resultType.countTypes()
        ? std::make_pair(badTypes, true)
        : std::make_pair(resultType, false);

    if (!negativeCheck)
      os_ << "!(";
    {
      bool first = true;
      for (Type t : checkTypes) {
        if (!first)
          os_ << " || ";
        os_ << nameOfFunctionCheckingForType(t) << '(';
        generateRegister(srcReg);
        os_ << ')';
        first = false;
      }
    }
    if (!negativeCheck)
      os_ << ')';

    os_ << ") " << errorExpr << ";\n";

    // Set the result, but do nothing if the registers are the same.
    if (dstReg != srcReg) {
      os_.indent(2);
      generateRegister(dstReg);
      os_ << " = ";
      generateRegister(srcReg);
      os_ << ";\n";
    }
  }
  void generateCheckedTypeCastInst(CheckedTypeCastInst &inst) {
    const Type resultType = inst.getType();
    const Type inputType = inst.getCheckedValue()->getType();

    assert(
        ra_.isAllocated(inst.getCheckedValue()) &&
        "operand of CheckedCastInst must have an allocated register");
    sh::Register srcReg = ra_.getRegister(inst.getCheckedValue());
    sh::Register dstReg = ra_.getRegister(&inst);

    // Are the input and output type the same?
    if (inputType.isSubsetOf(resultType)) {
      // If so, just move the value, but do nothing if the registers are the
      // same.
      if (dstReg != srcReg) {
        os_.indent(2);
        generateRegister(dstReg);
        os_ << " = ";
        generateValue(*inst.getCheckedValue());
        os_ << ";\n";
      }
      return;
    }

    // TODO: generate a type-specific error.
    _typeCastHelper(
        resultType,
        Type::subtractTy(inputType, resultType),
        dstReg,
        srcReg,
        "_sh_throw_type_error_ascii(shr, \"Checked cast failed\")");
  }
  void generateThrowIfInst(ThrowIfInst &inst) {
    assert(
        ra_.isAllocated(inst.getCheckedValue()) &&
        "operand of ThrowIf must have an allocated register");

    Type badTypes = inst.getInvalidTypes()->getData();
    assert(
        !badTypes.isNoType() &&
        badTypes.isSubsetOf(
            Type::unionTy(Type::createEmpty(), Type::createUninit())) &&
        "invalidTypes set can only contain Empty or Uninit");

    _typeCastHelper(
        inst.getType(),
        badTypes,
        ra_.getRegister(&inst),
        ra_.getRegister(inst.getCheckedValue()),
        "_sh_throw_empty(shr)");
  }
  void generateLIRDeadValueInst(LIRDeadValueInst &inst) {
    os_.indent(2);
    os_ << "__builtin_unreachable();\n";
    os_.indent(2);
    // Initialize the register with whatever.
    generateRegister(inst);
    os_ << " = _sh_ljs_double(0);\n";
  }
  void generateFastArrayLoadInst(FastArrayLoadInst &inst) {
    os_.indent(2);
    generateValue(inst);
    os_ << " = ";
    os_ << "_sh_fastarray_load(shr, ";
    generateRegisterPtr(*inst.getArray());
    os_ << ", _sh_ljs_get_double(";
    generateRegister(*inst.getIndex());
    os_ << "));\n";
  }
  void generateFastArrayStoreInst(FastArrayStoreInst &inst) {
    os_.indent(2);
    os_ << "_sh_fastarray_store(shr, ";
    generateRegisterPtr(*inst.getStoredValue());
    os_ << ", ";
    generateRegisterPtr(*inst.getArray());
    os_ << ", _sh_ljs_get_double(";
    generateRegister(*inst.getIndex());
    os_ << "));\n";
  }
  void generateFastArrayPushInst(FastArrayPushInst &inst) {
    os_.indent(2);
    os_ << "_sh_fastarray_push(shr, ";
    generateRegisterPtr(*inst.getPushedValue());
    os_ << ", ";
    generateRegisterPtr(*inst.getArray());
    os_ << ");\n";
  }
  void generateFastArrayAppendInst(FastArrayAppendInst &inst) {
    os_.indent(2);
    os_ << "_sh_fastarray_append(shr, ";
    generateRegisterPtr(*inst.getOther());
    os_ << ", ";
    generateRegisterPtr(*inst.getArray());
    os_ << ");\n";
  }
  void generateFastArrayLengthInst(FastArrayLengthInst &inst) {
    os_.indent(2);
    generateValue(inst);
    os_ << " = ";
    os_ << "_sh_fastarray_length(shr, ";
    generateRegisterPtr(*inst.getArray());
    os_ << ");\n";
  }
  void generateGetNativeRuntimeInst(GetNativeRuntimeInst &inst) {
    os_.indent(2);
    generateRegister(inst);
    os_ << " = _sh_ljs_native_pointer(shr);\n";
  }
  void generateNativeCallInst(NativeCallInst &inst) {
    NativeSignature *sig = inst.getSignature()->getData();
    unsigned parens = 0;

    os_.indent(2);
    if (sig->result() != NativeCType::c_void) {
      generateValue(inst);
      os_ << " = ";
      convertFromNativeResult(sig->result(), parens);
    }

    // If the callee is a literal, generate simpler code.
    if (auto *ne = llvh::dyn_cast<LiteralNativeExtern>(inst.getCallee())) {
      os_ << ne->getData()->name()->str();
    } else {
      os_ << "((";
      sig->format(os_);
      os_ << ")_sh_ljs_get_native_pointer(";
      generateValue(*inst.getCallee());
      os_ << "))";
    }
    os_ << '(';
    for (unsigned i = 0, e = inst.getNumArgs(); i != e; ++i) {
      if (i)
        os_ << ", ";
      convertToNativeArg(sig->params()[i], inst.getArg(i));
    }
    // End of argument list.
    os_ << ')';
    while (parens--)
      os_ << ')';

    os_ << ";\n";

    // If the function returns void, we must synthesize an 'undefined'.
    if (sig->result() == NativeCType::c_void) {
      os_.indent(2);
      generateValue(inst);
      os_ << " = _sh_ljs_undefined();\n";
    }
  }
  void generateLazyCompilationDataInst(LazyCompilationDataInst &inst) {
    hermes_fatal("lazy compilation unsupported in native backend");
  }
  void generateEvalCompilationDataInst(EvalCompilationDataInst &inst) {
    hermes_fatal("eval compilation unsupported in native backend");
  }
  /// Print code to convert the result of a native call to its corresponding
  /// JS type.
  ///
  /// \param ctype the native type of the result.
  /// \param parens the number of closing parentheses to print after. This
  ///     value is in-out.
  void convertFromNativeResult(NativeCType ctype, unsigned &parens) {
    if (ctype == NativeCType::c_void || ctype == NativeCType::c_hermes_value)
      return;

    MachineType mt = nativeContext_.md.mapCType(ctype);
    const MachineDesc::MTD &mtd = nativeContext_.md.getMTD(mt);

    ++parens;
    switch (mtd.cat) {
      case MachineDesc::Category::Int:
        // Integer types with sizes of 6 bytes or fewer can always fit in a
        // double.
        if (mtd.size <= 6) {
          os_ << "_sh_ljs_double((double)(" << ctype << ')';
        } else {
          ++parens;
          if (mtd.sign)
            os_ << "_sh_ljs_double(_sh_to_double_int64_or_throw(shr, ";
          else
            os_ << "_sh_ljs_double(_sh_to_double_uint64_or_throw(shr, ";
        }
        break;
      case MachineDesc::Category::FP:
        // Note that we treat floats as untrusted, because it is not defined
        // how the bit pattern of a float is transferred to a double.
        os_ << "_sh_ljs_untrusted_double(";
        break;
      case MachineDesc::Category::Ptr:
        os_ << "_sh_ljs_native_pointer_or_throw(shr, ";
        break;
    }
  }
  /// Convert a JS value to its corresponding native argument type.
  void convertToNativeArg(NativeCType ctype, Value *arg) {
    MachineType mt = nativeContext_.md.mapCType(ctype);
    const MachineDesc::MTD &mtd = nativeContext_.md.getMTD(mt);
    int parens = 1;

    switch (mtd.cat) {
      case MachineDesc::Category::Int:
        // If the integer value fits in int32, use the normal truncating
        // machinery.
        if (mtd.size <= 4) {
          os_ << '(' << ctype << ")_sh_to_int32_double(_sh_ljs_get_double(";
          ++parens;
        } else {
          // If the integer value is larger than 32 bits, we must check whether
          // it is within the integer range that can be safely represented in
          // a double, and throw otherwise.
          assert(mtd.size == 8 && "Invalid integer size");
          if (mtd.sign)
            os_ << "_sh_to_int64_double_or_throw(shr, _sh_ljs_get_double(";
          else
            os_ << "_sh_to_uint64_double_or_throw(shr, _sh_ljs_get_double(";
          ++parens;
        }
        break;
      case MachineDesc::Category::FP:
        assert(
            (mt == MachineType::f32 || mt == MachineType::f64) &&
            "invalid FP type");
        if (mt == MachineType::f32)
          os_ << "(float)";
        os_ << "_sh_ljs_get_double(";
        break;
      case MachineDesc::Category::Ptr:
        os_ << "_sh_ljs_get_native_pointer(";
        break;
    }

    generateValue(*arg);
    while (parens--)
      os_ << ')';
  }
};

/// Lower module IR to LIR, so it is suitable for register allocation.
/// \return true if lowering completed successfully.
bool lowerModuleIR(Module *M, bool optimize) {
  PassManager PM("SH Lower");
  PM.addLowerGeneratorFunction();
  // Lowering ExponentiationOperator and ThrowTypeError (in PeepholeLowering)
  // needs to run before LowerBuiltinCalls because it introduces calls to
  // HermesInternal.
  PM.addPass(sh::createPeepholeLowering());
  // LowerBuiltinCalls needs to run before the rest of the lowering.
  PM.addPass(new LowerBuiltinCalls());
  PM.addPass(new LowerNumericProperties());
  PM.addPass(new LowerAllocObjectLiteral());
  PM.addPass(new hbc::LowerArgumentsArray());
  PM.addPass(new LimitAllocArray(UINT16_MAX));
  PM.addPass(new hbc::DedupReifyArguments());
  // TODO Consider supporting LowerSwitchIntoJumpTables for optimization
  PM.addPass(new SwitchLowering());
  if (optimize) {
    // TODO(T204084366): TypeInference must run before OptEnvironmentInit,
    // because the latter will remove stores that may affect the inferred type.
    PM.addTypeInference();
    // OptEnvironmentInit checks for LiteralUndefined, so it needs to run before
    // LoadConstants.
    PM.addPass(createOptEnvironmentInit());
  }
  PM.addPass(sh::createLoadConstants());
  PM.addPass(createLowerScopes());
  if (optimize) {
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
  return PM.run(M);
}

/// Perform final lowering of a register-allocated function's IR.
void lowerAllocatedFunctionIR(
    Function *F,
    sh::SHRegisterAllocator &RA,
    bool optimize) {
  PassManager PM("SH LowerAllocatedFunctionIR");
  PM.addPass(new LowerStoreInstrs(RA));
  PM.addPass(sh::createLowerCalls(RA));
  if (optimize) {
    PM.addPass(new MovElimination(RA));
    PM.addPass(sh::createRecreateCheapValues(RA));
  }
  PM.addPass(new RemoveMovs(RA));
  PM.run(F);
}

/// Converts Function \p F into valid C code and outputs it through \p OS.
void generateFunction(
    Function &F,
    hermes::sh::LineDirectiveEmitter &OS,
    ModuleGen &moduleGen,
    uint32_t &nextCacheIdx,
    BytecodeGenerationOptions options) {
  auto PO = hermes::postOrderAnalysis(&F);

  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());

  sh::SHRegisterAllocator RA(&F);
  RA.allocate(order);

  if (options.format == DumpRA) {
    RA.dump(order);
    return;
  }

  lowerAllocatedFunctionIR(&F, RA, options.optimizationEnabled);

  if (options.format == DumpLRA) {
    RA.dump(order);
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

  auto &srcMgr = F.getContext().getSourceErrorManager();
  OS << "// ";
  srcMgr.dumpCoords(OS, F.getSourceRange().Start);
  OS << '\n';

  // Compute the try nesting depth of each basic block.
  auto [bbTryDepths, maxTryDepth] = getBlockTryDepths(&F);

  InstrGen instrGen(OS, RA, bbMap, F, moduleGen, nextCacheIdx, bbTryDepths);

  // Number of registers stored in the `locals` struct below.
  uint32_t localsSize = RA.getMaxRegisterUsage(sh::RegClass::LocalPtr);

  OS << "static SHLegacyValue ";
  moduleGen.nativeFunctionTable.generateFunctionLabel(&F, OS);
  OS << "(SHRuntime *shr) {\n";

  bool emitLineDirectives = options.emitLineDirectives;
  // Emit source location comments if requested, and we are *not* emitting line
  // directives. Otherwise, the resulting C is too noisy and unreadable.
  bool emitSrcLocComments = options.emitSourceLocations && !emitLineDirectives;
  if (emitLineDirectives) {
    OS.enableLineDirectives();
    // Initialize the location of the directive statement to source location of
    // the current function. This way, if the first BB initially doesn't have
    // instructions with source information, this value will provide a good
    // fallback.
    OS.setDirectiveInfo(F.getSourceRange().Start, srcMgr);
  }

  // In the global function, ensure that we are linking to the correct
  // library.
  if (F.isGlobalScope())
    OS << "  _SH_MODEL();\n";

  OS << "  struct {\n    SHLocals head;\n";

  for (size_t i = 0; i < localsSize; ++i) {
    OS << "    SHLegacyValue t" << i << ";\n";
  }

  OS << "  } locals;\n";

  // Emit the stack check if necessary.
  if (F.getParent()
          ->getContext()
          .getNativeContext()
          .settings.emitCheckNativeStack) {
    // This call contains the fast path out of line for now,
    // but _sh_check_native_stack_overflow can be updated to contain inline
    // code and avoid the function call in the future, when SHRuntime
    // has better visibility into the fields of vm::Runtime.
    OS << "  _sh_check_native_stack_overflow(shr);\n";
  }

  // Emit instructions for native stack traces if the debug info level is set to
  // at least throwing.
  bool emitNativeTraces =
      F.getContext().getDebugInfoSetting() >= DebugInfoSetting::THROWING;

  OS << "  SHLegacyValue *frame = _sh_enter(shr, &locals.head, "
     << (RA.getMaxArgumentRegisters() + hbc::StackFrameLayout::FirstLocal)
     << ");\n"
     << "  locals.head.count =" << localsSize << ";\n"
     << "  SHUnit *shUnit = shr->units[unit_index];\n";

  if (emitNativeTraces) {
    // Initialize the current SHUnit.
    OS << "  locals.head.unit = shUnit;\n";
    // Initialize the current source location to invalid.
    OS << "  locals.head.src_location_idx = "
       << SHSrcLocationTable::kInvalidLocIdx << ";\n";
  }

  // Initialize all registers to undefined.
  for (size_t i = 0; i < localsSize; ++i) {
    OS << "  locals.t" << i << " = _sh_ljs_undefined();\n";
  }
  for (size_t i = 0, e = RA.getMaxRegisterUsage(sh::RegClass::LocalNonPtr);
       i < e;
       ++i) {
    OS << "  SHLegacyValue np" << i << " = _sh_ljs_undefined();\n";
  }

  // Initialize SHJmpBufs for the maximum possible try nesting depth.
  for (size_t i = 0; i < maxTryDepth; ++i)
    OS << "  SHJmpBuf jmpBuf" << i << ";\n";

  if (emitNativeTraces) {
    // Setup the frame to reference this SHLocals.
    OS << "  frame[" << hbc::StackFrameLayout::SHLocals << "]"
       << " = _sh_ljs_native_pointer(&locals.head);\n";
  }

  // The most recent index into the source location table that was used by a
  // throwing instruction.
  SHSrcLocationTable::IdxTy prevSrcLocationIdx =
      SHSrcLocationTable::kInvalidLocIdx;
  for (auto &B : order) {
    // Line directives of the BB label will match those of the first
    // instruction in this basic block.
    OS.setDirectiveInfo(B->front().getLocation(), srcMgr);
    OS << "\n";
    generateBasicBlockLabel(B, OS, bbMap);
    OS << ":\n"
       // Print a no-op expression, since the line after a label must be an
       // expression (as opposed to a declaration).
       << "  ;\n";

    // The most recent location of an instruction that was printed as line
    // comments in the C file.
    SMLoc prevCommentLoc{};
    // The first instruction in the function should always have its line info
    // printed, since it has no previous instruction that it could be a
    // duplicate of.
    bool firstInst = true;
    for (auto &I : *B) {
      OS.setDirectiveInfo(I.getLocation(), srcMgr);
      if (emitSrcLocComments) {
        SMLoc curLoc = I.getLocation();
        // We only print out a line comment if the source location has changed
        // from the previous line comment printed.
        if (curLoc != prevCommentLoc || firstInst) {
          OS << "  // ";
          if (curLoc.isValid()) {
            srcMgr.dumpCoords(OS, curLoc);
          } else {
            // If we don't find any source location info, explicitly state that.
            OS << "no-src-info";
          }
          OS << '\n';
        }
        prevCommentLoc = curLoc;
        firstInst = false;
      }
      if (emitNativeTraces) {
        // If the instruction we are about to generate can throw, then we must
        // update the current source line we are executing in preparation.
        auto sideEffect = I.getSideEffect();
        if (sideEffect.getThrow()) {
          // If the instruction has no source location information, then just
          // use the invalid location.
          SHSrcLocationTable::IdxTy idx = SHSrcLocationTable::kInvalidLocIdx;
          SMLoc curLoc = I.getLocation();
          if (curLoc.isValid()) {
            hermes::SourceErrorManager::SourceCoords coords;
            if (srcMgr.findBufferLineAndLoc(curLoc, coords)) {
              idx = moduleGen.srcLocationTable.getIndex(srcMgr, coords);
            }
          }
          if (idx != prevSrcLocationIdx) {
            // We update the source location if the source location index of the
            // current instruction has changed from the previous update.
            OS << "  locals.head.src_location_idx = " << idx << ";\n";
          }
          prevSrcLocationIdx = idx;
        }
      }
      instrGen.generate(I);
    }
  }
  OS.disableLineDirectives();
  OS << "}\n";
}

/// Collect all the native externs used in the module.
std::unique_ptr<llvh::DenseSet<NativeExtern *>> collectUsedExterns(Module *M) {
  auto externs = llvh::make_unique<llvh::DenseSet<NativeExtern *>>();
  // Iterate all functions, basic blocks and operands, checking for
  // LiteralNativeExtern.
  for (auto &F : *M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        for (unsigned i = 0, count = I.getNumOperands(); i != count; ++i) {
          if (auto *ne = llvh::dyn_cast<LiteralNativeExtern>(I.getOperand(i))) {
            externs->insert(ne->getData());
          }
        }
      }
    }
  }
  return externs;
}

/// Generate the include statements requested by native externs.
void generateExternCIncludes(
    Module *M,
    llvh::raw_ostream &OS,
    const llvh::DenseSet<NativeExtern *> &usedExterns) {
  llvh::SmallSetVector<UniqueString *, 4> includes;
  for (NativeExtern *ne : M->getContext().getNativeContext().getAllExterns()) {
    if (usedExterns.count(ne))
      if (auto *inc = ne->include())
        includes.insert(inc);
  }
  for (auto *inc : includes) {
    OS << "#include <";
    OS.write_escaped(inc->c_str());
    OS << ">\n";
  }
  if (!includes.empty())
    OS << "\n";
}

/// Generate the external C declarations for native externs.
void generateExternC(
    Module *M,
    llvh::raw_ostream &OS,
    const llvh::DenseSet<NativeExtern *> &usedExterns) {
  size_t count = 0;
  for (NativeExtern *ne : M->getContext().getNativeContext().getAllExterns()) {
    if (usedExterns.count(ne) && !ne->declared() && !ne->include()) {
      ++count;
      if (count == 1)
        OS << '\n';
      ne->signature()->format(OS, ne->name()->c_str());
      OS << ";\n";
    }
  }
  if (count != 0)
    OS << '\n';
}

/// Converts Module \p M into valid C code and outputs it through \p OS.
/// Returns the cache size necessary to store all the cache indexes used.
void generateModule(
    Module *M,
    hermes::sh::LineDirectiveEmitter &OS,
    const BytecodeGenerationOptions &options) {
  if (!lowerModuleIR(M, options.optimizationEnabled)) {
    return;
  }

  if (options.verifyIR) {
    if (!verifyModule(*M, &llvh::errs(), VerificationMode::IR_LOWERED)) {
      M->getContext().getSourceErrorManager().error(
          SMLoc{}, "Lowered IR verification failed");
      return;
    }
  }

  if (options.format == DumpLIR) {
    M->dump();
    return;
  }

  // TODO: Share cache indices where the property name is the same and
  // -reuse-prop-cache is passed in.
  uint32_t nextCacheIdx = 0;
  ModuleGen moduleGen{M, options.optimizationEnabled};

  if (options.format == DumpBytecode || options.format == EmitBundle) {
    if (!isValidSHUnitName(options.unitName))
      hermes_fatal("Invalid unit name passed to SH backend.");
    OS << R"(
#include "hermes/VM/static_h.h"

#include <stdlib.h>

)";

    auto usedExterns = collectUsedExterns(M);
    generateExternCIncludes(M, OS, *usedExterns);

    OS << R"(
static uint32_t unit_index;
static inline SHSymbolID* get_symbols(SHUnit *);
static inline SHPropertyCacheEntry* get_prop_cache(SHUnit *);
static const SHSrcLoc s_source_locations[];
static SHNativeFuncInfo s_function_info_table[];
)";

    // Declare extern functions.
    generateExternC(M, OS, *usedExterns);
    // Free the used externs, we no longer need them.
    usedExterns.reset();

    // Forward declare every JS function.
    for (auto &F : *M) {
      OS << "static SHLegacyValue ";
      moduleGen.nativeFunctionTable.generateFunctionLabel(&F, OS);
      OS << "(SHRuntime *shr);\n";
    }
  }

  M->assignIndexToVariables();

  for (auto &F : *M)
    generateFunction(F, OS, moduleGen, nextCacheIdx, options);

  if (options.format == DumpBytecode || options.format == EmitBundle) {
    moduleGen.literalBuffers.generate(OS);
    moduleGen.srcLocationTable.generate(
        OS, M->getContext().getSourceErrorManager());
    moduleGen.nativeFunctionTable.generate(OS);
    // String table should be generated last, because the generate calls to
    // other module components may add new entries to the string table.
    moduleGen.stringTable.generate(OS);

    // Note that we prefix the unit name with sh_export_ to avoid potential
    // conflicts.
    OS << "#define CREATE_THIS_UNIT sh_export_" << options.unitName << "\n";

    OS << "struct UnitData {\n"
       << "  SHUnit unit;\n"
       << "  SHSymbolID symbol_data[" << moduleGen.stringTable.size() << "];\n"
       << "  SHPropertyCacheEntry prop_cache_data[" << nextCacheIdx << "];\n;"
       << "  SHCompressedPointer object_literal_class_cache["
       << moduleGen.literalBuffers.objShapeTable.size() << "];\n};\n"
       << "SHUnit *CREATE_THIS_UNIT(SHRuntime *shr) {\n"
       << "  struct UnitData *unit_data = calloc(sizeof(struct UnitData), 1);\n"
       << "  *unit_data = (struct UnitData){.unit = {.index = &unit_index,"
       << ".num_symbols =" << moduleGen.stringTable.size()
       << ", .num_prop_cache_entries = " << nextCacheIdx
       << ", .ascii_pool = s_ascii_pool, .u16_pool = s_u16_pool,"
       << ".strings = s_strings, .symbols = unit_data->symbol_data,"
       << ".prop_cache = unit_data->prop_cache_data,"
       << ".obj_key_buffer = s_obj_key_buffer, .obj_key_buffer_size = "
       << moduleGen.literalBuffers.objKeyBuffer.size() << ", "
       << ".literal_val_buffer = s_literal_val_buffer, .literal_val_buffer_size = "
       << moduleGen.literalBuffers.literalValueBuffer.size() << ", "
       << ".obj_shape_table = s_obj_shape_table, "
       << ".obj_shape_table_count = "
       << moduleGen.literalBuffers.objShapeTable.size() << ", "
       << ".object_literal_class_cache = unit_data->object_literal_class_cache, "
       << ".source_locations = s_source_locations, "
       << ".source_locations_size = " << moduleGen.srcLocationTable.size()
       << ", " << ".unit_main = _0_global, "
       << ".unit_main_info = &s_function_info_table[0], "
       << ".unit_name = \"sh_compiled\" }};\n"
       << "  return (SHUnit *)unit_data;\n}\n"
       << R"(
SHSymbolID *get_symbols(SHUnit *unit) {
  return ((struct UnitData *)unit)->symbol_data;
}

SHPropertyCacheEntry *get_prop_cache(SHUnit *unit) {
  return ((struct UnitData *)unit)->prop_cache_data;
}
)";
    if (options.emitMain) {
      OS << R"(
void init_console_bindings(SHRuntime *shr);

int main(int argc, char **argv) {
  SHRuntime *shr = _sh_init(argc, argv);
  init_console_bindings(shr);
  bool success = _sh_initialize_units(shr, 1, CREATE_THIS_UNIT);
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
  LineDirectiveEmitter emitter{OS};
  generateModule(M, emitter, options);
}
