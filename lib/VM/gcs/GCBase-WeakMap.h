/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCBASE_WEAKMAP_H
#define HERMES_VM_GCBASE_WEAKMAP_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/SkipWeakRefsAcceptor.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"

namespace hermes {
namespace vm {

/*static*/
template <typename IsMarkedFunc>
void GCBase::clearEntriesWithUnreachableKeys(
    GC &gc,
    JSWeakMap *weakMap,
    IsMarkedFunc objIsMarked) {
  for (auto iter = weakMap->keys_begin(), end = weakMap->keys_end();
       iter != end;
       iter++) {
    JSObject *keyObj = iter->getObjectInGC(gc);
    if (!keyObj || !objIsMarked(keyObj)) {
      weakMap->clearEntryDirect(gc, *iter);
    }
  }
}

/*static*/
template <typename Acceptor, typename ObjIsMarkedFunc, typename MarkFromValFunc>
bool GCBase::markFromReachableWeakMapKeys(
    GC &gc,
    JSWeakMap *weakMap,
    Acceptor &acceptor,
    llvh::DenseMap<JSWeakMap *, std::vector<detail::WeakRefKey *>>
        *unreachableKeys,
    ObjIsMarkedFunc objIsMarked,
    MarkFromValFunc markFromVal) {
  std::vector<detail::WeakRefKey *> *keyList = nullptr;
  auto keyListIter = unreachableKeys->find(weakMap);
  if (keyListIter == unreachableKeys->end()) {
    (*unreachableKeys)[weakMap] = GCBase::buildKeyList(gc, weakMap);
    keyList = &(*unreachableKeys)[weakMap];
  } else {
    keyList = &keyListIter->second;
  }
  bool newlyMarkedValue = false;
  // Find any reachable keys, mark from the corresponding value, and
  // remove them from the list.
  auto eraseFrom = std::remove_if(
      keyList->begin(),
      keyList->end(),
      [weakMap, &gc, objIsMarked, markFromVal, &newlyMarkedValue](
          detail::WeakRefKey *key) {
        GCCell *cell = key->getObjectInGC(gc);
        if (!cell) {
          // Remove key from list.
          return true;
        }

        if (objIsMarked(cell)) {
          GCHermesValue *valPtr = weakMap->getValueDirect(gc, *key);
          assert(valPtr != nullptr && "Key is not in the map?");
          if (valPtr->isPointer()) {
            GCCell *valCell = reinterpret_cast<GCCell *>(valPtr->getPointer());

            if (markFromVal(valCell, *valPtr)) {
              newlyMarkedValue = true;
            }
          }

          // Key was reachable; remove from list.
          return true;
        }
        // Key was unreachable; do not remove from list.
        return false;
      });
  keyList->erase(eraseFrom, keyList->end());
  return newlyMarkedValue;
}

/*static*/
template <
    typename Acceptor,
    typename ObjIsMarkedFunc,
    typename MarkFromValFunc,
    typename DrainMarkStackFunc,
    typename CheckMarkStackOverflowFunc>
gcheapsize_t GCBase::completeWeakMapMarking(
    GC &gc,
    Acceptor &acceptor,
    std::vector<JSWeakMap *> &reachableWeakMaps,
    ObjIsMarkedFunc objIsMarked,
    MarkFromValFunc markFromVal,
    DrainMarkStackFunc drainMarkStack,
    CheckMarkStackOverflowFunc checkMarkStackOverflow) {
  // If a WeakMap is present as a key in this map, the corresponding list
  // is a superset of the unreachable keys in the WeakMap.  (The set last
  // found to be unreachable, some of which may now be reachable.)
  llvh::DenseMap<JSWeakMap *, std::vector<detail::WeakRefKey *>>
      unreachableKeys;

  /// A specialized acceptor, which does not mark weak refs.  We will
  /// revisit the WeakMaps with an acceptor that does, at the end.
  SkipWeakRefsAcceptor<Acceptor> skipWeakAcceptor(gc, &acceptor);

  // Must declare this outside the loop, but the initial value doesn't matter:
  // we make it false at the start of each loop iteration.
  bool newReachableValueFound = true;

  // The set of weak maps that have already been scanned -- we do the
  // initial scan of each weak map only once.
  llvh::DenseSet<JSWeakMap *> scannedWeakMaps;

  /// The total size of the reachable WeakMaps.
  gcheapsize_t weakMapAllocBytes = 0;
  do {
    newReachableValueFound = false;
    // Note that new reachable weak maps may be discovered during the loop, so
    // reachableWeakMap.size() may increase during the loop.
    for (unsigned i = 0; i < reachableWeakMaps.size(); i++) {
      JSWeakMap *weakMap = reachableWeakMaps[i];

      if (scannedWeakMaps.count(weakMap) == 0) {
        weakMapAllocBytes += weakMap->getAllocatedSize();
        // We need to scan the weak map here, to ensure that objects
        // reachable from it (e.g., hidden class) are marked.  But we
        // have to make one exception: the valueStorage field.  The
        // whole point of weak map marking is to mark only the value
        // fields that correspond to already-reachable keys; if we
        // marked and drained, we would mark the value storage
        // normally, and thus mark *all* objects reachable from it.
        // So we temporarily null out the field, and restore it after.
        auto &valueStorageRef = weakMap->getValueStorageRef(gc);
        CompressedPointer valueStorage = valueStorageRef;
        valueStorageRef.setInGC(CompressedPointer{nullptr});
        gc.markCell(weakMap, skipWeakAcceptor);
        drainMarkStack(acceptor);
        valueStorageRef.setInGC(valueStorage);
        scannedWeakMaps.insert(weakMap);
        // This handles an obscure potential bug: if we've already
        // determined reachable keys for weak map 0, the scanning
        // above of weak map 1 may have made otherwise-unreachable
        // keys in weak map 0 reachable.  (Perhaps weak map 1 has a
        // property that is also a key in map 0.)  Therefore, to be
        // safe, make sure we make another pass through this loop
        // whenever we do any scanning.
        newReachableValueFound = true;
      }

      if (markFromReachableWeakMapKeys(
              gc,
              weakMap,
              acceptor,
              &unreachableKeys,
              objIsMarked,
              markFromVal)) {
        newReachableValueFound = true;
      }
    }
  } while (newReachableValueFound);

  // If mark stack overflow occurred, terminate.
  if (checkMarkStackOverflow()) {
    // We return 0 in this case; the reachable WeakMaps will be
    // discovered again, and we'll compute their total size in the iteration
    // that eventually succeeds.
    return 0;
  }

#ifndef NDEBUG
  const auto numReachableWeakMaps = reachableWeakMaps.size();
#endif
  for (auto *weakMap : reachableWeakMaps) {
    clearEntriesWithUnreachableKeys(gc, weakMap, objIsMarked);
    // Previously we scanned the weak map while its value storage was
    // temporarily nulled out, using an acceptor that skipped weak
    // references.  Now scan again, so that we mark both the weak
    // references and the value storage, and everything reachable from
    // them.  The values in the the value storage corresponding to
    // reachable keys have already been marked from, and values
    // corresponding to unreachable keys have been cleared.  But it is
    // still necessary to mark the value storage: for example, if it
    // is a SegmentedArray, we need to mark any reachable array
    // segment objects.  Note that we might visit some already-visited
    // fields, pointing to already-marked objects; this is why we
    // require the acceptor to be idempotent.
    gc.markCell(weakMap, acceptor);
    drainMarkStack(acceptor);
  }
  // Because of the limited nature of the marking done above, we can
  // assert that overflow did not occur.
  assert(!checkMarkStackOverflow());
  // Also ensure that no new WeakMaps were encountered during the final drains.
  assert(
      numReachableWeakMaps == reachableWeakMaps.size() &&
      "A reachable weak map wasn't marked");

  return weakMapAllocBytes;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_WEAKMAP_H
