/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "gc"
#include "hermes/VM/GC.h"

#include "hermes/Platform/Logging.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/SlotAcceptorDefault.h"
#include "hermes/VM/VTable.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/Format.h"
#include "llvh/Support/NativeFormatting.h"
#include "llvh/Support/raw_ostream.h"

#include <inttypes.h>
#include <stdexcept>
#include <system_error>

using llvh::dbgs;
using llvh::format;

namespace hermes {
namespace vm {

GCBase::GCBase(
    MetadataTable metaTable,
    GCCallbacks *gcCallbacks,
    PointerBase *pointerBase,
    const GCConfig &gcConfig,
    std::shared_ptr<CrashManager> crashMgr)
    : metaTable_(metaTable),
      gcCallbacks_(gcCallbacks),
      pointerBase_(pointerBase),
      crashMgr_(crashMgr),
      analyticsCallback_(gcConfig.getAnalyticsCallback()),
      recordGcStats_(gcConfig.getShouldRecordStats()),
      // Start off not in GC.
      inGC_(false),
      name_(gcConfig.getName()),
      allocationLocationTracker_(this),
      tripwireCallback_(gcConfig.getTripwireConfig().getCallback()),
      tripwireLimit_(gcConfig.getTripwireConfig().getLimit())
#ifdef HERMESVM_SANITIZE_HANDLES
      ,
      sanitizeRate_(gcConfig.getSanitizeConfig().getSanitizeRate())
#endif
#ifndef NDEBUG
      ,
      randomizeAllocSpace_(gcConfig.getShouldRandomizeAllocSpace())
#endif
{
#ifdef HERMESVM_PLATFORM_LOGGING
  hermesLog(
      "HermesGC",
      "Initialisation (Init: %dMB, Max: %dMB, Tripwire: %dMB)",
      gcConfig.getInitHeapSize() >> 20,
      gcConfig.getMaxHeapSize() >> 20,
      gcConfig.getTripwireConfig().getLimit() >> 20);
#endif // HERMESVM_PLATFORM_LOGGING
#ifdef HERMESVM_SANITIZE_HANDLES
  const std::minstd_rand::result_type seed =
      gcConfig.getSanitizeConfig().getRandomSeed() >= 0
      ? gcConfig.getSanitizeConfig().getRandomSeed()
      : std::random_device()();
  if (sanitizeRate_ > 0.0 && sanitizeRate_ < 1.0) {
    llvh::errs()
        << "Warning: you are using handle sanitization with random sampling.\n"
        << "Sanitize Rate: ";
    llvh::write_double(llvh::errs(), sanitizeRate_, llvh::FloatStyle::Percent);
    llvh::errs() << "\n"
                 << "Sanitize Rate Seed: " << seed << "\n"
                 << "Re-run with -gc-sanitize-handles-random-seed=" << seed
                 << " for deterministic crashes.\n";
  }
  randomEngine_.seed(seed);
#endif
}

GCBase::GCCycle::GCCycle(
    GCBase *gc,
    OptValue<GCCallbacks *> gcCallbacksOpt,
    std::string extraInfo)
    : gc_(gc),
      gcCallbacksOpt_(gcCallbacksOpt),
      extraInfo_(std::move(extraInfo)),
      previousInGC_(gc_->inGC_.load(std::memory_order_acquire)) {
  if (!previousInGC_) {
    if (gcCallbacksOpt_.hasValue()) {
      gcCallbacksOpt_.getValue()->onGCEvent(
          GCEventKind::CollectionStart, extraInfo_);
    }
    bool wasInGC_ = gc_->inGC_.exchange(true, std::memory_order_acquire);
    (void)wasInGC_;
    assert(!wasInGC_ && "inGC_ should not be concurrently modified!");
  }
}

GCBase::GCCycle::~GCCycle() {
  if (!previousInGC_) {
    gc_->inGC_.store(false, std::memory_order_release);
    if (gcCallbacksOpt_.hasValue()) {
      gcCallbacksOpt_.getValue()->onGCEvent(
          GCEventKind::CollectionEnd, extraInfo_);
    }
  }
}

void GCBase::runtimeWillExecute() {
  if (recordGcStats_ && !execStartTimeRecorded_) {
    execStartTime_ = std::chrono::steady_clock::now();
    execStartCPUTime_ = oscompat::thread_cpu_time();
    oscompat::num_context_switches(
        startNumVoluntaryContextSwitches_, startNumInvoluntaryContextSwitches_);
    execStartTimeRecorded_ = true;
  }
}

std::error_code GCBase::createSnapshotToFile(const std::string &fileName) {
  std::error_code code;
  llvh::raw_fd_ostream os(fileName, code, llvh::sys::fs::FileAccess::FA_Write);
  if (code) {
    return code;
  }
  createSnapshot(os);
  return std::error_code{};
}

namespace {

constexpr GCBase::IDTracker::ReservedObjectID objectIDForRootSection(
    RootAcceptor::Section section) {
  // Since root sections start at zero, and in IDTracker the root sections
  // start one past the reserved super root, this number can be added to
  // do conversions.
  return static_cast<GCBase::IDTracker::ReservedObjectID>(
      static_cast<uint64_t>(GCBase::IDTracker::ReservedObjectID::Root) + 1 +
      static_cast<uint64_t>(section));
}

// Abstract base class for all snapshot acceptors.
struct SnapshotAcceptor : public SlotAcceptorWithNamesDefault {
  using SlotAcceptorWithNamesDefault::accept;
  using SlotAcceptorWithNamesDefault::SlotAcceptorWithNamesDefault;

