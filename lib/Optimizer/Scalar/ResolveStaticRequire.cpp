/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "resolverequire"
#include "hermes/Optimizer/Scalar/ResolveStaticRequire.h"

#include "hermes/IR/IREval.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/Debug.h"
#include "llvh/Support/Path.h"

STATISTIC(NumRequireCalls, "Number of require() calls found");
STATISTIC(NumRequireCallsResolved, "Number of require() calls resolved");

namespace hermes {

namespace {

class ResolveStaticRequireImpl {
  /// The Module object we are processing. (NOTE: this is not a CJS module)
  Module *const M_;

  /// Convenient shortcut to the SourceErrorManager.
  SourceErrorManager &EM_;

  /// Builder used for new instructions.
  IRBuilder builder_;

  /// Set to false if there is at least one case that we couldn't resolve
  /// statically.
  bool canResolve_{true};

  /// An instance of using the "require" parameter or a value derived
  /// from it.
  struct Usage {
    /// The value that is being used. It can be one of:
    /// - Parameter (the "require" parameter)
    /// - AllocStackInst
    /// - LoadStackInst
    /// - Variable
    /// - LoadFrameVariable
    Value *V;

    /// The instruction using the value.
    Instruction *I;

    Usage(Value *V, Instruction *I) : V(V), I(I) {}
  };

  /// A resolved require() call.
  struct ResolvedRequire {
    /// The call instruction we resolved and will replace.
    CallInst *call;
    /// The resolved target.
    Literal *resolvedTarget;

    ResolvedRequire(CallInst *call, Literal *resolvedTarget)
        : call(call), resolvedTarget(resolvedTarget) {}
  };

  /// All resolved require calls. If canResolve_ is true, we can replace them
  /// with HermesInternal.require() calls.
  std::vector<ResolvedRequire> resolvedRequireCalls_{};

 public:
  ResolveStaticRequireImpl(Module *M)
      : M_(M), EM_(M->getContext().getSourceErrorManager()), builder_{M} {}

  bool run();

 private:
  void resolveCJSModule(Function *moduleFunction);
  void resolveRequireCall(Function *moduleFunction, CallInst *call);

