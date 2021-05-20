/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "vm"
#include "hermes/VM/Runtime.h"

#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/BCGen/HBC/SimpleBytecodeBuilder.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/InternalBytecode/InternalBytecode.h"
#include "hermes/Platform/Logging.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/FillerCell.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/MockedEnvironment.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/PredefinedStringIDs.h"
#include "hermes/VM/Profiler/CodeCoverageProfiler.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StackTracesTree.h"
#include "hermes/VM/StringView.h"

#ifndef HERMESVM_LEAN
#include "hermes/Support/MemoryBuffer.h"
#endif

#include "llvh/ADT/Hashing.h"
#include "llvh/Support/Debug.h"
#include "llvh/Support/raw_ostream.h"

#ifdef HERMESVM_PROFILER_BB
#include "hermes/VM/IterationKind.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Profiler/InlineCacheProfiler.h"
#include "llvh/ADT/DenseMap.h"
#endif

namespace hermes {
namespace vm {

namespace {

/// The maximum number of registers that can be requested in a RuntimeConfig.
static constexpr uint32_t kMaxSupportedNumRegisters =
    UINT32_MAX / sizeof(PinnedHermesValue);

// Only track I/O for buffers > 64 kB (which excludes things like
// Runtime::generateSpecialRuntimeBytecode).
static constexpr size_t MIN_IO_TRACKING_SIZE = 64 * 1024;

static const Predefined::Str fixedPropCacheNames[(size_t)PropCacheID::_COUNT] =
    {
#define V(id, predef) predef,
        PROP_CACHE_IDS(V)
#undef V
};

} // namespace

/* static */
std::shared_ptr<Runtime> Runtime::create(const RuntimeConfig &runtimeConfig) {
  return std::shared_ptr<Runtime>{
      new Runtime(StorageProvider::mmapProvider(), runtimeConfig)};
}

CallResult<PseudoHandle<>> Runtime::getNamed(
    Handle<JSObject> obj,
    PropCacheID id) {
  CompressedPointer clazzPtr{obj->getClassGCPtr()};
  auto *cacheEntry = &fixedPropCache_[static_cast<int>(id)];
  if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
    // The slot is cached, so it is safe to use the Internal function.
    return createPseudoHandle(
        JSObject::getNamedSlotValueUnsafe<PropStorage::Inline::Yes>(
            *obj, this, cacheEntry->slot)
            .unboxToHV(this));
  }
  auto sym = Predefined::getSymbolID(fixedPropCacheNames[static_cast<int>(id)]);
  NamedPropertyDescriptor desc;
  // Check writable and internalSetter flags since the cache slot is shared for
  // get/put.
  if (LLVM_LIKELY(
          JSObject::tryGetOwnNamedDescriptorFast(*obj, this, sym, desc)) &&
      !desc.flags.accessor && desc.flags.writable &&
      !desc.flags.internalSetter) {
    HiddenClass *clazz = vmcast<HiddenClass>(clazzPtr.getNonNull(this));
    if (LLVM_LIKELY(!clazz->isDictionary())) {
      // Cache the class, id and property slot.
      cacheEntry->clazz = clazzPtr;
      cacheEntry->slot = desc.slot;
    }
    return JSObject::getNamedSlotValue(createPseudoHandle(*obj), this, desc);
  }
  return JSObject::getNamed_RJS(obj, this, sym);
}

ExecutionStatus Runtime::putNamedThrowOnError(
    Handle<JSObject> obj,
    PropCacheID id,
    HermesValue hv) {
  CompressedPointer clazzPtr{obj->getClassGCPtr()};
  auto *cacheEntry = &fixedPropCache_[static_cast<int>(id)];
  if (LLVM_LIKELY(cacheEntry->clazz == clazzPtr)) {
    auto shv = SmallHermesValue::encodeHermesValue(hv, this);
    JSObject::setNamedSlotValueUnsafe<PropStorage::Inline::Yes>(
        *obj, this, cacheEntry->slot, shv);
    return ExecutionStatus::RETURNED;
  }
  auto sym = Predefined::getSymbolID(fixedPropCacheNames[static_cast<int>(id)]);
  NamedPropertyDescriptor desc;
  if (LLVM_LIKELY(
          JSObject::tryGetOwnNamedDescriptorFast(*obj, this, sym, desc)) &&
      !desc.flags.accessor && desc.flags.writable &&
      !desc.flags.internalSetter) {
    HiddenClass *clazz = vmcast<HiddenClass>(clazzPtr.getNonNull(this));
    if (LLVM_LIKELY(!clazz->isDictionary())) {
      // Cache the class and property slot.
      cacheEntry->clazz = clazzPtr;
      cacheEntry->slot = desc.slot;
    }
    auto shv = SmallHermesValue::encodeHermesValue(hv, this);
    JSObject::setNamedSlotValueUnsafe(*obj, this, desc.slot, shv);
    return ExecutionStatus::RETURNED;
  }
  return JSObject::putNamed_RJS(
             obj, this, sym, makeHandle(hv), PropOpFlags().plusThrowOnError())
      .getStatus();
}

Runtime::Runtime(
    std::shared_ptr<StorageProvider> provider,
    const RuntimeConfig &runtimeConfig)
    // The initial heap size can't be larger than the max.
    : enableEval(runtimeConfig.getEnableEval()),
      verifyEvalIR(runtimeConfig.getVerifyEvalIR()),
      optimizedEval(runtimeConfig.getOptimizedEval()),
      asyncBreakCheckInEval(runtimeConfig.getAsyncBreakCheckInEval()),
      heapStorage_(
          this,
          this,
          runtimeConfig.getGCConfig(),
          runtimeConfig.getCrashMgr(),
          std::move(provider),
          runtimeConfig.getVMExperimentFlags()),
      hasES6Promise_(runtimeConfig.getES6Promise()),
      hasES6Proxy_(runtimeConfig.getES6Proxy()),
      hasIntl_(runtimeConfig.getIntl()),
      shouldRandomizeMemoryLayout_(runtimeConfig.getRandomizeMemoryLayout()),
      bytecodeWarmupPercent_(runtimeConfig.getBytecodeWarmupPercent()),
      trackIO_(runtimeConfig.getTrackIO()),
      vmExperimentFlags_(runtimeConfig.getVMExperimentFlags()),
      runtimeStats_(runtimeConfig.getEnableSampledStats()),
      commonStorage_(
          createRuntimeCommonStorage(runtimeConfig.getTraceEnabled())),
      stackPointer_(),
      crashMgr_(runtimeConfig.getCrashMgr()),
      crashCallbackKey_(
          crashMgr_->registerCallback([this](int fd) { crashCallback(fd); })),
      codeCoverageProfiler_(std::make_unique<CodeCoverageProfiler>(this)),
      gcEventCallback_(runtimeConfig.getGCConfig().getCallback()),
      allowFunctionToStringWithRuntimeSource_(
          runtimeConfig.getAllowFunctionToStringWithRuntimeSource()) {
  assert(
      (void *)this == (void *)(HandleRootOwner *)this &&
      "cast to HandleRootOwner should be no-op");
#ifdef HERMES_FACEBOOK_BUILD
  const bool isSnapshot = std::strstr(__FILE__, "hermes-snapshot");
  crashMgr_->setCustomData("HermesIsSnapshot", isSnapshot ? "true" : "false");
#endif
  auto maxNumRegisters = runtimeConfig.getMaxNumRegisters();
  if (LLVM_UNLIKELY(maxNumRegisters > kMaxSupportedNumRegisters)) {
    hermes_fatal("RuntimeConfig maxNumRegisters too big");
  }
  registerStack_ = runtimeConfig.getRegisterStack();
  if (!registerStack_) {
    // registerStack_ should not be allocated with new, because then
    // default constructors would run for the whole stack space.
    // Round up to page size as required by vm_allocate.
    const auto numBytesForRegisters = llvh::alignTo(
        sizeof(PinnedHermesValue) * maxNumRegisters, oscompat::page_size());
    auto result = oscompat::vm_allocate(numBytesForRegisters);
    if (!result) {
      hermes_fatal("Failed to allocate register stack", result.getError());
    }
    registerStack_ = static_cast<PinnedHermesValue *>(result.get());
    registerStackBytesToUnmap_ = numBytesForRegisters;
    crashMgr_->registerMemory(registerStack_, numBytesForRegisters);
  } else {
    registerStackBytesToUnmap_ = 0;
  }

  registerStackEnd_ = registerStack_ + maxNumRegisters;
  if (shouldRandomizeMemoryLayout_) {
    const unsigned bytesOff = std::random_device()() % oscompat::page_size();
    registerStackEnd_ -= bytesOff / sizeof(PinnedHermesValue);
    assert(registerStackEnd_ >= registerStack_ && "register stack too small");
  }
  stackPointer_ = registerStackEnd_;

  // Setup the "root" stack frame.
  setCurrentFrameToTopOfStack();
  // Allocate the "reserved" registers in the root frame.
  allocStack(
      StackFrameLayout::CalleeExtraRegistersAtStart,
      HermesValue::encodeUndefinedValue());

#ifdef HERMESVM_SERIALIZE
  if (runtimeConfig.getDeserializeFile()) {
    assert(
        runtimeConfig.getExternalPointersVectorCallBack() &&
        "missing function pointer to map external pointers.");
    // If there is a serialized heap file available, use that to initialize
    // Runtime instead of re-creating the Runtime.
    Deserializer d(
        runtimeConfig.getDeserializeFile(),
        this,
        runtimeConfig.getExternalPointersVectorCallBack());
    deserializeImpl(d, runtimeConfig.getGCConfig().getAllocInYoung());

    LLVM_DEBUG(llvh::dbgs() << "Runtime initialized\n");

    if (runtimeConfig.getEnableSampleProfiling())
      samplingProfiler = std::make_unique<SamplingProfiler>(this);

    return;
  }
#endif // HERMESVM_SERIALIZE

  // Initialize Predefined Strings.
  // This function does not do any allocations.
  initPredefinedStrings();
  // Initialize special code blocks pointing to their own runtime module.
  // specialCodeBlockRuntimeModule_ will be owned by runtimeModuleList_.
  RuntimeModuleFlags flags;
  flags.hidesEpilogue = true;
  specialCodeBlockDomain_ = Domain::create(this).getHermesValue();
  specialCodeBlockRuntimeModule_ = RuntimeModule::createUninitialized(
      this, Handle<Domain>::vmcast(&specialCodeBlockDomain_), flags);
  assert(
      &runtimeModuleList_.back() == specialCodeBlockRuntimeModule_ &&
      "specialCodeBlockRuntimeModule_ not added to runtimeModuleList_");

  // At this point, allocations can begin, as all the roots are markable.

  // Initialize the pre-allocated character strings.
  initCharacterStrings();

  GCScope scope(this);

  // Explicitly initialize the specialCodeBlockRuntimeModule_ without CJS
  // modules.
  specialCodeBlockRuntimeModule_->initializeWithoutCJSModulesMayAllocate(
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          generateSpecialRuntimeBytecode())
          .first);
  emptyCodeBlock_ = specialCodeBlockRuntimeModule_->getCodeBlockMayAllocate(0);
  returnThisCodeBlock_ =
      specialCodeBlockRuntimeModule_->getCodeBlockMayAllocate(1);

