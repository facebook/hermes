/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/HBC.h"

#include "LoweringPipelines.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/Passes/InsertProfilePoint.h"
#include "hermes/BCGen/HBC/Passes/OptParentEnvironment.h"
#include "hermes/BCGen/HBC/Passes/PeepholeLowering.h"
#include "hermes/BCGen/HBC/TraverseLiteralStrings.h"
#include "hermes/BCGen/LowerBuiltinCalls.h"
#include "hermes/BCGen/LowerScopes.h"
#include "hermes/BCGen/LowerStoreInstrs.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/UTF8.h"

#include "llvh/Support/raw_ostream.h"

#define DEBUG_TYPE "hbc-backend"

namespace hermes {
namespace hbc {

namespace {

/// Used in delta optimizing mode.
/// \return a UniquingStringLiteralAccumulator seeded with strings  from a
/// bytecode provider \p bcProvider.
UniquingStringLiteralAccumulator stringAccumulatorFromBCProvider(
    const BCProviderBase &bcProvider) {
  uint32_t count = bcProvider.getStringCount();

  std::vector<StringTableEntry> entries;
  std::vector<bool> isIdentifier;

  entries.reserve(count);
  isIdentifier.reserve(count);

  {
    unsigned i = 0;
    for (auto kindEntry : bcProvider.getStringKinds()) {
      bool isIdentRun = kindEntry.kind() != StringKind::String;
      for (unsigned j = 0; j < kindEntry.count(); ++j, ++i) {
        entries.push_back(bcProvider.getStringTableEntry(i));
        isIdentifier.push_back(isIdentRun);
      }
    }

    assert(i == count && "Did not initialise every string");
  }

  auto strStorage = bcProvider.getStringStorage();
  ConsecutiveStringStorage css{std::move(entries), strStorage.vec()};

  return UniquingStringLiteralAccumulator{
      std::move(css), std::move(isIdentifier)};
}
}; // namespace

/// Encode a Unicode codepoint into a UTF8 sequence and append it to \p
/// storage. Code points above 0xFFFF are encoded into UTF16, and the
/// resulting surrogate pair values are encoded individually into UTF8.
static inline void appendUnicodeToStorage(
    uint32_t cp,
    llvh::SmallVectorImpl<char> &storage) {
  // Sized to allow for two 16-bit values to be encoded.
  // A 16-bit value takes up to three bytes encoded in UTF-8.
  char buf[8];
  char *d = buf;
  // We need to normalize code points which would be encoded with a surrogate
  // pair. Note that this produces technically invalid UTF-8.
  if (LLVM_LIKELY(cp < 0x10000)) {
    hermes::encodeUTF8(d, cp);
  } else {
    assert(cp <= UNICODE_MAX_VALUE && "invalid Unicode value");
    cp -= 0x10000;
    hermes::encodeUTF8(d, UTF16_HIGH_SURROGATE + ((cp >> 10) & 0x3FF));
    hermes::encodeUTF8(d, UTF16_LOW_SURROGATE + (cp & 0x3FF));
  }
  storage.append(buf, d);
}

std::unique_ptr<BytecodeModule> generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  PerfSection perf("Bytecode Generation");
  lowerModuleIR(M, options);

  if (options.format == DumpLIR)
    M->dump();

  BytecodeModuleGenerator BMGen(options, sourceMapGen);

  if (segment) {
    BMGen.setSegmentID(*segment);
  }
  // Empty if all functions should be generated (i.e. bundle splitting was not
  // requested).
  llvh::DenseSet<Function *> functionsToGenerate = segment
      ? M->getFunctionsInSegment(*segment)
      : llvh::DenseSet<Function *>{};

  /// \return true if we should generate function \p f.
  std::function<bool(const Function *)> shouldGenerate;
  if (segment) {
    shouldGenerate = [entryPoint, &functionsToGenerate](const Function *f) {
      return f == entryPoint || functionsToGenerate.count(f) > 0;
    };
  } else {
    shouldGenerate = [](const Function *) { return true; };
  }

  /// Mapping of the source text UTF-8 to the modified UTF-16-like
  /// representation used by string literal encoding.
  /// See appendUnicodeToStorage.
  /// If a function source isn't in this map, then it's entirely ASCII and can
  /// be added to the string table unmodified.
  /// This allows us to add strings to the StringLiteralTable,
  /// which will convert actual UTF-8 to UTF-16 automatically if it's detected,
  /// meaning we'd not be able to directly look up the original function source
  /// in the table.
  llvh::DenseMap<llvh::StringRef, llvh::SmallVector<char, 32>>
      unicodeFunctionSources{};