  SnapshotAcceptor(GC &gc, HeapSnapshot &snap)
      : SlotAcceptorWithNamesDefault(gc), snap_(snap) {}

  void accept(HermesValue &hv, const char *name) override {
    if (hv.isPointer()) {
      auto ptr = hv.getPointer();
      accept(ptr, name);
    }
  }

 protected:
  HeapSnapshot &snap_;
};

struct PrimitiveNodeAcceptor : public SnapshotAcceptor {
  using SnapshotAcceptor::accept;

  PrimitiveNodeAcceptor(GC &gc, HeapSnapshot &snap)
      : SnapshotAcceptor(gc, snap) {}

  // Do nothing for any value except a number.
  void accept(void *&ptr, const char *name) override {}

  void accept(HermesValue &hv, const char *) override {
    if (hv.isNumber()) {
      seenNumbers_.insert(hv.getNumber());
    }
  }

  void writeAllNodes() {
    // Always write out the nodes for singletons.
    snap_.beginNode();
    snap_.endNode(
        HeapSnapshot::NodeType::Object,
        "undefined",
        static_cast<HeapSnapshot::NodeID>(
            GCBase::IDTracker::ReservedObjectID::Undefined),
        0,
        0);
    snap_.beginNode();
    snap_.endNode(
        HeapSnapshot::NodeType::Object,
        "null",
        static_cast<HeapSnapshot::NodeID>(
            GCBase::IDTracker::ReservedObjectID::Null),
        0,
        0);
    snap_.beginNode();
    snap_.endNode(
        HeapSnapshot::NodeType::Object,
        "true",
        static_cast<HeapSnapshot::NodeID>(
            GCBase::IDTracker::ReservedObjectID::True),
        0,
        0);
    snap_.beginNode();
    snap_.endNode(
        HeapSnapshot::NodeType::Object,
        "false",
        static_cast<HeapSnapshot::NodeID>(
            GCBase::IDTracker::ReservedObjectID::False),
        0,
        0);
    for (double num : seenNumbers_) {
      // A number never has any edges, so just make a node for it.
      snap_.beginNode();
      // Convert the number value to a string, according to the JS conversion
      // routines.
      char buf[hermes::NUMBER_TO_STRING_BUF_SIZE];
      size_t len = hermes::numberToString(num, buf, sizeof(buf));
      snap_.endNode(
          HeapSnapshot::NodeType::Number,
          llvh::StringRef{buf, len},
          gc.getIDTracker().getNumberID(num),
          // Numbers are zero-sized in the heap because they're stored inline.
          0,
          0);
    }
  }

 private:
  // Track all numbers that are seen in a heap pass, and only emit one node for
  // each of them.
  llvh::DenseSet<double, GCBase::IDTracker::DoubleComparator> seenNumbers_;
};

struct EdgeAddingAcceptor : public SnapshotAcceptor, public WeakRefAcceptor {
  using SnapshotAcceptor::accept;

  EdgeAddingAcceptor(GC &gc, HeapSnapshot &snap) : SnapshotAcceptor(gc, snap) {}

  void accept(void *&ptr, const char *name) override {
    if (!ptr) {
      return;
    }
    snap_.addNamedEdge(
        HeapSnapshot::EdgeType::Internal,
        llvh::StringRef::withNullAsEmpty(name),
        gc.getObjectID(ptr));
  }