  // Initialize the root hidden class and its variants.
  {
    MutableHandle<HiddenClass> clazz(
        this,
        vmcast<HiddenClass>(
            ignoreAllocationFailure(HiddenClass::createRoot(this))));
    rootClazzes_[0] = clazz.getHermesValue();
    for (unsigned i = 1; i <= InternalProperty::NumInternalProperties; ++i) {
      auto addResult = HiddenClass::reserveSlot(clazz, this);
      assert(
          addResult != ExecutionStatus::EXCEPTION &&
          "Could not possibly grow larger than the limit");
      clazz = *addResult->first;
      rootClazzes_[i] = clazz.getHermesValue();
    }
  }

  global_ =
      JSObject::create(this, Handle<JSObject>(this, nullptr)).getHermesValue();

  JSLibFlags jsLibFlags{};
  jsLibFlags.enableHermesInternal = runtimeConfig.getEnableHermesInternal();
  jsLibFlags.enableHermesInternalTestMethods =
      runtimeConfig.getEnableHermesInternalTestMethods();
  initGlobalObject(this, jsLibFlags);

  // Once the global object has been initialized, populate native builtins to
  // the builtins table.
  initNativeBuiltins();

  stringCycleCheckVisited_ =
      ignoreAllocationFailure(ArrayStorage::create(this, 8));

  // Set the prototype of the global object to the standard object prototype,
  // which has now been defined.
  ignoreAllocationFailure(JSObject::setParent(
      vmcast<JSObject>(global_),
      this,
      vmcast<JSObject>(objectPrototype),
      PropOpFlags().plusThrowOnError()));

  symbolRegistry_.init(this);

#ifdef HERMESVM_SERIALIZE
  if (runtimeConfig.getSerializeAfterInitFile()) {
    assert(
        runtimeConfig.getExternalPointersVectorCallBack() &&
        "missing function pointer to map external pointers.");
    Serializer s(
        *runtimeConfig.getSerializeAfterInitFile(),
        this,
        runtimeConfig.getExternalPointersVectorCallBack());
    serialize(s);
  }
#endif // HERMESVM_SERIALIZE

  // BB Profiler need to be ready before running internal bytecode.
#ifdef HERMESVM_PROFILER_BB
  inlineCacheProfiler_.setHiddenClassArray(
      ignoreAllocationFailure(JSArray::create(this, 4, 4)).get());
#endif

  codeCoverageProfiler_->disable();
  // Execute our internal bytecode.
  auto jsBuiltinsObj = runInternalBytecode();
  codeCoverageProfiler_->restore();

  // Populate JS builtins returned from internal bytecode to the builtins table.
  initJSBuiltins(builtins_, jsBuiltinsObj);

  if (runtimeConfig.getEnableSampleProfiling())
    samplingProfiler = std::make_unique<SamplingProfiler>(this);

  LLVM_DEBUG(llvh::dbgs() << "Runtime initialized\n");
}

Runtime::~Runtime() {
  samplingProfiler.reset();
  getHeap().finalizeAll();
  // Now that all objects are finalized, there shouldn't be any native memory
  // keys left in the ID tracker for memory profiling. Assert that the only IDs
  // left are JS heap pointers.
  assert(
      !getHeap().getIDTracker().hasNativeIDs() &&
      "A pointer is left in the ID tracker that is from non-JS memory. "
      "Was untrackNative called?");
  crashMgr_->unregisterCallback(crashCallbackKey_);
  if (registerStackBytesToUnmap_ > 0) {
    crashMgr_->unregisterMemory(registerStack_);
    oscompat::vm_free(registerStack_, registerStackBytesToUnmap_);
  }
  // Remove inter-module dependencies so we can delete them in any order.
  for (auto &module : runtimeModuleList_) {
    module.prepareForRuntimeShutdown();
  }

  while (!runtimeModuleList_.empty()) {
    // Calling delete will automatically remove it from the list.
    delete &runtimeModuleList_.back();
  }

  for (auto callback : destructionCallbacks_) {
    callback(this);
  }
}

/// A helper class used to measure the duration of GC marking different roots.
/// It accumulates the times in \c Runtime::markRootsPhaseTimes[] and \c
/// Runtime::totalMarkRootsTime.
class Runtime::MarkRootsPhaseTimer {
 public:
  MarkRootsPhaseTimer(Runtime *rt, RootAcceptor::Section section)
      : rt_(rt), section_(section), start_(std::chrono::steady_clock::now()) {
    if (static_cast<unsigned>(section) == 0) {
      // The first phase; record the start as the start of markRoots.
      rt_->startOfMarkRoots_ = start_;
    }
  }
  ~MarkRootsPhaseTimer() {
    auto tp = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = (tp - start_);
    start_ = tp;
    unsigned index = static_cast<unsigned>(section_);
    rt_->markRootsPhaseTimes_[index] += elapsed.count();
    if (index + 1 ==
        static_cast<unsigned>(RootAcceptor::Section::NumSections)) {
      std::chrono::duration<double> totalElapsed =
          (tp - rt_->startOfMarkRoots_);
      rt_->totalMarkRootsTime_ += totalElapsed.count();
    }
  }

 private:
  Runtime *rt_;
  RootAcceptor::Section section_;
  std::chrono::time_point<std::chrono::steady_clock> start_;
};

void Runtime::markRoots(
    RootAndSlotAcceptorWithNames &acceptor,
    bool markLongLived) {
  // The body of markRoots should be sequence of blocks, each of which starts
  // with the declaration of an appropriate RootSection instance.
  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::Registers);
    acceptor.beginRootSection(RootAcceptor::Section::Registers);
    for (auto *p = stackPointer_, *e = registerStackEnd_; p != e; ++p)
      acceptor.accept(*p);
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::RuntimeInstanceVars);
    acceptor.beginRootSection(RootAcceptor::Section::RuntimeInstanceVars);
    for (auto &clazz : rootClazzes_)
      acceptor.accept(clazz, "rootClass");
#define RUNTIME_HV_FIELD_INSTANCE(name) acceptor.accept((name), #name);
#include "hermes/VM/RuntimeHermesValueFields.def"
#undef RUNTIME_HV_FIELD_INSTANCE
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::RuntimeModules);
    acceptor.beginRootSection(RootAcceptor::Section::RuntimeModules);
#define RUNTIME_HV_FIELD_RUNTIMEMODULE(name) acceptor.accept(name);
#include "hermes/VM/RuntimeHermesValueFields.def"
#undef RUNTIME_HV_FIELD_RUNTIMEMODULE
    for (auto &rm : runtimeModuleList_)
      rm.markRoots(acceptor, markLongLived);
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::CharStrings);
    acceptor.beginRootSection(RootAcceptor::Section::CharStrings);
    if (markLongLived) {
      for (auto &hv : charStrings_)
        acceptor.accept(hv);
    }
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::Builtins);
    acceptor.beginRootSection(RootAcceptor::Section::Builtins);
    for (Callable *&f : builtins_)
      acceptor.acceptPtr(f);
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::Jobs);
    acceptor.beginRootSection(RootAcceptor::Section::Jobs);
    for (Callable *&f : jobQueue_)
      acceptor.acceptPtr(f);
    acceptor.endRootSection();
  }

#ifdef MARK
#error "Shouldn't have defined mark already"
#endif
#define MARK(field) acceptor.accept((field), #field)
  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::Prototypes);
    acceptor.beginRootSection(RootAcceptor::Section::Prototypes);
    // Prototypes.
#define RUNTIME_HV_FIELD_PROTOTYPE(name) MARK(name);
#include "hermes/VM/RuntimeHermesValueFields.def"
#undef RUNTIME_HV_FIELD_PROTOTYPE
    acceptor.acceptPtr(objectPrototypeRawPtr, "objectPrototype");
    acceptor.acceptPtr(functionPrototypeRawPtr, "functionPrototype");
#undef MARK
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::IdentifierTable);
    if (markLongLived) {
      // Need to add nodes before the root section, and edges during the root
      // section.
      acceptor.provideSnapshot([this](HeapSnapshot &snap) {
        identifierTable_.snapshotAddNodes(snap);
      });
      acceptor.beginRootSection(RootAcceptor::Section::IdentifierTable);
      identifierTable_.markIdentifiers(acceptor, &getHeap());
      acceptor.provideSnapshot([this](HeapSnapshot &snap) {
        identifierTable_.snapshotAddEdges(snap);
      });
      acceptor.endRootSection();
    }
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::GCScopes);
    acceptor.beginRootSection(RootAcceptor::Section::GCScopes);
    markGCScopes(acceptor);
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::SymbolRegistry);
    acceptor.beginRootSection(RootAcceptor::Section::SymbolRegistry);
    symbolRegistry_.markRoots(acceptor);
    acceptor.endRootSection();
  }

  // Mark the alternative roots during the normal mark roots call.
  markRootsForCompleteMarking(acceptor);

  {
    MarkRootsPhaseTimer timer(
        this, RootAcceptor::Section::CodeCoverageProfiler);
    acceptor.beginRootSection(RootAcceptor::Section::CodeCoverageProfiler);
    if (codeCoverageProfiler_) {
      codeCoverageProfiler_->markRoots(acceptor);
    }
#ifdef HERMESVM_PROFILER_BB
    auto *&hiddenClassArray = inlineCacheProfiler_.getHiddenClassArray();
    if (hiddenClassArray) {
      acceptor.acceptPtr(hiddenClassArray);
    }
#endif
    acceptor.endRootSection();
  }

  {
    MarkRootsPhaseTimer timer(this, RootAcceptor::Section::Custom);
    // Define nodes before the root section starts.
    for (auto &fn : customSnapshotNodeFuncs_) {
      acceptor.provideSnapshot(fn);
    }
    acceptor.beginRootSection(RootAcceptor::Section::Custom);
    for (auto &fn : customMarkRootFuncs_)
      fn(&getHeap(), acceptor);
    // Define edges while inside the root section.
    for (auto &fn : customSnapshotEdgeFuncs_) {
      acceptor.provideSnapshot(fn);
    }
    acceptor.endRootSection();
  }
}

void Runtime::markWeakRoots(WeakRootAcceptor &acceptor, bool markLongLived) {
  MarkRootsPhaseTimer timer(this, RootAcceptor::Section::WeakRefs);
  acceptor.beginRootSection(RootAcceptor::Section::WeakRefs);
  if (markLongLived) {
    for (auto &entry : fixedPropCache_) {
      acceptor.acceptWeak(entry.clazz);
    }
    for (auto &rm : runtimeModuleList_)
      rm.markWeakRoots(acceptor);
  }
  for (auto &fn : customMarkWeakRootFuncs_)
    fn(&getHeap(), acceptor);
  acceptor.endRootSection();
}

void Runtime::markRootsForCompleteMarking(
    RootAndSlotAcceptorWithNames &acceptor) {
  MarkRootsPhaseTimer timer(this, RootAcceptor::Section::SamplingProfiler);
  acceptor.beginRootSection(RootAcceptor::Section::SamplingProfiler);
  if (samplingProfiler) {
    samplingProfiler->markRoots(acceptor);
  }
  acceptor.endRootSection();
}

void Runtime::visitIdentifiers(
    const std::function<void(SymbolID, const StringPrimitive *)> &acceptor) {
  identifierTable_.visitIdentifiers(acceptor);
}

std::string Runtime::convertSymbolToUTF8(SymbolID id) {
  return identifierTable_.convertSymbolToUTF8(id);
}

