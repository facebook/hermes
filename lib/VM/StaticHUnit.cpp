/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StaticH-internal.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/JSObject.h"

using namespace hermes;
using namespace hermes::vm;

static_assert(SH_PROPERTY_CACHE_ENTRY_SIZE == sizeof(PropertyCacheEntry));

/// Data associated with SHUnit, but fully managed by the runtime.
struct SHUnitExt {
  /// A map from NewObjectWithBuffer's <keyBufferIndex, numLiterals> tuple to
  /// its shared hidden class.
  /// During hashing, keyBufferIndex takes the top 24bits while numLiterals
  /// becomes the lower 8bits of the key.
  /// Cacheing will be skipped if keyBufferIndex is >= 2^24.
  llvh::DenseMap<uint32_t, WeakRoot<HiddenClass>> objectLiteralHiddenClasses{};

  /// A map from template object ids to template objects.
  llvh::DenseMap<uint32_t, JSObject *> templateMap{};
};

static void sh_unit_init_symbols(Runtime &runtime, SHUnit *unit);
static SHLegacyValue sh_unit_run(SHRuntime *shr, SHUnit *unit);

extern "C" bool _sh_initialize_units(SHRuntime *shr, uint32_t count, ...) {
  Runtime &runtime = getRuntime(shr);
  GCScope gcScope{runtime};

  SHLocals locals;
  SHLegacyValue *frame = runtime.getCurrentFrame().ptr();
  SHLegacyValue *savedSP = _sh_push_locals(shr, &locals, 0);
  locals.count = 1;
  SHJmpBuf jbuf;
  bool success = true;

  va_list ap;
  va_start(ap, count);

  if (_sh_try(shr, &jbuf) == 0) {
    for (uint32_t i = 0; i < count; ++i) {
      SHUnit *unit = va_arg(ap, SHUnit *);
      (void)_sh_unit_init(shr, unit);
    }

    _sh_end_try(shr, jbuf.prev);
  } else {
    SHLegacyValue exc = _sh_catch(shr, &locals, frame, savedSP - frame);
    // Make sure stdout catches up to stderr.
    llvh::outs().flush();
    runtime.printException(
        llvh::errs(), runtime.makeHandle(HermesValue::fromRaw(exc.raw)));
    success = false;
  }

  va_end(ap);

  _sh_pop_locals(shr, &locals, savedSP);
  return success;
}

extern "C" SHLegacyValue _sh_unit_init(SHRuntime *shr, SHUnit *unit) {
  Runtime &runtime = getRuntime(shr);

  if (unit->in_use) {
    fprintf(
        stderr,
        "SH unit '%s' already registered in an active runtime\n",
        unit->unit_name);
    abort();
  }
  unit->in_use = true;

  // If the unit is dirty, clean the property cache.
  if (unit->dirty) {
    memset(
        unit->prop_cache,
        0,
        unit->num_prop_cache_entries * sizeof(PropertyCacheEntry));
  }
  unit->dirty = true;

  // We need to clean the symbol table in case the GC runs while we are
  // populating it.
  std::fill_n(unit->symbols, unit->num_symbols, SymbolID::EMPTY_ID);

  unit->runtime_ext = new SHUnitExt{};

  // Register the unit with the runtime before it is initialized, because the
  // GC could run in the middle of initialization and the already defined
  // symbols should be roots.
  runtime.shUnits.push_back(unit);

  sh_unit_init_symbols(runtime, unit);
  return sh_unit_run(shr, unit);
}

static SHLegacyValue sh_unit_run(SHRuntime *shr, SHUnit *unit) {
  Runtime &runtime = getRuntime(shr);
  struct {
    SHLocals head;
    SHLegacyValue env;
  } locals;
  // NOTE: sh_unit_run() is not a function call, it executes in an existing
  // frame.
  SHLegacyValue *savedSP = _sh_push_locals(
      shr, &locals.head, hbc::StackFrameLayout::callerOutgoingRegisters(0));
  locals.head.count = 1;
  locals.env = _sh_ljs_null();

  SHLegacyValue closure = _sh_ljs_create_closure(
      shr,
      &locals.env,
      unit->unit_main,
      Predefined::getSymbolID(Predefined::Str::emptyString).unsafeGetRaw(),
      0);

  (void)StackFramePtr::initFrame(
      runtime.getStackPointer(),
      runtime.getCurrentFrame(),
      nullptr,
      nullptr,
      0,
      HermesValue::fromRaw(closure.raw),
      HermesValue::encodeUndefinedValue());

  SHLegacyValue res = SHLegacyFunction::_legacyCall(
      shr, vmcast<SHLegacyFunction>(HermesValue::fromRaw(closure.raw)));

  _sh_pop_locals(shr, &locals.head, savedSP);
  return res;
}