  void accept(HermesValue &hv, const char *name) override {
    if (hv.isPointer()) {
      auto ptr = hv.getPointer();
      accept(ptr, name);
    } else if (auto id = gc.getSnapshotID(hv)) {
      snap_.addNamedEdge(
          HeapSnapshot::EdgeType::Internal,
          llvh::StringRef::withNullAsEmpty(name),
          id.getValue());
    }
  }

  void accept(WeakRefBase &wr) override {
    WeakRefSlot *slot = wr.unsafeGetSlot();
    if (slot->state() == WeakSlotState::Free) {
      // If the slot is free, there's no edge to add.
      return;
    }
    if (!slot->hasPointer()) {
      // Filter out empty refs from adding edges.
      return;
    }
    // Assume all weak pointers have no names, and are stored in an array-like
    // structure.
    std::string indexName = oscompat::to_string(nextEdge_++);
    snap_.addNamedEdge(
        HeapSnapshot::EdgeType::Weak,
        indexName,
        gc.getObjectID(slot->getPointer()));
  }

 private:
  // For unnamed edges, use indices instead.
  unsigned nextEdge_{0};
};

struct SnapshotRootSectionAcceptor : public SnapshotAcceptor,
                                     public WeakRootAcceptorDefault {
  using SnapshotAcceptor::accept;
  using WeakRootAcceptor::acceptWeak;

  SnapshotRootSectionAcceptor(GC &gc, HeapSnapshot &snap)
      : SnapshotAcceptor(gc, snap), WeakRootAcceptorDefault(gc) {}

  void accept(void *&, const char *) override {
    // While adding edges to root sections, there's no need to do anything for
    // pointers.
  }

  void accept(WeakRefBase &wr) override {
    // Same goes for weak refs.
  }

  void acceptWeak(void *&ptr) override {
    // Same goes for weak pointers.
  }

  void beginRootSection(Section section) override {
    // Make an element edge from the super root to each root section.
    snap_.addIndexedEdge(
        HeapSnapshot::EdgeType::Element,
        rootSectionNum_++,
        static_cast<HeapSnapshot::NodeID>(objectIDForRootSection(section)));
  }

  void endRootSection() override {
    // Do nothing for the end of the root section.
  }

 private:
  // v8's roots start numbering at 1.
  int rootSectionNum_{1};
};

struct SnapshotRootAcceptor : public SnapshotAcceptor,
                              public WeakRootAcceptorDefault {
  using SnapshotAcceptor::accept;
  using WeakRootAcceptor::acceptWeak;

  SnapshotRootAcceptor(GC &gc, HeapSnapshot &snap)
      : SnapshotAcceptor(gc, snap), WeakRootAcceptorDefault(gc) {}

  void accept(void *&ptr, const char *name) override {
    pointerAccept(ptr, name, false);
  }

  void acceptWeak(void *&ptr) override {
    pointerAccept(ptr, nullptr, true);
  }

  void accept(WeakRefBase &wr) override {
    WeakRefSlot *slot = wr.unsafeGetSlot();
    if (slot->state() == WeakSlotState::Free) {
      // If the slot is free, there's no edge to add.
      return;
    }
    if (!slot->hasPointer()) {
      // Filter out empty refs from adding edges.
      return;
    }
    pointerAccept(slot->getPointer(), nullptr, true);
  }

  void beginRootSection(Section section) override {
    assert(
        currentSection_ == Section::InvalidSection &&
        "beginRootSection called while previous section is open");
    snap_.beginNode();
    currentSection_ = section;
  }

  void endRootSection() override {
    // A root section creates a synthetic node with that name and makes edges
    // come from that root.
    static const char *rootNames[] = {
// Parentheses around the name is adopted from V8's roots.
#define ROOT_SECTION(name) "(" #name ")",
#include "hermes/VM/RootSections.def"
    };
    snap_.endNode(
        HeapSnapshot::NodeType::Synthetic,
        rootNames[static_cast<unsigned>(currentSection_)],
        static_cast<HeapSnapshot::NodeID>(
            objectIDForRootSection(currentSection_)),
        // The heap visualizer doesn't like it when these synthetic nodes have a
        // size (it describes them as living in the heap).
        0,
        0);
    currentSection_ = Section::InvalidSection;
    // Reset the edge counter, so each root section's unnamed edges start at
    // zero.
    nextEdge_ = 0;
  }

 private:
  llvh::DenseSet<uint64_t> seenIDs_;
  // For unnamed edges, use indices instead.
  unsigned nextEdge_{0};
  Section currentSection_{Section::InvalidSection};

  void pointerAccept(void *ptr, const char *name, bool weak) {
    assert(
        currentSection_ != Section::InvalidSection &&
        "accept called outside of begin/end root section pair");
    if (!ptr) {
      return;
    }

    const auto id = gc.getObjectID(ptr);
    if (!seenIDs_.insert(id).second) {
      // Already seen this node, don't add another edge.
      return;
    }
    auto nameRef = llvh::StringRef::withNullAsEmpty(name);
    if (!nameRef.empty()) {
      snap_.addNamedEdge(
          weak ? HeapSnapshot::EdgeType::Weak
               : HeapSnapshot::EdgeType::Internal,
          nameRef,
          id);
    } else if (weak) {
      std::string numericName = oscompat::to_string(nextEdge_++);
      snap_.addNamedEdge(HeapSnapshot::EdgeType::Weak, numericName.c_str(), id);
    } else {
      // Unnamed edges get indices.
      snap_.addIndexedEdge(HeapSnapshot::EdgeType::Element, nextEdge_++, id);
    }
  }
};

} // namespace

void GCBase::createSnapshot(GC *gc, llvh::raw_ostream &os) {
  JSONEmitter json(os);
  HeapSnapshot snap(json, gcCallbacks_->getStackTracesTree());

  const auto rootScan = [gc, &snap]() {
    {
      // Make the super root node and add edges to each root section.
      SnapshotRootSectionAcceptor rootSectionAcceptor(*gc, snap);
      snap.beginNode();
      gc->markRoots(rootSectionAcceptor, true);
      gc->markWeakRoots(rootSectionAcceptor);
      snap.endNode(
          HeapSnapshot::NodeType::Synthetic,
          "(GC Roots)",
          static_cast<HeapSnapshot::NodeID>(IDTracker::ReservedObjectID::Root),
          0,
          0);
    }
    {
      // Make a node for each root section and add edges into the actual heap.
      // Within a root section, there might be duplicates. The root acceptor
      // filters out duplicate edges because there cannot be duplicate edges to
      // nodes reachable from the super root.
      SnapshotRootAcceptor rootAcceptor(*gc, snap);
      gc->markRoots(rootAcceptor, true);
      gc->markWeakRoots(rootAcceptor);
    }
  };

  snap.beginSection(HeapSnapshot::Section::Nodes);
  rootScan();
  // Add all primitive values as nodes if they weren't added before.
  // This must be done as a step before adding any edges to these nodes.
  // In particular, custom edge adders might try to add edges to primitives that
  // haven't been recorded yet.
  // The acceptor is recording some state between objects, so define it outside
  // the loop.
  PrimitiveNodeAcceptor primitiveAcceptor(*gc, snap);
  SlotVisitorWithNames<PrimitiveNodeAcceptor> primitiveVisitor{
      primitiveAcceptor};
  // Add a node for each object in the heap.
  const auto snapshotForObject = [&snap, &primitiveVisitor, gc](GCCell *cell) {
    auto &allocationLocationTracker = gc->getAllocationLocationTracker();
    // First add primitive nodes.
    GCBase::markCellWithNames(primitiveVisitor, cell, gc);
    EdgeAddingAcceptor acceptor(*gc, snap);
    SlotVisitorWithNames<EdgeAddingAcceptor> visitor(acceptor);
    // Allow nodes to add extra nodes not in the JS heap.
    cell->getVT()->snapshotMetaData.addNodes(cell, gc, snap);
    snap.beginNode();
    // Add all internal edges first.
    GCBase::markCellWithNames(visitor, cell, gc);
    // Allow nodes to add custom edges not represented by metadata.
    cell->getVT()->snapshotMetaData.addEdges(cell, gc, snap);
    auto stackTracesTreeNode =
        allocationLocationTracker.getStackTracesTreeNodeForAlloc(cell);
    snap.endNode(
        cell->getVT()->snapshotMetaData.nodeType(),
        cell->getVT()->snapshotMetaData.nameForNode(cell, gc),
        gc->getObjectID(cell),
        cell->getAllocatedSize(),
        stackTracesTreeNode ? stackTracesTreeNode->id : 0);
  };
  gc->forAllObjs(snapshotForObject);
  // Write the singleton number nodes into the snapshot.
  primitiveAcceptor.writeAllNodes();
  snap.endSection(HeapSnapshot::Section::Nodes);

  snap.beginSection(HeapSnapshot::Section::Edges);
  rootScan();
  // No need to run the primitive scan again, as it only adds nodes, not edges.
  // Add edges between objects in the heap.
  gc->forAllObjs(snapshotForObject);
  snap.endSection(HeapSnapshot::Section::Edges);

  snap.emitAllocationTraceInfo();

  snap.beginSection(HeapSnapshot::Section::Samples);
  GCBase::IDTracker::Sampler::TimePoint startTime;
  GCBase::IDTracker::Sampler::Samples samples;
  std::tie(startTime, samples) = gc->getIDTracker().endSamplingLastID();
  for (auto sample : samples) {
    uint64_t lastID = sample.second;
    uint64_t timestampMicros =
        std::chrono::duration_cast<std::chrono::microseconds>(
            sample.first - startTime)
            .count();
    json.emitValues({timestampMicros, lastID});
  }
  snap.endSection(HeapSnapshot::Section::Samples);

  snap.beginSection(HeapSnapshot::Section::Locations);
  gc->forAllObjs([&snap, gc](GCCell *cell) {
    cell->getVT()->snapshotMetaData.addLocations(cell, gc, snap);
  });
  snap.endSection(HeapSnapshot::Section::Locations);
}

void GCBase::checkTripwire(size_t dataSize) {
  if (LLVM_LIKELY(!tripwireCallback_) ||
      LLVM_LIKELY(dataSize < tripwireLimit_) || tripwireCalled_) {
    return;
  }

  class Ctx : public GCTripwireContext {
   public:
    Ctx(GCBase *gc) : gc_(gc) {}

    std::error_code createSnapshotToFile(const std::string &path) override {
      return gc_->createSnapshotToFile(path);
    }

   private:
    GCBase *gc_;
  } ctx(this);

  tripwireCalled_ = true;
  tripwireCallback_(ctx);
}

void GCBase::printAllCollectedStats(llvh::raw_ostream &os) {
  if (!recordGcStats_)
    return;

  dump(os);
  os << "GC stats:\n";
  JSONEmitter json{os, /*pretty*/ true};
  json.openDict();
  printStats(json);
  json.closeDict();
  os << "\n";
}

void GCBase::getHeapInfo(HeapInfo &info) {
  info.numCollections = cumStats_.numCollections;
}

#ifndef NDEBUG
void GCBase::getDebugHeapInfo(DebugHeapInfo &info) {
  recordNumAllocatedObjects();
  info.numAllocatedObjects = numAllocatedObjects_;
  info.numReachableObjects = numReachableObjects_;
  info.numCollectedObjects = numCollectedObjects_;
  info.numFinalizedObjects = numFinalizedObjects_;
  info.numMarkedSymbols = numMarkedSymbols_;
  info.numHiddenClasses = numHiddenClasses_;
  info.numLeafHiddenClasses = numLeafHiddenClasses_;
}
#endif

#ifndef NDEBUG
void GCBase::DebugHeapInfo::assertInvariants() const {
  // The number of allocated objects at any time is at least the number
  // found reachable in the last collection.
  assert(numAllocatedObjects >= numReachableObjects);
  // The number of objects finalized in the last collection is at most the
  // number of objects collected.
  assert(numCollectedObjects >= numFinalizedObjects);
}
#endif

void GCBase::dump(llvh::raw_ostream &, bool) { /* nop */
}

void GCBase::printStats(JSONEmitter &json) {
  json.emitKeyValue("type", "hermes");
  json.emitKeyValue("version", 0);
  gcCallbacks_->printRuntimeGCStats(json);

  std::chrono::duration<double> elapsedTime =
      std::chrono::steady_clock::now() - execStartTime_;
  auto elapsedCPUSeconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
          oscompat::thread_cpu_time())
          .count() -
      std::chrono::duration_cast<std::chrono::duration<double>>(
          execStartCPUTime_)
          .count();