void Runtime::printRuntimeGCStats(JSONEmitter &json) const {
  const unsigned kNumPhases =
      static_cast<unsigned>(RootAcceptor::Section::NumSections);
#define ROOT_SECTION(phase) "MarkRoots_" #phase,
  static const char *markRootsPhaseNames[kNumPhases] = {
#include "hermes/VM/RootSections.def"
  };
#undef ROOT_SECTION
  json.emitKey("runtime");
  json.openDict();
  json.emitKeyValue("totalMarkRootsTime", formatSecs(totalMarkRootsTime_).secs);
  for (unsigned phaseNum = 0; phaseNum < kNumPhases; phaseNum++) {
    json.emitKeyValue(
        std::string(markRootsPhaseNames[phaseNum]) + "Time",
        formatSecs(markRootsPhaseTimes_[phaseNum]).secs);
  }
  json.closeDict();
}

void Runtime::printHeapStats(llvh::raw_ostream &os) {
  // Printing the timings is unstable.
  if (shouldStabilizeInstructionCount())
    return;
  getHeap().printAllCollectedStats(os);
#ifndef NDEBUG
  printArrayCensus(llvh::outs());
#endif
  if (trackIO_) {
    getIOTrackingInfoJSON(os);
  }
}

void Runtime::getIOTrackingInfoJSON(llvh::raw_ostream &os) {
  JSONEmitter json(os);
  json.openArray();
  for (auto &module : getRuntimeModules()) {
    auto tracker = module.getBytecode()->getPageAccessTracker();
    if (tracker) {
      json.openDict();
      json.emitKeyValue("url", module.getSourceURL());
      json.emitKey("tracking_info");
      tracker->getJSONStats(json);
      json.closeDict();
    }
  }
  json.closeArray();
}

void Runtime::removeRuntimeModule(RuntimeModule *rm) {
#ifdef HERMES_ENABLE_DEBUGGER
  debugger_.willUnloadModule(rm);
#endif
  runtimeModuleList_.remove(*rm);
}

#ifndef NDEBUG
void Runtime::printArrayCensus(llvh::raw_ostream &os) {
  // Do array capacity histogram.
  // Map from array size to number of arrays that are that size.
  // Arrays includes ArrayStorage and SegmentedArray.
  std::map<std::pair<size_t, size_t>, std::pair<size_t, size_t>>
      arraySizeToCountAndWastedSlots;
  auto printTable = [&os](const std::map<
                          std::pair<size_t, size_t>,
                          std::pair<size_t, size_t>>
                              &arraySizeToCountAndWastedSlots) {
    os << llvh::format(
        "%8s %8s %8s %10s %15s %15s %15s %20s %25s\n",
        (const char *)"Capacity",
        (const char *)"Sizeof",
        (const char *)"Count",
        (const char *)"Count %",
        (const char *)"Cum Count %",
        (const char *)"Bytes %",
        (const char *)"Cum Bytes %",
        (const char *)"Wasted Slots %",
        (const char *)"Cum Wasted Slots %");
    size_t totalBytes = 0;
    size_t totalCount = 0;
    size_t totalWastedSlots = 0;
    for (const auto &p : arraySizeToCountAndWastedSlots) {
      totalBytes += p.first.second * p.second.first;
      totalCount += p.second.first;
      totalWastedSlots += p.second.second;
    }
    size_t cumulativeBytes = 0;
    size_t cumulativeCount = 0;
    size_t cumulativeWastedSlots = 0;
    for (const auto &p : arraySizeToCountAndWastedSlots) {
      cumulativeBytes += p.first.second * p.second.first;
      cumulativeCount += p.second.first;
      cumulativeWastedSlots += p.second.second;
      os << llvh::format(
          "%8d %8d %8d %9.2f%% %14.2f%% %14.2f%% %14.2f%% %19.2f%% %24.2f%%\n",
          p.first.first,
          p.first.second,
          p.second.first,
          p.second.first * 100.0 / totalCount,
          cumulativeCount * 100.0 / totalCount,
          p.first.second * p.second.first * 100.0 / totalBytes,
          cumulativeBytes * 100.0 / totalBytes,
          totalWastedSlots ? p.second.second * 100.0 / totalWastedSlots : 100.0,
          totalWastedSlots ? cumulativeWastedSlots * 100.0 / totalWastedSlots
                           : 100.0);
    }
    os << "\n";
  };

  os << "Array Census for ArrayStorage:\n";
  getHeap().forAllObjs([&arraySizeToCountAndWastedSlots](GCCell *cell) {
    if (cell->getKind() == CellKind::ArrayStorageKind) {
      ArrayStorage *arr = vmcast<ArrayStorage>(cell);
      const auto key = std::make_pair(arr->capacity(), arr->getAllocatedSize());
      arraySizeToCountAndWastedSlots[key].first++;
      arraySizeToCountAndWastedSlots[key].second +=
          arr->capacity() - arr->size();
    }
  });
  if (arraySizeToCountAndWastedSlots.empty()) {
    os << "\tNo ArrayStorages\n\n";
  } else {
    printTable(arraySizeToCountAndWastedSlots);
  }

  os << "Array Census for SegmentedArray:\n";
  arraySizeToCountAndWastedSlots.clear();
  getHeap().forAllObjs([&arraySizeToCountAndWastedSlots](GCCell *cell) {
    if (cell->getKind() == CellKind::SegmentedArrayKind) {
      SegmentedArray *arr = vmcast<SegmentedArray>(cell);
      const auto key =
          std::make_pair(arr->totalCapacityOfSpine(), arr->getAllocatedSize());
      arraySizeToCountAndWastedSlots[key].first++;
      arraySizeToCountAndWastedSlots[key].second +=
          arr->totalCapacityOfSpine() - arr->size();
    }
  });
  if (arraySizeToCountAndWastedSlots.empty()) {
    os << "\tNo SegmentedArrays\n\n";
  } else {
    printTable(arraySizeToCountAndWastedSlots);
  }

  os << "Array Census for Segment:\n";
  arraySizeToCountAndWastedSlots.clear();
  getHeap().forAllObjs([&arraySizeToCountAndWastedSlots](GCCell *cell) {
    if (cell->getKind() == CellKind::SegmentKind) {
      SegmentedArray::Segment *seg = vmcast<SegmentedArray::Segment>(cell);
      const auto key = std::make_pair(seg->length(), seg->getAllocatedSize());
      arraySizeToCountAndWastedSlots[key].first++;
      arraySizeToCountAndWastedSlots[key].second +=
          SegmentedArray::Segment::kMaxLength - seg->length();
    }
  });
  if (arraySizeToCountAndWastedSlots.empty()) {
    os << "\tNo Segments\n\n";
  } else {
    printTable(arraySizeToCountAndWastedSlots);
  }

  os << "Array Census for JSArray:\n";
  arraySizeToCountAndWastedSlots.clear();
  getHeap().forAllObjs([&arraySizeToCountAndWastedSlots, this](GCCell *cell) {
    if (cell->getKind() == CellKind::ArrayKind) {
      JSArray *arr = vmcast<JSArray>(cell);
      JSArray::StorageType *storage = arr->getIndexedStorage(this);
      const auto capacity = storage ? storage->totalCapacityOfSpine() : 0;
      const auto sz = storage ? storage->size() : 0;
      const auto key = std::make_pair(capacity, arr->getAllocatedSize());
      arraySizeToCountAndWastedSlots[key].first++;
      arraySizeToCountAndWastedSlots[key].second += capacity - sz;
    }
  });
  if (arraySizeToCountAndWastedSlots.empty()) {
    os << "\tNo JSArrays\n\n";
  } else {
    printTable(arraySizeToCountAndWastedSlots);
  }

  os << "\n";
}
#endif

unsigned Runtime::getSymbolsEnd() const {
  return identifierTable_.getSymbolsEnd();
}

void Runtime::unmarkSymbols() {
  identifierTable_.unmarkSymbols();
}

void Runtime::freeSymbols(const llvh::BitVector &markedSymbols) {
  identifierTable_.freeUnmarkedSymbols(markedSymbols, getHeap().getIDTracker());
}

#ifdef HERMES_SLOW_DEBUG
bool Runtime::isSymbolLive(SymbolID id) {
  return identifierTable_.isSymbolLive(id);
}

const void *Runtime::getStringForSymbol(SymbolID id) {
  return identifierTable_.getStringForSymbol(id);
}
#endif

size_t Runtime::mallocSize() const {
  // Register stack uses mmap and RuntimeModules are tracked by their owning
  // Domains. So this only considers IdentifierTable size.
  return sizeof(IdentifierTable) + identifierTable_.additionalMemorySize();
}

#ifdef HERMESVM_SANITIZE_HANDLES
void Runtime::potentiallyMoveHeap() {
  // Do a dummy allocation which could force a heap move if handle sanitization
  // is on.
  FillerCell::create(
      this,
      std::max<size_t>(
          heapAlignSize(sizeof(FillerCell)), GC::minAllocationSize()));
}
#endif

bool Runtime::shouldStabilizeInstructionCount() {
  return getCommonStorage()->env &&
      getCommonStorage()->env->stabilizeInstructionCount;
}

void Runtime::setMockedEnvironment(const MockedEnvironment &env) {
  getCommonStorage()->env = env;
}

LLVM_ATTRIBUTE_NOINLINE
static CallResult<HermesValue> interpretFunctionWithRandomStack(
    Runtime *runtime,
    CodeBlock *globalCode) {
  static void *volatile dummy;
  const unsigned amount = std::random_device()() % oscompat::page_size();
  // Prevent compiler from optimizing alloca away by assigning to volatile
  dummy = alloca(amount);
  (void)dummy;
  return runtime->interpretFunction(globalCode);
}

CallResult<HermesValue> Runtime::run(
    llvh::StringRef code,
    llvh::StringRef sourceURL,
    hbc::CompileFlags compileFlags) {
#ifdef HERMESVM_LEAN
  return raiseEvalUnsupported(code);
#else
  compileFlags.allowFunctionToStringWithRuntimeSource |=
      getAllowFunctionToStringWithRuntimeSource();
  std::unique_ptr<hermes::Buffer> buffer;
  if (compileFlags.lazy ||
      compileFlags.allowFunctionToStringWithRuntimeSource) {
    buffer.reset(new hermes::OwnedMemoryBuffer(
        llvh::MemoryBuffer::getMemBufferCopy(code)));
  } else {
    buffer.reset(
        new hermes::OwnedMemoryBuffer(llvh::MemoryBuffer::getMemBuffer(code)));
  }
  return run(std::move(buffer), sourceURL, compileFlags);
#endif
}

CallResult<HermesValue> Runtime::run(
    std::unique_ptr<hermes::Buffer> code,
    llvh::StringRef sourceURL,
    hbc::CompileFlags compileFlags) {
#ifdef HERMESVM_LEAN
  auto buffer = code.get();
  return raiseEvalUnsupported(llvh::StringRef(
      reinterpret_cast<const char *>(buffer->data()), buffer->size()));
#else
  compileFlags.allowFunctionToStringWithRuntimeSource |=
      getAllowFunctionToStringWithRuntimeSource();
  std::unique_ptr<hbc::BCProviderFromSrc> bytecode;
  {
    PerfSection loading("Loading new JavaScript code");
    loading.addArg("url", sourceURL);
    auto bytecode_err = hbc::BCProviderFromSrc::createBCProviderFromSrc(
        std::move(code), sourceURL, compileFlags);
    if (!bytecode_err.first) {
      return raiseSyntaxError(TwineChar16(bytecode_err.second));
    }
    bytecode = std::move(bytecode_err.first);
  }

  PerfSection loading("Executing global function");
  RuntimeModuleFlags rmflags;
  rmflags.persistent = true;
  return runBytecode(
      std::move(bytecode), rmflags, sourceURL, makeNullHandle<Environment>());
#endif
}