static void sh_unit_init_symbols(Runtime &runtime, SHUnit *unit) {
  // Iterate over all strings and add them to the identifier table.

  // NOTE: since SH units cannot be destroyed/unloaded before the runtime itself
  // is destroyed, we treat units as "persistent", which allows us to register
  // all strings as "lazy". A lazy string reserves an entry in the identifier
  // table with a pointer to the string source, but does not allocate a copy in
  // the GC heap.
  // This speeds up the init path by eliminating almost all GC operations and
  // copies.
  //
  // If units could be unloaded, we would have to adopt the more complicated
  // scheme used by the bytecode. Identifier strings are always registered, even
  // if it requires a copy in the GC heap, while string literals are registered
  // lazily when needed. This ensures that operations using ids remain fast.

  const uint32_t *stringData = unit->strings;
  for (uint32_t symIndex = 0, nSyms = unit->num_symbols; symIndex != nSyms;
       ++symIndex, stringData += 3) {
    SHSymbolID id;
    // UTF-16 string?
    if (stringData[0] & 0x80000000) {
      auto str = UTF16Ref{
          unit->u16_pool + (stringData[0] & 0x7FFFFFFF), stringData[1]};
      // TODO: HACK HACK: if the hash is 0, calculate it.
      uint32_t hash = stringData[2] ? stringData[2] : hashString(str);
      id = runtime.getIdentifierTable()
               .registerLazyIdentifier(str, hash)
               .unsafeGetRaw();
    } else {
      auto str = ASCIIRef{unit->ascii_pool + stringData[0], stringData[1]};
      // TODO: HACK HACK: if the hash is 0, calculate it.
      uint32_t hash = stringData[2] ? stringData[2] : hashString(str);
      id = runtime.getIdentifierTable()
               .registerLazyIdentifier(str, hash)
               .unsafeGetRaw();
    }
    unit->symbols[symIndex] = id;
  }
}

void hermes::vm::sh_unit_done(Runtime &runtime, SHUnit *unit) {
  assert(unit->in_use && "destroying unit which is not in use");

  delete unit->runtime_ext;
  unit->runtime_ext = nullptr;

  unit->in_use = false;
}

size_t hermes::vm::sh_unit_additional_memory_size(const SHUnit *unit) {
  return sizeof(unit->runtime_ext) +
      unit->runtime_ext->objectLiteralHiddenClasses.getMemorySize() +
      unit->runtime_ext->templateMap.getMemorySize();
}

void hermes::vm::sh_unit_mark_roots(
    SHUnit *unit,
    RootAndSlotAcceptorWithNames &acceptor,
    bool markLongLived) {
  for (auto &it : unit->runtime_ext->templateMap) {
    acceptor.acceptPtr(it.second);
  }

  if (markLongLived) {
    for (const SHSymbolID *p = unit->symbols, *e = p + unit->num_symbols;
         p != e;
         ++p) {
      const RootSymbolID sym(SymbolID::unsafeCreate(*p));
      if (sym.isValid())
        acceptor.accept(sym);
    }
  }
}

void hermes::vm::sh_unit_mark_long_lived_weak_roots(
    SHUnit *unit,
    WeakRootAcceptor &acceptor) {
  for (auto &prop : llvh::makeMutableArrayRef(
           reinterpret_cast<PropertyCacheEntry *>(unit->prop_cache),
           unit->num_prop_cache_entries)) {
    if (prop.clazz)
      acceptor.acceptWeak(prop.clazz);
  }

  for (auto &entry : unit->runtime_ext->objectLiteralHiddenClasses) {
    if (entry.second) {
      acceptor.acceptWeak(entry.second);
    }
  }
}

extern "C" void *_sh_find_cached_template_object(
    const SHUnit *unit,
    uint32_t templateObjID) {
  return unit->runtime_ext->templateMap.lookup(templateObjID);
}

extern "C" void _sh_cache_template_object(
    SHUnit *unit,
    uint32_t templateObjID,
    SHLegacyValue templateObj) {
  assert(
      unit->runtime_ext->templateMap.count(templateObjID) == 0 &&
      "The template object already exists.");
  unit->runtime_ext->templateMap[templateObjID] =
      vmcast<JSObject>(HermesValue::fromRaw(templateObj.raw));
}