  { // Collect all the strings in the bytecode module into a storage.
    // If we are in delta optimizing mode, start with the string storage from
    // our base bytecode provider.
    auto strings = baseBCProvider
        ? stringAccumulatorFromBCProvider(*baseBCProvider)
        : UniquingStringLiteralAccumulator{};

    auto addStringOrIdent = [&strings](llvh::StringRef str, bool isIdentifier) {
      strings.addString(str, isIdentifier);
    };

    auto addString = [&strings](llvh::StringRef str) {
      strings.addString(str, /* isIdentifier */ false);
    };

    traverseLiteralStrings(M, shouldGenerate, addStringOrIdent);

    if (options.stripFunctionNames) {
      addString(kStrippedFunctionName);
    }

    /// Add the original function source \p str to the \c strings table.
    /// If it's not ASCII, re-encode it using the string table's string literal
    /// encoding and map from the original source to the newly encoded source in
    /// unicodeFunctionSources,so it can be reused below.
    auto addFunctionSource = [&strings,
                              &unicodeFunctionSources](llvh::StringRef str) {
      if (hermes::isAllASCII(str.begin(), str.end())) {
        // Fast path, no re-encoding needed.
        strings.addString(str, /* isIdentifier */ false);
      } else {
        auto &storage = unicodeFunctionSources[str];
        if (!storage.empty())
          return;
        for (const char *cur = str.begin(), *e = str.end(); cur != e;
             /* increment in body */) {
          if (LLVM_UNLIKELY(isUTF8Start(*cur))) {
            // Decode and re-encode the character and append it to the string
            // storage
            appendUnicodeToStorage(
                hermes::_decodeUTF8SlowPath<false>(
                    cur, [](const llvh::Twine &) {}),
                storage);
          } else {
            storage.push_back(*cur);
            ++cur;
          }
        }
        strings.addString(
            llvh::StringRef{storage.begin(), storage.size()},
            /* isIdentifier */ false);
      }
    };

    // Populate strings table and if the source of a function contains unicode,
    // add an entry to the unicodeFunctionSources.
    traverseFunctions(
        M,
        shouldGenerate,
        addString,
        addFunctionSource,
        options.stripFunctionNames);

    if (!M->getCJSModulesResolved()) {
      traverseCJSModuleNames(M, shouldGenerate, addString);
    }

    BMGen.initializeStringTable(UniquingStringLiteralAccumulator::toTable(
        std::move(strings), options.optimizationEnabled));
  }

  // Generate the serialized literal buffers.
  {
    BMGen.initializeSerializedLiterals(LiteralBufferBuilder::generate(
        M,
        shouldGenerate,
        [&BMGen](llvh::StringRef str) { return BMGen.getIdentifierID(str); },
        [&BMGen](llvh::StringRef str) { return BMGen.getStringID(str); },
        options.optimizationEnabled));
  }

  // Add each function to BMGen so that each function has a unique ID.
  for (auto &F : *M) {
    if (!shouldGenerate(&F)) {
      continue;
    }

    unsigned index = BMGen.addFunction(&F);
    if (&F == entryPoint) {
      BMGen.setEntryPointIndex(index);
    }

    auto *cjsModule = M->findCJSModule(&F);
    if (cjsModule) {
      if (M->getCJSModulesResolved()) {
        BMGen.addCJSModuleStatic(cjsModule->id, index);
      } else {
        BMGen.addCJSModule(index, BMGen.getStringID(cjsModule->filename.str()));
      }
    }

    // Add entries to function source table for non-default source.
    if (!F.isGlobalScope()) {
      if (auto source = F.getSourceRepresentationStr()) {
        auto it = unicodeFunctionSources.find(*source);
        // If the original source was mapped to a re-encoded one in
        // unicodeFunctionSources, then use the re-encoded source to lookup the
        // string ID. Otherwise it's ASCII and can be used directly.
        if (it != unicodeFunctionSources.end()) {
          BMGen.addFunctionSource(
              index,
              BMGen.getStringID(
                  llvh::StringRef{it->second.begin(), it->second.size()}));
        } else {
          BMGen.addFunctionSource(index, BMGen.getStringID(*source));
        }
      }
    }
  }
  assert(BMGen.getEntryPointIndex() != -1 && "Entry point not added");
  return std::move(BMGen).generate();
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