CallResult<HermesValue> Runtime::runBytecode(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    RuntimeModuleFlags flags,
    llvh::StringRef sourceURL,
    Handle<Environment> environment,
    Handle<> thisArg) {
  clearThrownValue();

#ifdef HERMESVM_SERIALIZE
  // If we are constructed from serialize data with a ClosureFunction, execute
  // the function.
  if (!serializeClosure.isUndefined()) {
    ScopedNativeCallFrame newFrame{
        this,
        0,
        serializeClosure,
        HermesValue::encodeUndefinedValue(),
        *thisArg};
    if (LLVM_UNLIKELY(newFrame.overflowed()))
      return raiseStackOverflow(StackOverflowKind::NativeStack);
    return shouldRandomizeMemoryLayout_
        ? interpretFunctionWithRandomStack(
              this, vmcast<JSFunction>(serializeClosure)->getCodeBlock())
        : interpretFunction(
              vmcast<JSFunction>(serializeClosure)->getCodeBlock());
  }
#endif

  auto globalFunctionIndex = bytecode->getGlobalFunctionIndex();

  if (bytecode->getBytecodeOptions().staticBuiltins && !builtinsFrozen_) {
    if (assertBuiltinsUnmodified() == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    freezeBuiltins();
    assert(builtinsFrozen_ && "Builtins must be frozen by now.");
  }

  if (bytecode->getBytecodeOptions().hasAsync && !hasES6Promise_) {
    return raiseTypeError(
        "Cannot execute a bytecode having async functions when Promise is disabled.");
  }

  if (flags.persistent) {
    persistentBCProviders_.push_back(bytecode);
    if (bytecodeWarmupPercent_ > 0) {
      // Start the warmup thread for this bytecode if it's a buffer.
      bytecode->startWarmup(bytecodeWarmupPercent_);
    }
    if (getVMExperimentFlags() & experiments::MAdviseRandom) {
      bytecode->madvise(oscompat::MAdvice::Random);
    } else if (getVMExperimentFlags() & experiments::MAdviseSequential) {
      bytecode->madvise(oscompat::MAdvice::Sequential);
    }
    if (getVMExperimentFlags() & experiments::VerifyBytecodeChecksum) {
      llvh::ArrayRef<uint8_t> buf = bytecode->getRawBuffer();
      // buf is empty for non-buffer providers
      if (!buf.empty()) {
        if (!hbc::BCProviderFromBuffer::bytecodeHashIsValid(buf)) {
          const char *msg = "Bytecode checksum verification failed";
          hermesLog("Hermes", "%s", msg);
          hermes_fatal(msg);
        }
      }
    }
  }
  // Only track I/O for buffers > 64 kB (which excludes things like
  // Runtime::generateSpecialRuntimeBytecode).
  if (flags.persistent && trackIO_ &&
      bytecode->getRawBuffer().size() > MIN_IO_TRACKING_SIZE) {
    bytecode->startPageAccessTracker();
    if (!bytecode->getPageAccessTracker()) {
      hermesLog(
          "Hermes",
          "Failed to start bytecode I/O instrumentation, "
          "maybe not supported on this platform.");
    }
  }

  GCScope scope(this);

  Handle<Domain> domain = makeHandle(Domain::create(this));

  auto runtimeModuleRes = RuntimeModule::create(
      this, domain, nextScriptId_++, std::move(bytecode), flags, sourceURL);
  if (LLVM_UNLIKELY(runtimeModuleRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto runtimeModule = *runtimeModuleRes;
  auto globalCode = runtimeModule->getCodeBlockMayAllocate(globalFunctionIndex);

#ifdef HERMES_ENABLE_DEBUGGER
  // If the debugger is configured to pause on load, give it a chance to pause.
  getDebugger().willExecuteModule(runtimeModule, globalCode);
#endif

  if (runtimeModule->hasCJSModules()) {
    auto requireContext = RequireContext::create(
        this, domain, getPredefinedStringHandle(Predefined::dotSlash));
    return runRequireCall(
        this,
        requireContext,
        domain,
        *domain->getCJSModuleOffset(this, domain->getCJSEntryModuleID()));
  } else if (runtimeModule->hasCJSModulesStatic()) {
    return runRequireCall(
        this,
        makeNullHandle<RequireContext>(),
        domain,
        *domain->getCJSModuleOffset(this, domain->getCJSEntryModuleID()));
  } else {
    // Create a JSFunction which will reference count the runtime module.
    // Note that its handle gets registered in the scope, so we don't need to
    // save it. Also note that environment will often be null here, except if
    // this is local eval.
    auto func = JSFunction::create(
        this,
        domain,
        Handle<JSObject>::vmcast(&functionPrototype),
        environment,
        globalCode);

    ScopedNativeCallFrame newFrame{
        this,
        0,
        func.getHermesValue(),
        HermesValue::encodeUndefinedValue(),
        *thisArg};
    if (LLVM_UNLIKELY(newFrame.overflowed()))
      return raiseStackOverflow(StackOverflowKind::NativeStack);
    return shouldRandomizeMemoryLayout_
        ? interpretFunctionWithRandomStack(this, globalCode)
        : interpretFunction(globalCode);
  }
}

ExecutionStatus Runtime::loadSegment(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    Handle<RequireContext> requireContext,
    RuntimeModuleFlags flags) {
  GCScopeMarkerRAII marker{this};
  auto domain = makeHandle(RequireContext::getDomain(this, *requireContext));

  if (LLVM_UNLIKELY(
          RuntimeModule::create(
              this, domain, nextScriptId_++, std::move(bytecode), flags, "") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return ExecutionStatus::RETURNED;
}

Handle<JSObject> Runtime::runInternalBytecode() {
  auto module = getInternalBytecode();
  std::pair<std::unique_ptr<hbc::BCProvider>, std::string> bcResult =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::make_unique<Buffer>(module.data(), module.size()));
  if (LLVM_UNLIKELY(!bcResult.first)) {
    hermes_fatal((llvh::Twine("Error running internal bytecode: ") +
                  bcResult.second.c_str())
                     .str());
  }
  // The bytes backing our buffer are immortal, so we can be persistent.
  RuntimeModuleFlags flags;
  flags.persistent = true;
  flags.hidesEpilogue = true;
  auto res = runBytecode(
      std::move(bcResult.first),
      flags,
      /*sourceURL*/ "",
      makeNullHandle<Environment>());
  // It is a fatal error for the internal bytecode to throw an exception.
  assert(
      res != ExecutionStatus::EXCEPTION && "Internal bytecode threw exception");
  assert(
      res->isObject() &&
      "Completion value of internal bytecode must be an object");

  return makeHandle<JSObject>(*res);
}

void Runtime::printException(llvh::raw_ostream &os, Handle<> valueHandle) {
  os << "Uncaught ";
  clearThrownValue();

  // Try to fetch the stack trace.
  CallResult<PseudoHandle<>> propRes{ExecutionStatus::EXCEPTION};
  if (auto objHandle = Handle<JSObject>::dyn_vmcast(valueHandle)) {
    if (LLVM_UNLIKELY(
            (propRes = JSObject::getNamed_RJS(
                 objHandle,
                 this,
                 Predefined::getSymbolID(Predefined::stack))) ==
            ExecutionStatus::EXCEPTION)) {
      os << "exception thrown while getting stack trace\n";
      return;
    }
  }
  SmallU16String<32> tmp;
  if (LLVM_UNLIKELY(
          propRes == ExecutionStatus::EXCEPTION || (*propRes)->isUndefined())) {
    // If stack trace is unavailable, we just print error.toString.
    auto strRes = toString_RJS(this, valueHandle);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      os << "exception thrown in toString of original exception\n";
      return;
    }

    strRes->get()->appendUTF16String(tmp);
    os << tmp << "\n";
    return;
  }
  // stack trace is available, try to convert it to string.
  auto strRes = toString_RJS(this, makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    os << "exception thrown in toString of stack trace\n";
    return;
  }
  PseudoHandle<StringPrimitive> str = std::move(*strRes);
  if (str->getStringLength() == 0) {
    str.invalidate();
    // If the final value is the empty string,
    // fall back to just printing the error.toString directly.
    auto errToStringRes = toString_RJS(this, valueHandle);
    if (LLVM_UNLIKELY(errToStringRes == ExecutionStatus::EXCEPTION)) {
      os << "exception thrown in toString of original exception\n";
      return;
    }
    str = std::move(*errToStringRes);
  }
  str->appendUTF16String(tmp);
  os << tmp << "\n";
}

Handle<JSObject> Runtime::getGlobal() {
  return Handle<JSObject>::vmcast(&global_);
}

std::vector<llvh::ArrayRef<uint8_t>> Runtime::getEpilogues() {
  std::vector<llvh::ArrayRef<uint8_t>> result;
  for (const auto &m : runtimeModuleList_) {
    if (!m.hidesEpilogue()) {
      result.push_back(m.getEpilogue());
    }
  }
  return result;
}

#ifdef HERMES_ENABLE_DEBUGGER

llvh::Optional<Runtime::StackFrameInfo> Runtime::stackFrameInfoByIndex(
    uint32_t frameIdx) const {
  // Locate the frame.
  auto frames = getStackFrames();
  auto it = frames.begin();
  for (; frameIdx && it != frames.end(); ++it, --frameIdx) {
  }
  if (it == frames.end())
    return llvh::None;

  StackFrameInfo info;
  info.frame = *it++;
  info.isGlobal = it == frames.end();
  return info;
}

/// Calculate and \return the offset between the location of the specified
/// frame and the start of the stack. This value increases with every nested
/// call.
uint32_t Runtime::calcFrameOffset(ConstStackFrameIterator it) const {
  assert(it != getStackFrames().end() && "invalid frame");
  return registerStackEnd_ - it->ptr();
}

/// \return the offset between the location of the current frame and the
///   start of the stack. This value increases with every nested call.
uint32_t Runtime::getCurrentFrameOffset() const {
  return calcFrameOffset(getStackFrames().begin());
}

#endif

static ExecutionStatus
raisePlaceholder(Runtime *runtime, Handle<JSError> errorObj, Handle<> message) {
  JSError::recordStackTrace(errorObj, runtime);
  JSError::setupStack(errorObj, runtime);
  JSError::setMessage(errorObj, runtime, message);
  return runtime->setThrownValue(errorObj.getHermesValue());
}

/// A placeholder used to construct a Error Object that takes in a the specified
/// message.
static ExecutionStatus raisePlaceholder(
    Runtime *runtime,
    Handle<JSObject> prototype,
    Handle<> message) {
  GCScopeMarkerRAII gcScope{runtime};

  // Create the error object, initialize stack property and set message.
  auto errorObj = runtime->makeHandle(JSError::create(runtime, prototype));
  return raisePlaceholder(runtime, errorObj, message);
}

/// A placeholder used to construct a Error Object that takes in a const
/// message. A new StringPrimitive is created each time.
// TODO: Predefine each error message.
static ExecutionStatus raisePlaceholder(
    Runtime *runtime,
    Handle<JSObject> prototype,
    const TwineChar16 &msg) {
  // Since this happens unexpectedly and rarely, don't rely on the parent
  // GCScope.
  GCScope gcScope{runtime};

  SmallU16String<64> buf;
  msg.toVector(buf);

  auto strRes = StringPrimitive::create(runtime, buf);
  if (strRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto str = runtime->makeHandle<StringPrimitive>(*strRes);
  LLVM_DEBUG(llvh::errs() << buf.arrayRef() << "\n");
  return raisePlaceholder(runtime, prototype, str);
}

ExecutionStatus Runtime::raiseTypeError(Handle<> message) {
  // Since this happens unexpectedly and rarely, don't rely on the parent
  // GCScope.
  GCScope gcScope{this};
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&TypeErrorPrototype), message);
}

ExecutionStatus Runtime::raiseTypeErrorForValue(
    llvh::StringRef msg1,
    Handle<> value,
    llvh::StringRef msg2) {
  switch (value->getTag()) {
    case ObjectTag:
      return raiseTypeError(msg1 + TwineChar16("Object") + msg2);
    case StrTag:
      return raiseTypeError(
          msg1 + TwineChar16("'") + vmcast<StringPrimitive>(*value) + "'" +
          msg2);
    case BoolTag:
      if (value->getBool()) {
        return raiseTypeError(msg1 + TwineChar16("true") + msg2);
      } else {
        return raiseTypeError(msg1 + TwineChar16("false") + msg2);
      }
    case UndefinedNullTag:
      if (value->isUndefined())
        return raiseTypeError(msg1 + TwineChar16("undefined") + msg2);
      else
        return raiseTypeError(msg1 + TwineChar16("null") + msg2);
    default:
      if (value->isNumber()) {
        char buf[hermes::NUMBER_TO_STRING_BUF_SIZE];
        size_t len = numberToString(
            value->getNumber(), buf, hermes::NUMBER_TO_STRING_BUF_SIZE);
        return raiseTypeError(
            msg1 + TwineChar16(llvh::StringRef{buf, len}) + msg2);
      }
      return raiseTypeError(msg1 + TwineChar16("Value") + msg2);
  }
}

ExecutionStatus Runtime::raiseTypeError(const TwineChar16 &msg) {
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&TypeErrorPrototype), msg);
}