  HeapInfo info;
  getHeapInfoWithMallocSize(info);
  getHeapInfo(info);
#ifndef NDEBUG
  DebugHeapInfo debugInfo;
  getDebugHeapInfo(debugInfo);
#endif

  json.emitKey("heapInfo");
  json.openDict();
#ifndef NDEBUG
  json.emitKeyValue("Num allocated cells", debugInfo.numAllocatedObjects);
  json.emitKeyValue("Num reachable cells", debugInfo.numReachableObjects);
  json.emitKeyValue("Num collected cells", debugInfo.numCollectedObjects);
  json.emitKeyValue("Num finalized cells", debugInfo.numFinalizedObjects);
  json.emitKeyValue("Num marked symbols", debugInfo.numMarkedSymbols);
  json.emitKeyValue("Num hidden classes", debugInfo.numHiddenClasses);
  json.emitKeyValue("Num leaf classes", debugInfo.numLeafHiddenClasses);
  json.emitKeyValue("Num weak references", ((GC *)this)->countUsedWeakRefs());
#endif
  json.emitKeyValue("Peak RSS", oscompat::peak_rss());
  json.emitKeyValue("Current RSS", oscompat::current_rss());
  json.emitKeyValue("Current Dirty", oscompat::current_private_dirty());
  json.emitKeyValue("Heap size", info.heapSize);
  json.emitKeyValue("Allocated bytes", info.allocatedBytes);
  json.emitKeyValue("Num collections", info.numCollections);
  json.emitKeyValue("Malloc size", info.mallocSizeEstimate);
  json.closeDict();

