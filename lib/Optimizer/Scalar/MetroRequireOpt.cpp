/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// This optimization identifies calls whose callable is known to be the
/// Metro require function.  It relies on a Babel transform to introduce
/// $SHBuiltin.moduleFactory calls, which identify the module factory
/// functions.  It then assumes that a particular argument to such module
/// factory functions is the require function. It tracks use/def chains of
/// this argument, stopping at calls.  These get a new "metro-require"
/// attribute.  These may get treated specially in other optimization passes,
/// or in codegen.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "metrorequire"

#include "hermes/IR/IREval.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
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

class MetroRequireImpl {
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
  MetroRequireImpl(Module *M)
      : M_(M), EM_(M->getContext().getSourceErrorManager()), builder_{M} {}

  bool run();

 private:
  void resolveMetroModule(Function *moduleFunction);
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

bool MetroRequireImpl::run() {
  for (const auto &jsModuleFactoryFunc : M_->jsModuleFactoryFunctions()) {
    resolveMetroModule(jsModuleFactoryFunc);
  }

  if (!canResolve_)
    return false;

  // Replace all resolved calls with calls to HermesInternal.requireFast().
  for (const auto &RR : resolvedRequireCalls_) {
    builder_.setInsertionPointAfter(RR.call);

    builder_.setLocation(RR.call->getLocation());

    /// (CallBuiltin "requireFast", resolvedTarget)
    auto callHI = builder_.createCallBuiltinInst(
        BuiltinMethod::HermesBuiltin_requireFast, RR.resolvedTarget);

    RR.call->replaceAllUsesWith(callHI);
    RR.call->eraseFromParent();
  }

  return !resolvedRequireCalls_.empty();
}

static constexpr unsigned kRequireIndex = 2;

void MetroRequireImpl::resolveMetroModule(Function *moduleFactoryFunction) {
  // Module factory functions must have enough arguments (including 'this')
  // to make kRequireIndex a valid argument index.
  assert(
      moduleFactoryFunction->getJSDynamicParams().size() > kRequireIndex &&
      "Metro module functions missing parameters");

  // A phi is the 'require' function iff all it's input values are.
  // But we will discover this only over time, as the different arguments
  // to the phi are considered.  This maps phi instructions that have at
  // least one argument known to be 'require' to the set of their entry
  // indices known to be require.  When all entries are confirmed to be
  // require we treat the phi as a 'require.'
  llvh::DenseMap<PhiInst *, llvh::SmallBitVector> phis;

  // If a 'require' value is stored to the stack, we can consider that stack
  // location to be 'require' if all stores store 'require'.  This maps
  // stack locations (results of AllocStackInst) to the set of
  // StoreStackLocations known to store 'require'.  We check at the end
  // whether all stores were 'require'; if so, we consider loads from that
  // stack location to be require.
  llvh::DenseMap<AllocStackInst *, llvh::DenseSet<StoreStackInst *>>
      requireStoreStacks;

  // Visited instructions.
  llvh::SmallDenseSet<Instruction *> visited{};

  // Usages to process.
  llvh::SmallVector<Usage, 8> workList{};

  // Add all unvisited users of a value to the work list.
  auto addUsers = [&visited, &workList, &phis](Value *value) {
    for (Instruction *I : value->getUsers()) {
      // We handle phi instructions specially: if a user is a phi,
      // we keep track of how many of the phi's inputs have been found
      // to be require values.  When they all are, we add the phi
      // to the workList (and the main loop will add the phi's users).
      if (auto *phi = llvh::dyn_cast<PhiInst>(I)) {
        unsigned i = 0;
        for (; i < phi->getNumEntries(); i++) {
          if (phi->getEntry(i).first == value) {
            break;
          }
        }
        assert(i < phi->getNumEntries() && "should have found the index.");
        // Ensure that there's an entry for phi with the right number of
        // bits.
        auto [it, inserted] =
            phis.try_emplace(phi, phi->getNumEntries(), false);
        llvh::SmallBitVector &requireEntries = it->second;
        if (!inserted) {
          requireEntries.resize(phi->getNumEntries());
        }
        requireEntries.set(i);
        if (requireEntries.find_first_unset() >= 0) {
          // We've haven't (yet) found all the phi arguments
          // to be requires.  Only add it to workList (below) when
          // we have.
          continue;
        }
      }
      if (visited.insert(I).second) {
        workList.emplace_back(value, I);
      }
    }
  };

  // Add all usages of the "require" parameter to the worklist, which then may
  // add more usages and so on.
  for (auto *I :
       moduleFactoryFunction->getJSDynamicParam(kRequireIndex)->getUsers()) {
    assert(
        llvh::isa<LoadParamInst>(I) &&
        "Use of JSDynamicParam must be LoadParamInst");
    addUsers(I);
  }

  while (!workList.empty()) {
    Usage U = workList.pop_back_val();

    if (auto *call = llvh::dyn_cast<BaseCallInst>(U.I)) {
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

      if (!llvh::isa<CallInst>(call)) {
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            call->getLocation(),
            "'require' used in unexpected way");
        canResolve_ = false;
        continue;
      }

      if (!llvh::isa<LiteralUndefined>(call->getNewTarget())) {
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            call->getLocation(),
            "'require' used as a constructor");
        canResolve_ = false;
        continue;
      }

      assert(
          call->getCallee() == U.V && "Value is not used at all in CallInst");

      resolveRequireCall(moduleFactoryFunction, llvh::cast<CallInst>(call));
    } else if (auto *SS = llvh::dyn_cast<StoreStackInst>(U.I)) {
      // Storing "require" into a stack location.  Record that this location
      // has a store of 'require'; later we'll see if all stores store
      // 'require'.
      requireStoreStacks[SS->getPtr()].insert(SS);

    } else if (auto *LS = llvh::dyn_cast<LoadStackInst>(U.I)) {
      // Loading "require" from a stack location.

      addUsers(LS);
    } else if (auto *SF = llvh::dyn_cast<StoreFrameInst>(U.I)) {
      // Storing "require" into a frame variable.

      if (!isStoreOnceVariable(SF->getVariable())) {
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            SF->getLocation(),
            "In function '" +
                SF->getFunction()->getOriginalOrInferredName().str() +
                "', 'require' is copied to a variable which " +
                "cannot be analyzed");
        canResolve_ = false;
        continue;
      }
      addUsers(SF->getVariable());
    } else if (auto *LF = llvh::dyn_cast<LoadFrameInst>(U.I)) {
      // Loading "require" from a frame variable.

      addUsers(LF);
    } else if (auto *LPI = llvh::dyn_cast<BaseLoadPropertyInst>(U.I)) {
      if (LPI->getProperty() == U.V) {
        // `require` must not be used as a key in a LoadPropertyInst,
        // because it could be used in a getter and escape.
        canResolve_ = false;
        EM_.warning(
            Warning::UnresolvedStaticRequire,
            U.I->getLocation(),
            "In function '" +
                SF->getFunction()->getOriginalOrInferredName().str() +
                "', 'require' is used as a property key and " +
                "cannot be analyzed");
      }
      // Loading values from require is allowed.
      // In particular, require.context is used to load new segments.

    } else if (auto *phi = llvh::dyn_cast<PhiInst>(U.I)) {
      // If a phi has been placed on the worklist, it's known that
      // all its inputs are require.  Add the phi's users.
      addUsers(phi);

    } else {
      canResolve_ = false;
      EM_.warning(
          Warning::UnresolvedStaticRequire,
          U.I->getLocation(),
          "In function '" +
              U.I->getFunction()->getOriginalOrInferredName().str() +
              "', 'require' escapes or is modified");
    }

    if (workList.empty()) {
      // See if we've determined that some stack locations contain only
      // 'require'.
      for (auto &[stackLoc, requireStores] : requireStoreStacks) {
        llvh::SmallVector<LoadStackInst *, 4> uses;
        unsigned numStores = 0;
        bool allLoadsOrStores = true;
        for (auto *stackLocUser : stackLoc->getUsers()) {
          if (auto *loadStack = llvh::dyn_cast<LoadStackInst>(stackLocUser)) {
            uses.push_back(loadStack);
          } else if (auto *def = llvh::dyn_cast<StoreStackInst>(stackLocUser)) {
            numStores++;
          } else {
            allLoadsOrStores = false;
            break;
          }
        }
        if (allLoadsOrStores && numStores == requireStores.size()) {
          // All uses are loads or stores, and the stores are all stores of
          // 'require'.  So all loads from this location yield 'require'.  Add
          // all uses of those loads to the worklist.
          for (auto *loadStack : uses) {
            addUsers(loadStack);
          }
        } else {
          // Emit error messages.
          for (auto *stackLocUser : stackLoc->getUsers()) {
            if (auto *def = llvh::dyn_cast<StoreStackInst>(stackLocUser)) {
              EM_.warning(
                  Warning::UnresolvedStaticRequire,
                  def->getLocation(),
                  "In function '" +
                      U.I->getFunction()->getOriginalOrInferredName().str() +
                      "', 'require' stored into local cannot be resolved");
            }
          }
        }
      }
    }
  }
}