ExecutionStatus Runtime::raiseSyntaxError(const TwineChar16 &msg) {
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&SyntaxErrorPrototype), msg);
}

ExecutionStatus Runtime::raiseRangeError(const TwineChar16 &msg) {
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&RangeErrorPrototype), msg);
}

ExecutionStatus Runtime::raiseReferenceError(const TwineChar16 &msg) {
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&ReferenceErrorPrototype), msg);
}

ExecutionStatus Runtime::raiseURIError(const TwineChar16 &msg) {
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&URIErrorPrototype), msg);
}

ExecutionStatus Runtime::raiseStackOverflow(StackOverflowKind kind) {
  const char *msg;
  switch (kind) {
    case StackOverflowKind::JSRegisterStack:
      msg = "Maximum call stack size exceeded";
      break;
    case StackOverflowKind::NativeStack:
      msg = "Maximum call stack size exceeded (native stack depth)";
      break;
    case StackOverflowKind::JSONParser:
      msg = "Maximum nesting level in JSON parser exceeded";
      break;
    case StackOverflowKind::JSONStringify:
      msg = "Maximum nesting level in JSON stringifyer exceeded";
      break;
  }
  return raisePlaceholder(
      this, Handle<JSObject>::vmcast(&RangeErrorPrototype), msg);
}

ExecutionStatus Runtime::raiseQuitError() {
  return raiseUncatchableError(
      Handle<JSObject>::vmcast(&QuitErrorPrototype), "Quit");
}

ExecutionStatus Runtime::raiseTimeoutError() {
  return raiseUncatchableError(
      Handle<JSObject>::vmcast(&TimeoutErrorPrototype),
      "Javascript execution has timed out.");
}

ExecutionStatus Runtime::raiseUncatchableError(
    Handle<JSObject> prototype,
    llvh::StringRef errMessage) {
  Handle<JSError> err = makeHandle(JSError::createUncatchable(this, prototype));
  auto res = StringPrimitive::create(
      this, llvh::ASCIIRef{errMessage.begin(), errMessage.end()});
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto str = makeHandle(*res);
  return raisePlaceholder(this, err, str);
}

ExecutionStatus Runtime::raiseEvalUnsupported(llvh::StringRef code) {
  return raiseSyntaxError(
      TwineChar16("Parsing source code unsupported: ") + code.substr(0, 32));
}