  long vol = -1;
  long invol = -1;
  if (oscompat::num_context_switches(vol, invol)) {
    vol -= startNumVoluntaryContextSwitches_;
    invol -= startNumInvoluntaryContextSwitches_;
  }

  json.emitKey("general");
  json.openDict();
  json.emitKeyValue("numCollections", cumStats_.numCollections);
  json.emitKeyValue("totalTime", elapsedTime.count());
  json.emitKeyValue("totalCPUTime", elapsedCPUSeconds);
  json.emitKeyValue("totalGCTime", formatSecs(cumStats_.gcWallTime.sum()).secs);
  json.emitKeyValue("volCtxSwitch", vol);
  json.emitKeyValue("involCtxSwitch", invol);
  json.emitKeyValue(
      "avgGCPause", formatSecs(cumStats_.gcWallTime.average()).secs);
  json.emitKeyValue("maxGCPause", formatSecs(cumStats_.gcWallTime.max()).secs);
  json.emitKeyValue(
      "totalGCCPUTime", formatSecs(cumStats_.gcCPUTime.sum()).secs);
  json.emitKeyValue(
      "avgGCCPUPause", formatSecs(cumStats_.gcCPUTime.average()).secs);
  json.emitKeyValue(
      "maxGCCPUPause", formatSecs(cumStats_.gcCPUTime.max()).secs);
  json.emitKeyValue("finalHeapSize", formatSize(cumStats_.finalHeapSize).bytes);
  json.emitKeyValue(
      "peakAllocatedBytes", formatSize(getPeakAllocatedBytes()).bytes);
  json.emitKeyValue("peakLiveAfterGC", formatSize(getPeakLiveAfterGC()).bytes);
  json.emitKeyValue(
      "totalAllocatedBytes", formatSize(info.totalAllocatedBytes).bytes);
  json.closeDict();