  /// Attempt to resolve the specified require() target. On failure report a
  /// warning at the specified location and return nullptr.
  /// \param moduleFunction the CJS module in which context to perform the
  ///   resolution.
  /// \param target the resolve target, which was the first argument of
  ///   require().
  /// \param errorLoc report the warning at that location if resolution fails.
  /// \return a literal representing the resolved module or nullptr on error.
  Literal *resolveModuleTarget(
      Function *moduleFunction,
      LiteralString *target,
      SMLoc errorLoc);
};

bool ResolveStaticRequireImpl::run() {
  for (const auto &module : M_->getCJSModules()) {
    resolveCJSModule(module.function);
  }

  if (!canResolve_)
    return false;

  // Replace all resolved calls with calls to HermesInternal.requireFast().
  for (const auto &RR : resolvedRequireCalls_) {
    builder_.setInsertionPointAfter(RR.call);

    builder_.setLocation(RR.call->getLocation());
    builder_.setCurrentSourceLevelScope(RR.call->getSourceLevelScope());

    /// (CallBuiltin "requireFast", resolvedTarget)
    auto callHI = builder_.createCallBuiltinInst(
        BuiltinMethod::HermesBuiltin_requireFast, RR.resolvedTarget);

    RR.call->replaceAllUsesWith(callHI);
    RR.call->eraseFromParent();
  }

  M_->setCJSModulesResolved(true);
  return !resolvedRequireCalls_.empty();
}

void ResolveStaticRequireImpl::resolveCJSModule(Function *moduleFunction) {
  assert(
      moduleFunction->getParameters().size() == 3 &&
      "CJS module functions must have three parameters");

  Parameter *requireParam = moduleFunction->getParameters()[1];
  assert(
      requireParam->getName().str() == "require" &&
      "CJS module second parameter must be 'require'");

  // Visited instructions.
  llvh::SmallDenseSet<Instruction *> visited{};

  // Usages to process.
  llvh::SmallVector<Usage, 8> workList{};

  // Add all unvisited users of a value to the work list.
  auto addUsers = [&visited, &workList](Value *value) {
    for (Instruction *I : value->getUsers())
      if (visited.insert(I).second)
        workList.emplace_back(value, I);
  };

  // Add all usages of the "require" parameter to the worklist, which then may
  // add more usages and so on.
  addUsers(requireParam);

  while (!workList.empty()) {
    Usage U = workList.pop_back_val();

    if (auto *call = llvh::dyn_cast<CallInst>(U.I)) {
      // Make sure require() doesn't escape as a parameter.
      bool fail = false;
      for (unsigned numArgs = call->getNumArguments(), arg = 0; arg != numArgs;
           ++arg) {
        if (call->getArgument(arg) == U.V) {
          // Oops, "require" is passed as parameter.
          EM_.warning(
              Warning::UnresolvedStaticRequire,
              call->getLocation(),
              "'require' used as function call argument");
          // No need to analyze this usage anymore.
          fail = true;
          canResolve_ = false;
          break;
        }
      }

      if (fail)
        continue;

      if (llvh::isa<ConstructInst>(call) || llvh::isa<HBCConstructInst>(call)) {
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            call->getLocation(),
            "'require' used as a constructor");
        canResolve_ = false;
        continue;
      }

      assert(
          call->getCallee() == U.V && "Value is not used at all in CallInst");

      resolveRequireCall(moduleFunction, call);
    } else if (auto *SS = llvh::dyn_cast<StoreStackInst>(U.I)) {
      // Storing "require" into a stack location.

      if (!isStoreOnceStackLocation(cast<AllocStackInst>(SS->getPtr()))) {
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            SS->getLocation(),
            "'require' stored into local cannot be resolved");
        canResolve_ = false;
        continue;
      }

      addUsers(SS->getPtr());
    } else if (auto *LS = llvh::dyn_cast<LoadStackInst>(U.I)) {
      // Loading "require" from a stack location.

      addUsers(LS);
    } else if (auto *SF = llvh::dyn_cast<StoreFrameInst>(U.I)) {
      // Storing "require" into a frame variable.

      if (!isStoreOnceVariable(SF->getVariable())) {
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            SF->getLocation(),
            "'require' is copied to a variable which cannot be analyzed");
        canResolve_ = false;
        continue;
      }

      addUsers(SF->getVariable());
    } else if (auto *LF = llvh::dyn_cast<LoadFrameInst>(U.I)) {
      // Loading "require" from a frame variable.

      addUsers(LF);
    } else if (auto *LPI = llvh::dyn_cast<LoadPropertyInst>(U.I)) {
      if (LPI->getProperty() == U.V) {
        // `require` must not be used as a key in a LoadPropertyInst,
        // because it could be used in a getter and escape.
        canResolve_ = false;
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            U.I->getLocation(),
            "'require' is used as a property key and cannot be analyzed");
      }
      // Loading values from require is allowed.
      // In particular, require.context is used to load new segments.
    } else {
      canResolve_ = false;
      EM_.warning(
          Warning::UnresolvedStaticRequire,
          U.I->getLocation(),
          "'require' escapes or is modified");
    }
  }
}

void ResolveStaticRequireImpl::resolveRequireCall(
    Function *moduleFunction,
    CallInst *call) {
  ++NumRequireCalls;

  if (call->getNumArguments() < 2) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require() invoked without arguments");
    canResolve_ = false;
    return;
  }

  if (call->getNumArguments() > 2) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "Additional require() arguments will be ignored");
  }

  LiteralString *stringTarget = nullptr;
  if (auto *lit = llvh::dyn_cast<Literal>(call->getArgument(1)))
    stringTarget = evalToString(builder_, lit);

  if (!stringTarget) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require() argument cannot be coerced to constant string at compile time");
    if (auto *inst = llvh::dyn_cast<Instruction>(call->getArgument(1)))
      if (inst->hasLocation())
        EM_.note(inst->getLocation(), "First argument of require()");

    canResolve_ = false;
    return;
  }

  auto *resolved =
      resolveModuleTarget(moduleFunction, stringTarget, call->getLocation());
  if (!resolved) {
    canResolve_ = false;
    return;
  }

  resolvedRequireCalls_.emplace_back(call, resolved);
  ++NumRequireCallsResolved;
}