void MetroRequireImpl::resolveRequireCall(
    Function *moduleFactoryFunction,
    CallInst *call) {
  ++NumRequireCalls;
  constexpr unsigned kRequireArgs = 2;
  if (call->getNumArguments() < kRequireArgs) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require() invoked with insufficient arguments");
    canResolve_ = false;
    return;
  }

  auto *litNum = llvh::dyn_cast<LiteralNumber>(call->getArgument(1));
  if (!litNum) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require argument not literal number");
    canResolve_ = false;
    return;
  }
  if (!litNum->isUInt32Representible()) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require argument must be non-negative integer");
    canResolve_ = false;
    return;
  }

  if (!call->getCalleeIsAlwaysClosure()) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require call whose callee is not always closure");
    canResolve_ = false;
    return;
  }

  if (!llvh::isa<EmptySentinel>(call->getEnvironment())) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require call with an 'env' argument");
    canResolve_ = false;
    return;
  }

  if (!call->getNewTarget()->getType().isUndefinedType()) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require call whose 'newTarget' is not undefined");
    canResolve_ = false;
    return;
  }
  if (!call->getThis()->getType().isUndefinedType()) {
    EM_.warning(
        Warning::UnresolvedStaticRequire,
        call->getLocation(),
        "require call whose 'this' is not undefined");
    canResolve_ = false;
    return;
  }

  call->getAttributesRef(moduleFactoryFunction->getParent()).isMetroRequire = 1;

  ++NumRequireCallsResolved;
}

} // namespace

Pass *createMetroRequire() {
  class ThisPass : public ModulePass {
   public:
    explicit ThisPass() : ModulePass("MetroRequire") {}
    ~ThisPass() override = default;

    bool runOnModule(Module *M) override {
      if (!M->getContext().getMetroRequireOpt()) {
        return false;
      }

      SourceErrorManager::SaveAndBufferMessages bm{
          &M->getContext().getSourceErrorManager()};
      MetroRequireImpl impl{M};
      return impl.run();
    }
  };
  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