  json.emitKey("collections");
  json.openArray();
  for (const auto &event : analyticsEvents_) {
    json.openDict();
    json.emitKeyValue("runtimeDescription", event.runtimeDescription);
    json.emitKeyValue("gcKind", event.gcKind);
    json.emitKeyValue("collectionType", event.collectionType);
    json.emitKeyValue("duration", event.duration.count());
    json.emitKeyValue("cpuDuration", event.cpuDuration.count());
    json.emitKeyValue("preAllocated", event.preAllocated);
    json.emitKeyValue("preSize", event.preSize);
    json.emitKeyValue("postAllocated", event.postAllocated);
    json.emitKeyValue("postSize", event.postSize);
    json.emitKeyValue("survivalRatio", event.survivalRatio);
    json.closeDict();
  }
  json.closeArray();
}

void GCBase::recordGCStats(
    const GCAnalyticsEvent &event,
    CumulativeHeapStats *stats) {
  stats->gcWallTime.record(
      std::chrono::duration<double>(event.duration).count());
  stats->gcCPUTime.record(
      std::chrono::duration<double>(event.cpuDuration).count());
  stats->finalHeapSize = event.postSize;
  stats->usedBefore.record(event.preAllocated);
  stats->usedAfter.record(event.postAllocated);
  stats->numCollections++;
}

void GCBase::recordGCStats(const GCAnalyticsEvent &event) {
  if (analyticsCallback_) {
    analyticsCallback_(event);
  }
  if (recordGcStats_) {
    analyticsEvents_.push_back(event);
  }
  recordGCStats(event, &cumStats_);
}

void GCBase::oom(std::error_code reason) {
#ifdef HERMESVM_EXCEPTION_ON_OOM
  HeapInfo heapInfo;
  getHeapInfo(heapInfo);
  char detailBuffer[400];
  snprintf(
      detailBuffer,
      sizeof(detailBuffer),
      "Javascript heap memory exhausted: heap size = %d, allocated = %d.",
      heapInfo.heapSize,
      heapInfo.allocatedBytes);
  throw JSOutOfMemoryError(
      std::string(detailBuffer) + "\ncall stack:\n" +
      gcCallbacks_->getCallStackNoAlloc());
#else
  oomDetail(reason);
  hermes_fatal((llvh::Twine("OOM: ") + convert_error_to_message(reason)).str());
#endif
}

void GCBase::oomDetail(std::error_code reason) {
  HeapInfo heapInfo;
  getHeapInfo(heapInfo);
  // Could use a stringstream here, but want to avoid dynamic allocation.
  char detailBuffer[400];
  snprintf(
      detailBuffer,
      sizeof(detailBuffer),
      "[%.20s] reason = %.150s (%d from category: %.50s), numCollections = %d, heapSize = %d, allocated = %d, va = %" PRIu64,
      name_.c_str(),
      reason.message().c_str(),
      reason.value(),
      reason.category().name(),
      heapInfo.numCollections,
      heapInfo.heapSize,
      heapInfo.allocatedBytes,
      heapInfo.va);
  hermesLog("HermesGC", "OOM: %s.", detailBuffer);
  // Record the OOM custom data with the crash manager.
  crashMgr_->setCustomData("HermesGCOOMDetailBasic", detailBuffer);
}

#ifndef NDEBUG
/*static*/
bool GCBase::isMostRecentCellInFinalizerVector(
    const std::vector<GCCell *> &finalizables,
    const GCCell *cell) {
  return !finalizables.empty() && finalizables.back() == cell;
}
#endif

#ifdef HERMESVM_SANITIZE_HANDLES
bool GCBase::shouldSanitizeHandles() {
  static std::uniform_real_distribution<> dist(0.0, 1.0);
  return dist(randomEngine_) < sanitizeRate_;
}
#endif

/*static*/
std::list<detail::WeakRefKey *> GCBase::buildKeyList(
    GC *gc,
    JSWeakMap *weakMap) {
  std::list<detail::WeakRefKey *> res;
  for (auto iter = weakMap->keys_begin(), end = weakMap->keys_end();
       iter != end;
       iter++) {
    if (iter->getObject(gc)) {
      res.push_back(&(*iter));
    }
  }
  return res;
}

WeakRefMutex &GCBase::weakRefMutex() {
  return weakRefMutex_;
}

/*static*/
double GCBase::clockDiffSeconds(TimePoint start, TimePoint end) {
  std::chrono::duration<double> elapsed = (end - start);
  return elapsed.count();
}

/*static*/
double GCBase::clockDiffSeconds(
    std::chrono::microseconds start,
    std::chrono::microseconds end) {
  std::chrono::duration<double> elapsed = (end - start);
  return elapsed.count();
}

llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const DurationFormatObj &dfo) {
  if (dfo.secs >= 1.0) {
    os << format("%5.3f", dfo.secs) << " s";
  } else if (dfo.secs >= 0.001) {
    os << format("%5.3f", dfo.secs * 1000.0) << " ms";
  } else {
    os << format("%5.3f", dfo.secs * 1000000.0) << " us";
  }
  return os;
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const SizeFormatObj &sfo) {
  double dblsize = static_cast<double>(sfo.bytes);
  if (sfo.bytes >= (1024 * 1024 * 1024)) {
    double gbs = dblsize / (1024.0 * 1024.0 * 1024.0);
    os << format("%0.3f GiB", gbs);
  } else if (sfo.bytes >= (1024 * 1024)) {
    double mbs = dblsize / (1024.0 * 1024.0);
    os << format("%0.3f MiB", mbs);
  } else if (sfo.bytes >= 1024) {
    double kbs = dblsize / 1024.0;
    os << format("%0.3f KiB", kbs);
  } else {
    os << sfo.bytes << " B";
  }
  return os;
}