/// Canonicalize \p dirname and \p target together, placing the result in
/// \p dirname.
/// This assumes that the target is given relative to the current dirname.
static void canonicalizePath(
    llvh::SmallVectorImpl<char> &dirname,
    llvh::StringRef target) {
  if (!target.empty() && target[0] == '/') {
    // If the target is absolute (starts with a '/'), resolve from the module
    // root (disregard the dirname).
    dirname.clear();
    llvh::sys::path::append(dirname, target.drop_front(1));
    return;
  }
  llvh::sys::path::append(dirname, llvh::sys::path::Style::posix, target);

  // Remove all dots. This is done to get rid of ../ or anything like ././.
  llvh::sys::path::remove_dots(dirname, true, llvh::sys::path::Style::posix);
}

Literal *ResolveStaticRequireImpl::resolveModuleTarget(
    Function *moduleFunction,
    LiteralString *target,
    SMLoc errorLoc) {
  IRBuilder builder{moduleFunction->getParent()};

  Module *module = moduleFunction->getParent();
  auto *cjsModule = module->findCJSModule(moduleFunction);
  assert(cjsModule && "Cannot run require() from a non-module function");

  Identifier targetIdentifier{};
  assert(
      !targetIdentifier.isValid() &&
      "Uninitialized identifier must be invalid");

  // Attempt to resolve the require directly using the resolution table.
  const auto *resolutionTable =
      moduleFunction->getContext().getResolutionTable();
  if (resolutionTable) {
    LLVM_DEBUG(
        llvh::errs() << "Resolving " << cjsModule->filename.str() << " @ "
                     << target->getValue().str() << '\n');
    auto fileEntry = resolutionTable->find(cjsModule->filename.str());
    if (fileEntry != resolutionTable->end()) {
      auto targetEntry = fileEntry->second.find(target->getValue().str());
      if (targetEntry != fileEntry->second.end()) {
        targetIdentifier = builder.createIdentifier(targetEntry->second);
      }
    }
  }

  // If we failed to resolve using the resolution table, fall back to the
  // basic canonicalization procedure.
  if (!targetIdentifier.isValid()) {
    // First, place the directory name into canonicalPath.
    llvh::SmallString<32> canonicalPath = cjsModule->filename.str();
    llvh::sys::path::remove_filename(
        canonicalPath, llvh::sys::path::Style::posix);

    // Canonicalize (canonicalPath + target) together to get the final path.
    canonicalizePath(canonicalPath, target->getValue().str());
    targetIdentifier = builder.createIdentifier(canonicalPath);
  }

  assert(
      targetIdentifier.isValid() && "Failed to construct valid target string");

  auto targetModuleID = module->findCJSModuleID(targetIdentifier);
  if (!targetModuleID) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        errorLoc,
        "Cannot resolve target module of require");
    return nullptr;
  }
  return builder.getLiteralNumber(*targetModuleID);
}

} // anonymous namespace

bool ResolveStaticRequire::runOnModule(Module *M) {
  if (!M->getContext().getUseCJSModules() ||
      !M->getContext().getOptimizationSettings().staticRequire) {
    return false;
  }

  SourceErrorManager::SaveAndBufferMessages bm{
      &M->getContext().getSourceErrorManager()};
  ResolveStaticRequireImpl impl{M};
  return impl.run();
}

Pass *createResolveStaticRequire() {
  return new ResolveStaticRequire();
}

} // namespace hermes

#undef DEBUG_TYPE