CallResult<bool> Runtime::insertVisitedObject(Handle<JSObject> obj) {
  bool foundCycle = false;
  MutableHandle<ArrayStorage> stack{
      this, vmcast<ArrayStorage>(stringCycleCheckVisited_)};
  for (uint32_t i = 0, len = stack->size(); i < len; ++i) {
    if (stack->at(i).getObject() == obj.get()) {
      foundCycle = true;
      break;
    }
  }
  if (ArrayStorage::push_back(stack, this, obj) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  stringCycleCheckVisited_ = stack.getHermesValue();
  return foundCycle;
}

void Runtime::removeVisitedObject(Handle<JSObject> obj) {
  (void)obj;
  auto stack = Handle<ArrayStorage>::vmcast(&stringCycleCheckVisited_);
  auto elem = stack->pop_back(this);
  (void)elem;
  assert(
      elem.isObject() && elem.getObject() == obj.get() &&
      "string cycle check: stack corrupted");
}

std::unique_ptr<Buffer> Runtime::generateSpecialRuntimeBytecode() {
  hbc::SimpleBytecodeBuilder builder;
  {
    hbc::BytecodeInstructionGenerator bcGen;
    bcGen.emitLoadConstUndefined(0);
    bcGen.emitRet(0);
    builder.addFunction(1, 1, bcGen.acquireBytecode());
  }
  {
    hbc::BytecodeInstructionGenerator bcGen;
    bcGen.emitGetGlobalObject(0);
    bcGen.emitRet(0);
    builder.addFunction(1, 1, bcGen.acquireBytecode());
  }
  auto buffer = builder.generateBytecodeBuffer();
  assert(buffer->size() < MIN_IO_TRACKING_SIZE);
  return buffer;
}

void Runtime::initPredefinedStrings() {
  assert(!getTopGCScope() && "There shouldn't be any handles allocated yet");

  auto buffer = predefStringAndSymbolChars;
  auto propLengths = predefPropertyLengths;
  auto strLengths = predefStringLengths;
  auto symLengths = predefSymbolLengths;

  static const uint32_t hashes[] = {
#define STR(name, string) constexprHashString(string),
#include "hermes/VM/PredefinedStrings.def"
  };

  uint32_t offset = 0;
  uint32_t registered = 0;
  (void)registered;
  const uint32_t strCount = Predefined::NumStrings;
  const uint32_t symCount = Predefined::NumSymbols;
  identifierTable_.reserve(Predefined::_IPROP_AFTER_LAST + strCount + symCount);

  for (uint32_t idx = 0; idx < Predefined::_IPROP_AFTER_LAST; ++idx) {
    SymbolID sym = identifierTable_.createNotUniquedLazySymbol(
        ASCIIRef{&buffer[offset], propLengths[idx]});

    assert(sym == Predefined::getSymbolID((Predefined::IProp)registered++));
    (void)sym;

    offset += propLengths[idx];
  }

  assert(
      strCount == sizeof hashes / sizeof hashes[0] &&
      "Arrays should have same length");
  for (uint32_t idx = 0; idx < strCount; idx++) {
    SymbolID sym = identifierTable_.registerLazyIdentifier(
        ASCIIRef{&buffer[offset], strLengths[idx]}, hashes[idx]);

    assert(sym == Predefined::getSymbolID((Predefined::Str)registered++));
    (void)sym;

    offset += strLengths[idx];
  }

  for (uint32_t idx = 0; idx < symCount; ++idx) {
    SymbolID sym = identifierTable_.createNotUniquedLazySymbol(
        ASCIIRef{&buffer[offset], symLengths[idx]});

    assert(sym == Predefined::getSymbolID((Predefined::Sym)registered++));
    (void)sym;

    offset += symLengths[idx];
  }

  assert(
      !getTopGCScope() &&
      "There shouldn't be any handles allocated during initializing the predefined strings");
}

void Runtime::initCharacterStrings() {
  GCScope gc(this);
  auto marker = gc.createMarker();
  charStrings_.reserve(256);
  for (char16_t ch = 0; ch < 256; ++ch) {
    gc.flushToMarker(marker);
    charStrings_.push_back(allocateCharacterString(ch).getHermesValue());
  }
}

Handle<StringPrimitive> Runtime::allocateCharacterString(char16_t ch) {
  // This can in theory throw when called out of initialization phase.
  // However there is only that many character strings and in practice this
  // is not a problem.  Note that we allocate these as "long-lived" objects,
  // so we don't have to scan the charStrings_ array in
  // young-generation collections.

  PinnedHermesValue strRes;
  if (LLVM_LIKELY(ch < 128)) {
    strRes = ignoreAllocationFailure(
        StringPrimitive::createLongLived(this, ASCIIRef(ch)));
  } else {
    strRes = ignoreAllocationFailure(
        StringPrimitive::createLongLived(this, UTF16Ref(ch)));
  }
  return makeHandle<StringPrimitive>(strRes);
}

Handle<StringPrimitive> Runtime::getCharacterString(char16_t ch) {
  if (LLVM_LIKELY(ch < 256))
    return Handle<StringPrimitive>::vmcast(&charStrings_[ch]);

  return makeHandle<StringPrimitive>(
      ignoreAllocationFailure(StringPrimitive::create(this, UTF16Ref(ch))));
}

// Store all object and symbol ids in a static table to conserve code size.
static const struct {
  uint16_t object, method;
#ifndef NDEBUG
  const char *name;
#define BUILTIN_METHOD(object, method) \
  {(uint16_t)Predefined::object,       \
   (uint16_t)Predefined::method,       \
   #object "::" #method},
#else
#define BUILTIN_METHOD(object, method) \
  {(uint16_t)Predefined::object, (uint16_t)Predefined::method},
#endif
#define PRIVATE_BUILTIN(name)
#define JS_BUILTIN(name)
} publicNativeBuiltins[] = {
#include "hermes/FrontEndDefs/Builtins.def"
};

static_assert(
    sizeof(publicNativeBuiltins) / sizeof(publicNativeBuiltins[0]) ==
        BuiltinMethod::_firstPrivate,
    "builtin method table mismatch");

ExecutionStatus Runtime::forEachPublicNativeBuiltin(
    const std::function<ExecutionStatus(
        unsigned methodIndex,
        Predefined::Str objectName,
        Handle<JSObject> &object,
        SymbolID methodID)> &callback) {
  MutableHandle<JSObject> lastObject{this};
  Predefined::Str lastObjectName = Predefined::_STRING_AFTER_LAST;

  for (unsigned methodIndex = 0; methodIndex < BuiltinMethod::_firstPrivate;
       ++methodIndex) {
    GCScopeMarkerRAII marker{this};
    LLVM_DEBUG(llvh::dbgs() << publicNativeBuiltins[methodIndex].name << "\n");
    // Find the object first, if it changed.
    auto objectName = (Predefined::Str)publicNativeBuiltins[methodIndex].object;
    if (objectName != lastObjectName) {
      auto objectID = Predefined::getSymbolID(objectName);
      auto cr = JSObject::getNamed_RJS(getGlobal(), this, objectID);
      assert(
          cr.getStatus() != ExecutionStatus::EXCEPTION &&
          "getNamed() of builtin object failed");
      assert(
          vmisa<JSObject>(cr->get()) &&
          "getNamed() of builtin object must be an object");

      lastObject = vmcast<JSObject>(cr->get());
      lastObjectName = objectName;
    }

    // Find the method.
    auto methodName = (Predefined::Str)publicNativeBuiltins[methodIndex].method;
    auto methodID = Predefined::getSymbolID(methodName);

    ExecutionStatus status =
        callback(methodIndex, objectName, lastObject, methodID);
    if (status != ExecutionStatus::RETURNED) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return ExecutionStatus::RETURNED;
}

void Runtime::initNativeBuiltins() {
  GCScopeMarkerRAII gcScope{this};

  builtins_.resize(BuiltinMethod::_count);

  (void)forEachPublicNativeBuiltin([this](
                                       unsigned methodIndex,
                                       Predefined::Str /* objectName */,
                                       Handle<JSObject> &currentObject,
                                       SymbolID methodID) {
    auto cr = JSObject::getNamed_RJS(currentObject, this, methodID);
    assert(
        cr.getStatus() != ExecutionStatus::EXCEPTION &&
        "getNamed() of builtin method failed");
    assert(
        vmisa<NativeFunction>(cr->get()) &&
        "getNamed() of builtin method must be a NativeFunction");
    builtins_[methodIndex] = vmcast<NativeFunction>(cr->get());
    return ExecutionStatus::RETURNED;
  });

  // Now add the private native builtins.
  createHermesBuiltins(this, builtins_);
#ifndef NDEBUG
  // Make sure native builtins are all defined.
  for (unsigned i = 0; i < BuiltinMethod::_firstJS; ++i) {
    assert(builtins_[i] && "native builtin not initialized");
  }
#endif
}

static const struct JSBuiltin {
  uint16_t symID, builtinIndex;
} jsBuiltins[] = {
#define BUILTIN_METHOD(object, method)
#define PRIVATE_BUILTIN(name)
#define JS_BUILTIN(name)                                \
  {                                                     \
    (uint16_t) Predefined::name,                        \
        (uint16_t)BuiltinMethod::HermesBuiltin##_##name \
  }
#include "hermes/FrontEndDefs/Builtins.def"
};

void Runtime::initJSBuiltins(
    llvh::MutableArrayRef<Callable *> builtins,
    Handle<JSObject> jsBuiltinsObj) {
  for (const JSBuiltin &jsBuiltin : jsBuiltins) {
    auto symID = jsBuiltin.symID;
    auto builtinIndex = jsBuiltin.builtinIndex;

    // Try to get the JS function from jsBuiltinsObj.
    auto getRes = JSObject::getNamed_RJS(
        jsBuiltinsObj, this, Predefined::getSymbolID((Predefined::Str)symID));
    assert(getRes == ExecutionStatus::RETURNED && "Failed to get JS builtin.");
    JSFunction *jsFunc = vmcast<JSFunction>(getRes->getHermesValue());

    builtins[builtinIndex] = jsFunc;
  }
}

ExecutionStatus Runtime::assertBuiltinsUnmodified() {
  assert(!builtinsFrozen_ && "Builtins are already frozen.");
  GCScope gcScope(this);

  return forEachPublicNativeBuiltin([this](
                                        unsigned methodIndex,
                                        Predefined::Str /* objectName */,
                                        Handle<JSObject> &currentObject,
                                        SymbolID methodID) {
    auto cr = JSObject::getNamed_RJS(currentObject, this, methodID);
    assert(
        cr.getStatus() != ExecutionStatus::EXCEPTION &&
        "getNamed() of builtin method failed");
    // Check if the builtin is overridden.
    auto currentBuiltin = dyn_vmcast<NativeFunction>(std::move(cr->get()));
    if (!currentBuiltin || currentBuiltin != builtins_[methodIndex]) {
      return raiseTypeError(
          "Cannot execute a bytecode compiled with -fstatic-builtins when builtin functions are overriden.");
    }
    return ExecutionStatus::RETURNED;
  });
}

void Runtime::freezeBuiltins() {
  assert(!builtinsFrozen_ && "Builtins are already frozen.");
  GCScope gcScope{this};

  // A list storing all the object ids that we will freeze on the global object
  // in the end.
  std::vector<SymbolID> objectList;
  // A list storing all the method ids on the same object that we will freeze on
  // each object.
  std::vector<SymbolID> methodList;

  // Masks for setting the property a static builtin and read-only.
  PropertyFlags clearFlags;
  clearFlags.configurable = 1;
  clearFlags.writable = 1;
  PropertyFlags setFlags;
  setFlags.staticBuiltin = 1;

  (void)forEachPublicNativeBuiltin(
      [this, &objectList, &methodList, &clearFlags, &setFlags](
          unsigned methodIndex,
          Predefined::Str objectName,
          Handle<JSObject> &currentObject,
          SymbolID methodID) {
        methodList.push_back(methodID);
        // This is the last method on current object.
        if (methodIndex + 1 == BuiltinMethod::_publicCount ||
            objectName != publicNativeBuiltins[methodIndex + 1].object) {
          // Store the object id in the object set.
          SymbolID objectID = Predefined::getSymbolID(objectName);
          objectList.push_back(objectID);
          // Freeze all methods and mark them as static builtins on the current
          // object.
          JSObject::updatePropertyFlagsWithoutTransitions(
              currentObject,
              this,
              clearFlags,
              setFlags,
              llvh::ArrayRef<SymbolID>(methodList));
          methodList.clear();
        }
        return ExecutionStatus::RETURNED;
      });

  // Freeze all builtin objects and mark them as static builtins on the global
  // object.
  JSObject::updatePropertyFlagsWithoutTransitions(
      getGlobal(),
      this,
      clearFlags,
      setFlags,
      llvh::ArrayRef<SymbolID>(objectList));

  builtinsFrozen_ = true;
}

ExecutionStatus Runtime::drainJobs() {
  GCScope gcScope{this};
  MutableHandle<Callable> job{this};
  // Note that new jobs can be enqueued during the draining.
  while (!jobQueue_.empty()) {
    GCScopeMarkerRAII marker{gcScope};

    job = jobQueue_.front();
    jobQueue_.pop_front();

    // Jobs are guaranteed to behave as thunks.
    auto callRes =
        Callable::executeCall0(job, this, Runtime::getUndefinedValue());

    // Early return to signal the caller. Note that the exceptional job has been
    // popped, so re-invocation would pick up from the next available job.
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return ExecutionStatus::RETURNED;
}

uint64_t Runtime::gcStableHashHermesValue(Handle<HermesValue> value) {
  switch (value->getTag()) {
    case ObjectTag: {
      // For objects, because pointers can move, we need a unique ID
      // that does not change for each object.
      auto id = JSObject::getObjectID(vmcast<JSObject>(*value), this);
      return llvh::hash_value(id);
    }
    case StrTag: {
      // For strings, we hash the string content.
      auto strView = StringPrimitive::createStringView(
          this, Handle<StringPrimitive>::vmcast(value));
      return llvh::hash_combine_range(strView.begin(), strView.end());
    }
    default:
      assert(!value->isPointer() && "Unhandled pointer type");
      if (value->isNumber() && value->getNumber() == 0) {
        // To normalize -0 to 0.
        return 0;
      } else {
        // For everything else, we just take advantage of HermesValue.
        return llvh::hash_value(value->getRaw());
      }
  }
}

bool Runtime::symbolEqualsToStringPrim(SymbolID id, StringPrimitive *strPrim) {
  auto view = identifierTable_.getStringView(this, id);
  return strPrim->equals(view);
}

LLVM_ATTRIBUTE_NOINLINE
void Runtime::allocStack(uint32_t count, HermesValue initValue) {
  // Note: it is important that allocStack be defined out-of-line. If inline,
  // constants are propagated into initValue, which enables clang to use
  // memset_pattern_16. This ends up being a significant loss as it is an
  // indirect call.
  allocUninitializedStack(count);
  // Initialize the new registers.
  std::uninitialized_fill_n(stackPointer_, count, initValue);
}

void Runtime::dumpCallFrames(llvh::raw_ostream &OS) {
  OS << "== Call Frames ==\n";
  const PinnedHermesValue *next = getStackPointer();
  unsigned i = 0;
  for (StackFramePtr sf : getStackFrames()) {
    OS << i++ << " ";
    if (auto *closure = sf.getCalleeClosure()) {
      OS << cellKindStr(closure->getKind()) << " ";
    }
    if (auto *cb = sf.getCalleeCodeBlock()) {
      OS << formatSymbolID(cb->getNameMayAllocate()) << " ";
    }
    dumpStackFrame(sf, OS, next);
    next = sf.ptr();
  }
}

LLVM_ATTRIBUTE_NOINLINE
void Runtime::dumpCallFrames() {
  dumpCallFrames(llvh::errs());
}

StackRuntime::StackRuntime(const RuntimeConfig &config)
    : StackRuntime(StorageProvider::mmapProvider(), config) {}

StackRuntime::StackRuntime(
    std::shared_ptr<StorageProvider> provider,
    const RuntimeConfig &config)
    : Runtime(std::move(provider), config) {}

StackRuntime::~StackRuntime() {}

/// Serialize a SymbolID.
llvh::raw_ostream &operator<<(
    llvh::raw_ostream &OS,
    Runtime::FormatSymbolID format) {
  if (!format.symbolID.isValid())
    return OS << "SymbolID(INVALID)";

  OS << "SymbolID("
     << (format.symbolID.isNotUniqued() ? "(External)" : "(Internal)")
     << format.symbolID.unsafeGetIndex() << " \"";

  OS << format.runtime->getIdentifierTable().convertSymbolToUTF8(
      format.symbolID);
  return OS << "\")";
}

template <typename T>
static std::string &llvmStreamableToString(const T &v) {
  // Use a static string to back this function to avoid allocations. We should
  // only be calling this from the crash dumper so not have to worry about
  // multi-threaded usage.
  static std::string buf;
  buf.clear();
  llvh::raw_string_ostream strstrm(buf);
  strstrm << v;
  strstrm.flush();
  return buf;
}

/****************************************************************************
 * WARNING: This code is run after a crash. Avoid walking data structures,
 *          doing memory allocation, or using libc etc. as much as possible
 ****************************************************************************/
void Runtime::crashCallback(int fd) {
  llvh::raw_fd_ostream jsonStream(fd, false);
  JSONEmitter json(jsonStream);
  json.openDict();
  json.emitKeyValue("type", "runtime");
  json.emitKeyValue(
      "address", llvmStreamableToString(llvh::format_hex((uintptr_t)this, 10)));
  json.emitKeyValue(
      "registerStack",
      llvmStreamableToString(llvh::format_hex((uintptr_t)registerStack_, 10)));
  json.emitKeyValue(
      "registerStackPointer",
      llvmStreamableToString(llvh::format_hex((uintptr_t)stackPointer_, 10)));
  json.emitKeyValue(
      "registerStackEnd",
      llvmStreamableToString(
          llvh::format_hex((uintptr_t)registerStackEnd_, 10)));
  json.emitKey("callstack");
  crashWriteCallStack(json);
  json.closeDict();
}

/****************************************************************************
 * WARNING: This code is run after a crash. Avoid walking data structures,
 *          doing memory allocation, or using libc etc. as much as possible
 ****************************************************************************/
void Runtime::crashWriteCallStack(JSONEmitter &json) {
  json.openArray();
  for (auto frame : getStackFrames()) {
    json.openDict();
    json.emitKeyValue(
        "StackFrameRegOffs", (uint32_t)(registerStackEnd_ - frame.ptr()));
    auto codeBlock = frame.getSavedCodeBlock();
    if (codeBlock) {
      json.emitKeyValue("FunctionID", codeBlock->getFunctionID());
      auto bytecodeOffs = codeBlock->getOffsetOf(frame.getSavedIP());
      json.emitKeyValue("ByteCodeOffset", bytecodeOffs);
      auto blockSourceCode = codeBlock->getDebugSourceLocationsOffset();
      const RuntimeModule *runtimeModule = codeBlock->getRuntimeModule();
      if (blockSourceCode.hasValue()) {
        auto debugInfo = runtimeModule->getBytecode()->getDebugInfo();
        auto sourceLocation = debugInfo->getLocationForAddress(
            blockSourceCode.getValue(), bytecodeOffs);
        if (sourceLocation) {
          auto file = debugInfo->getFilenameByID(sourceLocation->filenameId);
          llvh::SmallString<256> srcLocStorage;
          json.emitKeyValue(
              "SourceLocation",
              (llvh::Twine(file) + llvh::Twine(":") +
               llvh::Twine(sourceLocation->line) + llvh::Twine(":") +
               llvh::Twine(sourceLocation->column))
                  .toStringRef(srcLocStorage));
        }
      }
      uint32_t segmentID = runtimeModule->getBytecode()->getSegmentID();
      llvh::StringRef sourceURL = runtimeModule->getSourceURL();
      json.emitKeyValue("SegmentID", segmentID);
      json.emitKeyValue("SourceURL", sourceURL);
    } else {
      json.emitKeyValue("NativeCode", true);
    }
    json.closeDict(); // frame
  }
  json.closeArray(); // frames
}

std::string Runtime::getCallStackNoAlloc(const Inst *ip) {
  NoAllocScope noAlloc(this);
  std::string res;
  // Note that the order of iteration is youngest (leaf) frame to oldest.
  for (auto frame : getStackFrames()) {
    auto codeBlock = frame->getCalleeCodeBlock();
    if (codeBlock) {
      res += codeBlock->getNameString(this);
      // Default to the function entrypoint, this
      // ensures source location is provided for leaf frame even
      // if ip is not available.
      const uint32_t bytecodeOffs =
          ip != nullptr ? codeBlock->getOffsetOf(ip) : 0;
      auto blockSourceCode = codeBlock->getDebugSourceLocationsOffset();
      if (blockSourceCode.hasValue()) {
        auto debugInfo =
            codeBlock->getRuntimeModule()->getBytecode()->getDebugInfo();
        auto sourceLocation = debugInfo->getLocationForAddress(
            blockSourceCode.getValue(), bytecodeOffs);
        if (sourceLocation) {
          auto file = debugInfo->getFilenameByID(sourceLocation->filenameId);
          res += ": " + file + ":" + oscompat::to_string(sourceLocation->line) +
              ":" + oscompat::to_string(sourceLocation->column);
        }
      }
      res += "\n";
    } else {
      res += "<Native code>\n";
    }
    // Get the ip of the caller frame -- which will then be correct for the
    // next iteration.
    ip = frame.getSavedIP();
  }
  return res;
}

void Runtime::onGCEvent(GCEventKind kind, const std::string &extraInfo) {
  if (samplingProfiler != nullptr) {
    samplingProfiler->onGCEvent(kind, extraInfo);
  }
  if (gcEventCallback_) {
    gcEventCallback_(kind, extraInfo.c_str());
  }
}

#ifdef HERMESVM_PROFILER_BB

llvh::Optional<std::tuple<std::string, uint32_t, uint32_t>>
Runtime::getIPSourceLocation(const CodeBlock *codeBlock, const Inst *ip) {
  auto bytecodeOffs = codeBlock->getOffsetOf(ip);
  auto blockSourceCode = codeBlock->getDebugSourceLocationsOffset();

  if (!blockSourceCode) {
    return llvh::None;
  }
  auto debugInfo = codeBlock->getRuntimeModule()->getBytecode()->getDebugInfo();
  auto sourceLocation = debugInfo->getLocationForAddress(
      blockSourceCode.getValue(), bytecodeOffs);
  if (!sourceLocation) {
    return llvh::None;
  }
  auto filename = debugInfo->getFilenameByID(sourceLocation->filenameId);
  auto line = sourceLocation->line;
  auto col = sourceLocation->column;
  return std::make_tuple(filename, line, col);
}

void Runtime::preventHCGC(HiddenClass *hc) {
  auto &classIdToIdxMap = inlineCacheProfiler_.getClassIdtoIndexMap();
  auto &hcIdx = inlineCacheProfiler_.getHiddenClassArrayIndex();
  auto ret = classIdToIdxMap.insert(
      std::pair<ClassId, uint32_t>(getHeap().getObjectID(hc), hcIdx));
  if (ret.second) {
    auto *hiddenClassArray = inlineCacheProfiler_.getHiddenClassArray();
    JSArray::setElementAt(
        makeHandle(hiddenClassArray), this, hcIdx++, makeHandle(hc));
  }
}

void Runtime::recordHiddenClass(
    CodeBlock *codeBlock,
    const Inst *cacheMissInst,
    SymbolID symbolID,
    HiddenClass *objectHiddenClass,
    HiddenClass *cachedHiddenClass) {
  auto offset = codeBlock->getOffsetOf(cacheMissInst);

  // inline caching hit
  if (objectHiddenClass == cachedHiddenClass) {
    inlineCacheProfiler_.insertICHit(codeBlock, offset);
    return;
  }

  // inline caching miss
  assert(objectHiddenClass != nullptr && "object hidden class should exist");
  // prevent object hidden class from being GC-ed
  preventHCGC(objectHiddenClass);
  ClassId objectHiddenClassId = getHeap().getObjectID(objectHiddenClass);
  // prevent cached hidden class from being GC-ed
  ClassId cachedHiddenClassId =
      static_cast<ClassId>(GCBase::IDTracker::kInvalidNode);
  if (cachedHiddenClass != nullptr) {
    preventHCGC(cachedHiddenClass);
    cachedHiddenClassId = getHeap().getObjectID(cachedHiddenClass);
  }
  // add the record to inline caching profiler
  inlineCacheProfiler_.insertICMiss(
      codeBlock, offset, symbolID, objectHiddenClassId, cachedHiddenClassId);
}

void Runtime::getInlineCacheProfilerInfo(llvh::raw_ostream &ostream) {
  inlineCacheProfiler_.dumpRankedInlineCachingMisses(this, ostream);
}

HiddenClass *Runtime::resolveHiddenClassId(ClassId classId) {
  if (classId == static_cast<ClassId>(GCBase::IDTracker::kInvalidNode)) {
    return nullptr;
  }
  auto &classIdToIdxMap = inlineCacheProfiler_.getClassIdtoIndexMap();
  auto *hiddenClassArray = inlineCacheProfiler_.getHiddenClassArray();
  auto hcHermesVal = hiddenClassArray->at(this, classIdToIdxMap[classId]);
  return vmcast<HiddenClass>(hcHermesVal);
}

#endif

#ifdef HERMESVM_SERIALIZE
void Runtime::serialize(Serializer &s) {
  // Full GC here.
  getHeap().collect("serialize");

  s.writeCurrentOffset();
  getHeap().serializeWeakRefs(s);

  s.writeCurrentOffset();
  serializeIdentifierTable(s);

  s.writeCurrentOffset();
  symbolRegistry_.serialize(s);

  s.writeCurrentOffset();
  serializeRuntimeFields(s);

  s.writeCurrentOffset();
  getHeap().serializeHeap(s);

  s.writeCurrentOffset();
  getHeap().getIDTracker().serialize(s);

  s.writeCurrentOffset();
  getHeap().getAllocationLocationTracker().serialize(s);

  // In the end record the size of the object table and flush the string
  // buffers, so the deserializer can read it. TODO: perhaps seek to the
  // beginning and record there.
  s.writeCurrentOffset();
  s.writeEpilogue();
}

void Runtime::serializeIdentifierTable(Serializer &s) {
  identifierTable_.serialize(s);
}

void Runtime::serializeRuntimeFields(Serializer &s) {
// Serialize all HermesValue
#define RUNTIME_HV_FIELD(name) s.writeHermesValue(name);
#define RUNTIME_HV_FIELD_PROTOTYPE(name) RUNTIME_HV_FIELD(name)
#define RUNTIME_HV_FIELD_INSTANCE(name) RUNTIME_HV_FIELD(name)
#define RUNTIME_HV_FIELD_RUNTIMEMODULE(name) RUNTIME_HV_FIELD(name)
#include "hermes/VM/RuntimeHermesValueFields.def"
#undef RUNTIME_HV_FIELD
  // stringCycleCheckVisited_ owns an ArrayStorage. This is managed via a RAII
  // so it will never be cleared when deserialized if we serialize it. We will
  // not serialize the contents of this storage but only do relocation for the
  // ArrayStorage *.
  bool hasArray = (bool)vmcast<ArrayStorage>(stringCycleCheckVisited_);
  s.writeInt<uint8_t>(hasArray);
  if (hasArray) {
    s.endObject(vmcast<ArrayStorage>(stringCycleCheckVisited_));
  }

  // Do not Serialize any raw pointers of HermesValue fields. Get those pointers
  // after relocation finishes.

  // RegExpMatch regExpLastMatch{};
  // Ignore for now since we only serialize/deserialize after global object
  // initialization.
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.

  // TODO: ignore all fields from runtimeConfig, about frames, stacks etc.
  // come back later to check again from heap_ to nativeCallFrameDepth_.

  s.writeRelocation(emptyCodeBlock_);
  s.writeRelocation(returnThisCodeBlock_);

  /// Field specialCodeBlockRuntimeModule_;
  /// Field RuntimeModuleList runtimeModuleList_{}. We don't S/D the list of
  /// Runtimemodules now because Runtime doesn't own them, Domain owns them, so
  /// they will be Serialized/Deserialized with Domain. When new RuntimeModules
  /// are deserialized, they will add themselves to this list.
  s.writeRelocation(specialCodeBlockRuntimeModule_);

  // Field rootClazzes_[];
  for (const auto &clazz : rootClazzes_)
    s.writeHermesValue(clazz);

  // Field PropertyCacheEntry fixedPropCache_[(size_t)PropCacheID::_COUNT];
  // Ignore for now.
  // TODO: come back later.

  // Field std::vector<PinnedHermesValue> charStrings_{};
  s.writeInt<uint32_t>(charStrings_.size());
  for (auto &str : charStrings_) {
    s.writeHermesValue(str);
  }

  // Field std::vector<NativeFunction *> builtins_{};
  s.writeInt<uint32_t>(builtins_.size());
  for (auto *nativeFunction : builtins_) {
    s.writeRelocation(nativeFunction);
  }

  s.writeInt<uint8_t>(builtinsFrozen_);

  // TODO: Come back later. serialize/deserialize this to be able to
  // serialize/deserialize after user code.
  // Field const CrashManager::CallbackKey crashCallbackKey_;
  // Field std::shared_ptr<SamplingProfiler> samplingProfiler;

#ifdef HERMES_ENABLE_DEBUGGER
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.
  // Field Debugger debugger_{this};
  // Field std::atomic<uint8_t> debuggerRequestedFlag_{0};
#endif
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.
  // Field std::vector<std::shared_ptr<hbc::BCProvider>> persistentBCProviders_;

#ifdef HERMES_ENABLE_DEBUGGER
  // Field const inst::Inst *savedIP_{nullptr};
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.
#else
#endif // HERMES_ENABLE_DEBUGGER
}

void Runtime::deserializeRuntimeFields(Deserializer &d) {
// Deserialize all HermesValue
#define RUNTIME_HV_FIELD(name) d.readHermesValue(&name);
#define RUNTIME_HV_FIELD_PROTOTYPE(name) RUNTIME_HV_FIELD(name)
#define RUNTIME_HV_FIELD_INSTANCE(name) RUNTIME_HV_FIELD(name)
#define RUNTIME_HV_FIELD_RUNTIMEMODULE(name) RUNTIME_HV_FIELD(name)
#include "hermes/VM/RuntimeHermesValueFields.def"
#undef RUNTIME_HV_FIELD
  // stringCycleCheckVisited_ owns an ArrayStorage. It is managed via a RAII so
  // we don't serialize the contents of the storage. Create an empty
  // ArrayStorage here if needed to handle relocation.
  if (d.readInt<uint8_t>()) {
    auto arrRes = this->ignoreAllocationFailure(ArrayStorage::create(this, 0));
    d.endObject(vmcast<ArrayStorage>(arrRes));
  }

  // Do not Deserialize any raw pointers of HermesValue fields. Get those
  // pointers after relocation finishes.

  // Field RegExpMatch regExpLastMatch{};
  // Ignore for now since we only serialize/deserialize after global object
  // initialization.
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.

  // TODO: Ignore all fields from runtimeConfig, about frames, stacks etc for
  // now. Come back later to check again from heap_ to nativeCallFrameDepth_.

  d.readRelocation(&emptyCodeBlock_, RelocationKind::NativePointer);
  d.readRelocation(&returnThisCodeBlock_, RelocationKind::NativePointer);

  /// Field specialCodeBlockRuntimeModule_;
  /// Field RuntimeModuleList runtimeModuleList_{}. We don't S/D the list of
  /// Runtimemodules now because Runtime doesn't own them, Domain owns them, so
  /// they will be Serialized/Deserialized with Domain. When new RuntimeModules
  /// are deserialized, they will add themselves to this list.
  d.readRelocation(
      &specialCodeBlockRuntimeModule_, RelocationKind::NativePointer);

  // Field rootClazzes_[];
  for (auto &clazz : rootClazzes_)
    d.readHermesValue(&clazz);

  // Field PropertyCacheEntry fixedPropCache_[(size_t)PropCacheID::_COUNT];
  // Ignore for now.
  // TODO: come back later.

  // Field std::vector<PinnedHermesValue> charStrings_{};
  size_t size = d.readInt<uint32_t>();
  charStrings_.resize(size);
  for (auto &str : charStrings_) {
    d.readHermesValue(&str);
  }

  // Field std::vector<NativeFunction *> builtins_{};
  size = d.readInt<uint32_t>();
  builtins_.resize(size);
  for (auto *&nativeFunction : builtins_) {
    d.readRelocation(&nativeFunction, RelocationKind::NativePointer);
  }

  builtinsFrozen_ = d.readInt<uint8_t>();

  // TODO: Come back later. Serialize/deserialize this to be able to
  // serialize/deserialize after user code.
  // Field const CrashManager::CallbackKey crashCallbackKey_;
  // Field std::shared_ptr<SamplingProfiler> samplingProfiler;

#ifdef HERMES_ENABLE_DEBUGGER
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.
  // Field Debugger debugger_{this}; Field std::atomic<uint8_t>
  // debuggerRequestedFlag_{0};
#endif
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.
  // Field std::vector<std::shared_ptr<hbc::BCProvider>> persistentBCProviders_;

#ifdef HERMES_ENABLE_DEBUGGER
  // Field const inst::Inst *savedIP_{nullptr};
  // TODO: serialize/deserialize this to be able to serialize/deserialize after
  // user code.
#else
#endif // HERMES_ENABLE_DEBUGGER
}

void Runtime::deserializeImpl(Deserializer &d, bool currentlyInYoung) {
  if (currentlyInYoung) {
    getHeap().deserializeStart();
  }

  GCScope scope(this);

  d.readAndCheckOffset();
  getHeap().deserializeWeakRefs(d);

  d.readAndCheckOffset();
  identifierTable_.deserialize(d);

  d.readAndCheckOffset();
  symbolRegistry_.deserialize(d);

  d.readAndCheckOffset();
  deserializeRuntimeFields(d);

  d.readAndCheckOffset();
  getHeap().deserializeHeap(d);

  d.readAndCheckOffset();
  getHeap().getIDTracker().deserialize(d);

  d.readAndCheckOffset();
  getHeap().getAllocationLocationTracker().deserialize(d);

  d.readAndCheckOffset();
  d.flushRelocationQueue();

  // Now update the runtime pointer to prototypes.
  // JSObject *objectPrototypeRawPtr{};
  objectPrototypeRawPtr = vmcast<JSObject>(objectPrototype);
  // JSObject *functionPrototypeRawPtr{};
  functionPrototypeRawPtr = vmcast<NativeFunction>(functionPrototype);

  LLVM_DEBUG(llvh::dbgs() << "Finish deserializing\n");

  if (currentlyInYoung) {
    // Only switch back now if the config says allocInYoung. Otherwise
    // wait until tti.
    getHeap().deserializeEnd();
  }
}

void Runtime::populateHeaderRuntimeConfig(SerializeHeader &header) {
  header.enableEval = enableEval;
  header.hasES6Promise = hasES6Promise_;
  header.hasES6Proxy = hasES6Proxy_;
  header.hasIntl = hasIntl_;
  header.bytecodeWarmupPercent = bytecodeWarmupPercent_;
  header.trackIO = trackIO_;
}

void Runtime::checkHeaderRuntimeConfig(SerializeHeader &header) const {
  if (header.enableEval != enableEval) {
    hermes_fatal(
        "serialize/deserialize Runtime Configs don't match (enableEval)");
  }
  if (header.hasES6Promise != hasES6Promise_) {
    hermes_fatal(
        "serialize/deserialize Runtime Configs don't match (es6Promise)");
  }
  if (header.hasES6Proxy != hasES6Proxy_) {
    hermes_fatal(
        "serialize/deserialize Runtime Configs don't match (es6Proxy)");
  }
  if (header.hasIntl != hasIntl_) {
    hermes_fatal("serialize/deserialize Runtime Configs don't match (intl)");
  }

  if (header.bytecodeWarmupPercent != bytecodeWarmupPercent_) {
    hermes_fatal(
        "serialize/deserialize Runtime Configs don't match (bytecodeWarmupPercent)");
  }
  if (header.trackIO != trackIO_) {
    hermes_fatal("serialize/deserialize Runtime Configs don't match (trackIO)");
  }
}
#endif

ExecutionStatus Runtime::notifyTimeout() {
  // TODO: allow a vector of callbacks.
  return raiseTimeoutError();
}

#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES

std::pair<const CodeBlock *, const inst::Inst *>
Runtime::getCurrentInterpreterLocation(const inst::Inst *ip) const {
  assert(ip && "IP being null implies we're not currently in the interpreter.");
  auto callFrames = getStackFrames();
  const CodeBlock *codeBlock = nullptr;
  for (auto frameIt = callFrames.begin(); frameIt != callFrames.end();
       ++frameIt) {
    codeBlock = frameIt->getCalleeCodeBlock();
    if (codeBlock) {
      break;
    } else {
      ip = frameIt->getSavedIP();
    }
  }
  assert(codeBlock && "Could not find CodeBlock.");
  return {codeBlock, ip};
}

StackTracesTreeNode *Runtime::getCurrentStackTracesTreeNode(
    const inst::Inst *ip) {
  assert(stackTracesTree_ && "Runtime not configured to track alloc stacks");
  assert(
      (getHeap().getAllocationLocationTracker().isEnabled() ||
       getHeap().getSamplingAllocationTracker().isEnabled()) &&
      "AllocationLocationTracker not enabled");
  if (!ip) {
    return nullptr;
  }
  const CodeBlock *codeBlock;
  std::tie(codeBlock, ip) = getCurrentInterpreterLocation(ip);
  return stackTracesTree_->getStackTrace(this, codeBlock, ip);
}

void Runtime::enableAllocationLocationTracker(
    std::function<void(
        uint64_t,
        std::chrono::microseconds,
        std::vector<GCBase::AllocationLocationTracker::HeapStatsUpdate>)>
        fragmentCallback) {
  if (!stackTracesTree_) {
    stackTracesTree_ = std::make_unique<StackTracesTree>();
  }
  stackTracesTree_->syncWithRuntimeStack(this);
  getHeap().enableHeapProfiler(std::move(fragmentCallback));
}

void Runtime::disableAllocationLocationTracker(bool clearExistingTree) {
  getHeap().disableHeapProfiler();
  if (clearExistingTree) {
    stackTracesTree_.reset();
  }
}

void Runtime::enableSamplingHeapProfiler(
    size_t samplingInterval,
    int64_t seed) {
  if (!stackTracesTree_) {
    stackTracesTree_ = std::make_unique<StackTracesTree>();
  }
  stackTracesTree_->syncWithRuntimeStack(this);
  getHeap().enableSamplingHeapProfiler(samplingInterval, seed);
}

void Runtime::disableSamplingHeapProfiler(llvh::raw_ostream &os) {
  getHeap().disableSamplingHeapProfiler(os);
  stackTracesTree_.reset();
}

void Runtime::popCallStackImpl() {
  assert(stackTracesTree_ && "Runtime not configured to track alloc stacks");
  stackTracesTree_->popCallStack();
}

void Runtime::pushCallStackImpl(
    const CodeBlock *codeBlock,
    const inst::Inst *ip) {
  assert(stackTracesTree_ && "Runtime not configured to track alloc stacks");
  stackTracesTree_->pushCallStack(this, codeBlock, ip);
}

#else // !defined(HERMES_ENABLE_ALLOCATION_LOCATION_TRACES)

std::pair<const CodeBlock *, const inst::Inst *>
Runtime::getCurrentInterpreterLocation(const inst::Inst *ip) const {
  return {nullptr, nullptr};
}

StackTracesTreeNode *Runtime::getCurrentStackTracesTreeNode(
    const inst::Inst *ip) {
  return nullptr;
}

void Runtime::enableAllocationLocationTracker(
    std::function<void(
        uint64_t,
        std::chrono::microseconds,
        std::vector<GCBase::AllocationLocationTracker::HeapStatsUpdate>)>) {}

void Runtime::disableAllocationLocationTracker(bool) {}

void Runtime::enableSamplingHeapProfiler(size_t, int64_t) {}

void Runtime::disableSamplingHeapProfiler(llvh::raw_ostream &) {}

void Runtime::popCallStackImpl() {}

void Runtime::pushCallStackImpl(const CodeBlock *, const inst::Inst *) {}

#endif // !defined(HERMES_ENABLE_ALLOCATION_LOCATION_TRACES)

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