GCBase::GCCallbacks::~GCCallbacks() {}

GCBase::IDTracker::IDTracker() {
  assert(nextID_ % 2 == 1 && "First JS object ID isn't odd");
}

#ifdef HERMESVM_SERIALIZE
void GCBase::AllocationLocationTracker::serialize(Serializer &s) const {
  if (enabled_) {
    hermes_fatal(
        "Serialization not supported when AllocationLocationTracker enabled");
  }
}

void GCBase::AllocationLocationTracker::deserialize(Deserializer &d) {
  if (enabled_) {
    hermes_fatal(
        "Deserialization not supported when AllocationLocationTracker enabled");
  }
}

void GCBase::IDTracker::serialize(Serializer &s) const {
  s.writeInt<HeapSnapshot::NodeID>(nextID_);
  s.writeInt<size_t>(objectIDMap_.size());
  for (auto it = objectIDMap_.begin(); it != objectIDMap_.end(); it++) {
    s.writeRelocation(it->first);
    s.writeInt<HeapSnapshot::NodeID>(it->second);
  }
}

void GCBase::IDTracker::deserialize(Deserializer &d) {
  nextID_ = d.readInt<HeapSnapshot::NodeID>();
  size_t size = d.readInt<size_t>();
  for (size_t i = 0; i < size; i++) {
    // Heap must have been deserialized before this function. All deserialized
    // pointer must be non-null at this time.
    const void *ptr = d.getNonNullPtr();
    auto res =
        objectIDMap_.try_emplace(ptr, d.readInt<HeapSnapshot::NodeID>()).second;
    (void)res;
    assert(res && "Shouldn't fail to insert during deserialization");
  }
}
#endif

void GCBase::IDTracker::untrackUnmarkedSymbols(
    const std::vector<bool> &markedSymbols) {
  std::vector<uint32_t> toUntrack;
  for (const auto &pair : symbolIDMap_) {
    if (!markedSymbols[pair.first]) {
      toUntrack.push_back(pair.first);
    }
  }
  for (uint32_t symIdx : toUntrack) {
    symbolIDMap_.erase(symIdx);
  }
}

HeapSnapshot::NodeID GCBase::IDTracker::getNumberID(double num) {
  auto &numberRef = numberIDMap_[num];
  // If the entry didn't exist, the value was initialized to 0.
  if (numberRef != 0) {
    return numberRef;
  }
  // Else, it is a number that hasn't been seen before.
  return numberRef = nextNumberID();
}

void GCBase::IDTracker::beginSamplingLastID(Sampler::Duration duration) {
  endSamplingLastID();
  sampler_ = llvh::make_unique<Sampler>(nextID_, duration);
}

std::pair<
    GCBase::IDTracker::Sampler::TimePoint,
    GCBase::IDTracker::Sampler::Samples>
GCBase::IDTracker::endSamplingLastID() {
  if (sampler_) {
    std::pair<Sampler::TimePoint, Sampler::Samples> result =
        make_pair(sampler_->startTime(), sampler_->stop());
    sampler_.reset();
    return result;
  } else {
    return std::make_pair(Sampler::TimePoint{}, Sampler::Samples{});
  }
}

void GCBase::AllocationLocationTracker::enable() {
  enabled_ = true;
  GC *gc = reinterpret_cast<GC *>(gc_);
  // For correct visualization of the allocation timeline, it's necessary that
  // objects in the heap snapshot that existed before sampling was enabled have
  // numerically lower IDs than those allocated during sampling. We ensure this
  // by assigning IDs to everything here.
  gc->forAllObjs([gc](GCCell *cell) { gc->getIDTracker().getObjectID(cell); });
  gc->getIDTracker().beginSamplingLastID(std::chrono::milliseconds(50));
}

void GCBase::AllocationLocationTracker::disable() {
  gc_->getIDTracker().endSamplingLastID();
  enabled_ = false;
}

llvh::Optional<HeapSnapshot::NodeID> GCBase::getSnapshotID(HermesValue val) {
  if (val.isPointer() && val.getPointer()) {
    // Make nullptr HermesValue look like a JS null.
    // This should be rare, but is occasionally used by some parts of the VM.
    return val.getPointer()
        ? getObjectID(val.getPointer())
        : static_cast<HeapSnapshot::NodeID>(IDTracker::ReservedObjectID::Null);
  } else if (val.isNumber()) {
    return idTracker_.getNumberID(val.getNumber());
  } else if (val.isUndefined()) {
    return static_cast<HeapSnapshot::NodeID>(
        IDTracker::ReservedObjectID::Undefined);
  } else if (val.isNull()) {
    return static_cast<HeapSnapshot::NodeID>(IDTracker::ReservedObjectID::Null);
  } else if (val.isBool()) {
    return static_cast<HeapSnapshot::NodeID>(
        val.getBool() ? IDTracker::ReservedObjectID::True
                      : IDTracker::ReservedObjectID::False);
  } else {
    return llvh::None;
  }
}

} // namespace vm
} // namespace hermes
