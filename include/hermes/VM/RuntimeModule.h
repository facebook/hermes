#ifndef HERMES_VM_RUNTIMEMODULE_H
#define HERMES_VM_RUNTIMEMODULE_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/Support/HashString.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/StringRefUtils.h"

#include "llvm/ADT/simple_ilist.h"

namespace hermes {
namespace vm {

class CodeBlock;
class Runtime;

using StringID = uint32_t;

namespace detail {
/// Unit tests need to call into this function. We cannot expose the
/// templated version as its definition is in the cpp file, and will
/// cause a link error.
StringID mapString(RuntimeModule &module, const char *str);
} // namespace detail

/// Flags supporting RuntimeModule.
union RuntimeModuleFlags {
  struct {
    /// Whether this runtime module should persist in memory (i.e. never get
    /// freed even when refCount_ goes to 0.) This is needed when we want to
    /// have lazy identifiers whose string content is a pointer to the string
    /// storage in the bytecode module. We should only make the first (biggest)
    /// module persistent.
    bool persistent : 1;

    /// Whether this runtime module's epilogue should be hidden in
    /// runtime.getEpilogues().
    bool hidesEpilogue : 1;
  };
  uint8_t flags;
  RuntimeModuleFlags() : flags(0) {}
};

/// This class is used to store the non-instruction information needed to
/// execute code. The RuntimeModule owns a BytecodeModule, from which it copies
/// the string ID map and function map. Every CodeBlock contains a reference to
/// the RuntimeModule that contains its relevant information. Whenever a
/// JSFunction is created/destroyed, it will update the reference count of the
/// runtime module following through the code block.
/// CodeBlock's bytecode buffers live in a BytecodeFunction, which is owned by
/// BytecodeModule, which is stored in this RuntimeModule.
///
/// If executing a CodeBlock, construct a RuntimeModule with
/// RuntimeModule::create(runtime) first. If the string ID map and function map
/// are needed, then use RuntimeModule::create(runtime, bytecodeModule).
/// If the runtime module created shall be owned by the current scope instead
/// of JSFunctions, use createManual.
///
/// All RuntimeModule-s associated with a \c Runtime are kept together in a
/// linked list which can be walked to perform memory management tasks.
class RuntimeModule final : public llvm::ilist_node<RuntimeModule> {
 public:
  /// Keeps track of each CommonJS module,
  /// which are kept on a per RuntimeModule basis.
  /// If module == nullptr, initialization has not begun.
  /// If cachedExports is empty:
  ///   - If module == nullptr, then initialization has not begun.
  ///   - Else, initialization is in progress (recursive require).
  /// If cachedExports is non-empty, initialization is complete,
  /// and cachedExports contains the final exported property.
  struct CJSModule {
    /// Cache of the module.exports property, populated after require()
    /// completes. We store both module and cachedExports in order to correctly
    /// handle the case when the CJS module is still initializing and the
    /// exports value may change during module initialization.
    PinnedHermesValue cachedExports;

    /// Encapsulating object of the given module.
    /// Created when initialization of the CJS module begins.
    /// Contains the `exports` property, which contains the exported values.
    JSObject *module;

    /// Index of the function in this RuntimeModule.
    uint32_t functionIndex;

    CJSModule(uint32_t functionIndex)
        : cachedExports(HermesValue::encodeEmptyValue()),
          module(nullptr),
          functionIndex(functionIndex) {}
  };

 private:
  friend StringID detail::mapString(RuntimeModule &module, const char *str);

  /// The runtime this module is associated with.
  Runtime *runtime_;

  /// The table maps from a sequential string id in the bytecode to an
  /// SymbolID.
  std::vector<SymbolID> stringIDMap_;

  /// The table maps from a function index to a CodeBlock.
  std::vector<CodeBlock *> functionMap_{};

  /// The byte-code provider for this RuntimeModule. The RuntimeModule is
  /// designed to own the provider exclusively, especially because in some
  /// cases the bytecode can be modified (e.g. for breakpoints). This however
  /// is a shared_ptr<> instead of unique_ptr<> for a pragmatic reason - when
  /// we run performance tests, we want to re-use a BCProvider between runtimes
  /// in order to minimize the noise.
  std::shared_ptr<hbc::BCProvider> bcProvider_{};

  /// RuntimeModule is manually managed through reference counting,
  /// which is more efficient than shared_ptr.
  uint32_t refCount_{0};

  /// Flags associated with the module.
  RuntimeModuleFlags flags_{};

  /// The sourceURL set explicitly for the module, or empty if none.
  std::string sourceURL_{};

  /// A list of RuntimeModules that this module depends on, specifically
  /// because they're lazily compiled and should be considered a unit.
  std::vector<RuntimeModule *> dependentModules_{};

  /// A map from NewObjectWithBuffer's <keyBufferIndex, numLiterals> tuple to
  /// its shared hidden class.
  /// During hashing, keyBufferIndex takes the top 24bits while numLiterals
  /// becomes the lower 8bits of the key.
  /// Cacheing will be skipped if keyBufferIndex is >= 2^24.
  llvm::DenseMap<uint32_t, HiddenClass *> objectLiteralHiddenClasses_;

  explicit RuntimeModule(
      Runtime *runtime,
      RuntimeModuleFlags flags,
      llvm::StringRef sourceURL);

  CodeBlock *getCodeBlockSlowPath(unsigned index);

  /// CJS Modules used when modules have been resolved ahead of time.
  /// Used during requireFast modules by index.
  /// For example, requireFast(2) requires the 2nd element of this vector.
  /// Avoid using a deque here to avoid unnecessary overhead when not running
  /// CJS modules.
  llvm::SmallVector<CJSModule, 1> cjsModules_{};

  /// Map of { StringID => CJS module index }.
  /// Used when doing a slow require() call that needs to resolve a filename.
  /// Need to store the index instead of a direct pointer because SmallVector
  /// can copy on push_back, unlike deque.
  llvm::SmallDenseMap<SymbolID, uint32_t, 1> cjsModuleTable_{};

 public:
  ~RuntimeModule();

  /// Creates a new RuntimeModule under \p runtime.
  /// \param runtime the runtime to use for the identifier table.
  /// \param bytecode the bytecode to import strings and functions from.
  /// \param sourceURL the filename to report in exception backtraces.
  /// \return a raw pointer to the runtime module.
  static RuntimeModule *create(
      Runtime *runtime,
      std::shared_ptr<hbc::BCProvider> &&bytecode = nullptr,
      RuntimeModuleFlags flags = {},
      llvm::StringRef sourceURL = {});

  /// Creates a new RuntimeModule that is not yet initialized. It may be
  /// initialized later through lazy compilation.
  /// \param runtime the runtime to use for the identifier table.
  /// \return a raw pointer to the runtime module.
  static RuntimeModule *createUninitialized(Runtime *runtime) {
    return new RuntimeModule(runtime, RuntimeModuleFlags{}, "");
  }

#ifndef HERMESVM_LEAN
  /// Crates a lazy RuntimeModule as part of lazy compilation. This module
  /// will contain only one CodeBlock that points to \p function. This newly
  /// created RuntimeModule is going to be a dependent of the \p parent.
  static RuntimeModule *createLazyModule(
      Runtime *runtime,
      RuntimeModule *parent,
      uint32_t functionID);

  /// If a CodeBlock in this module is compiled lazily, it generates a new
  /// RuntimeModule. The parent module should have a dependency on the child.
  void addDependency(RuntimeModule *module);

  /// Verifies that there is only one CodeBlock in this module, and return it.
  /// This is used when a lazy code block is created which should be the only
  /// block in the module.
  CodeBlock *getOnlyLazyCodeBlock() const {
    assert(functionMap_.size() == 1 && functionMap_[0] && "Not a lazy module?");
    return functionMap_[0];
  }

  /// Get the name symbol ID associated with the getOnlyLazyCodeBlock().
  SymbolID getLazyName();

  /// Initialize lazy modules created with \p createUninitialized.
  /// Calls `initialize` and does a bit of extra work.
  /// \param bytecode the bytecode data to initialize it with.
  void initializeLazy(std::unique_ptr<hbc::BCProvider> bytecode);
#endif

  /// Initialize modules created with \p createUninitialized.
  /// \param bytecode the bytecode data to initialize it with.
  void initialize(std::shared_ptr<hbc::BCProvider> &&bytecode);

  /// Creates a RuntimeModule who will be managed manually, and after creation
  /// is guaranteed to have a user.
  /// If the number of users goes to zero it will be destroyed, and if not it
  /// will be destroyed when the Runtime is destroyed.
  static RuntimeModule *createManual(
      Runtime *runtime,
      std::shared_ptr<hbc::BCProvider> &&bytecode = nullptr,
      RuntimeModuleFlags flags = {});

  /// Prepares this RuntimeModule for the systematic destruction of all modules.
  /// Normal destruction is reference counted, but when the Runtime shuts down,
  /// we ignore that count and delete all in an arbitrary order.
  void prepareForRuntimeShutdown();

  /// For opcodes that use a stringID as identifier explicitly, we know that
  /// the compiler would have marked the stringID as identifier, and hence
  /// we should have created the symbol during identifier table initialization.
  /// The symbol must already exist in the map. This is a fast path.
  SymbolID getSymbolIDMustExist(StringID stringID) {
    assert(
        stringIDMap_[stringID].isValid() &&
        "Symbol must exist for this string ID");
    return stringIDMap_[stringID];
  }

  /// \return the \c SymbolID for a string by string index. The symbol may not
  /// already exist for this given string ID. Hence we may need to create it
  /// on the fly.
  SymbolID getSymbolIDFromStringID(StringID stringID) {
    SymbolID id = stringIDMap_[stringID];
    if (LLVM_UNLIKELY(!id.isValid())) {
      // Materialize this lazily created symbol.
      auto entry = bcProvider_->getStringTableEntry(stringID);
      assert(
          !entry.isIdentifier() &&
          "Identifier entries should have been created at module load time");
      id = createSymbolFromStringID(stringID, entry, llvm::None);
    }
    assert(id.isValid() && "Failed to create symbol for stringID");
    return id;
  }

  /// Gets the SymbolID and looks it up in the runtime's identifier table.
  /// \return the StringPrimitive for a string by string index.
  StringPrimitive *getStringPrimFromStringID(StringID stringID);

  /// \return the RegExp bytecode for a given regexp ID.
  llvm::ArrayRef<uint8_t> getRegExpBytecodeFromRegExpID(
      uint32_t regExpId) const;

  /// \return the number of functions in the function map.
  uint32_t getNumCodeBlocks() const {
    return functionMap_.size();
  }

  /// \return the CodeBlock for a function by function index.
  inline CodeBlock *getCodeBlock(unsigned index) {
    if (LLVM_LIKELY(functionMap_[index])) {
      return functionMap_[index];
    }
    return getCodeBlockSlowPath(index);
  }

  /// \return whether this RuntimeModule has been initialized.
  bool isInitialized() const {
    return !bcProvider_->isLazy();
  }

  const hbc::BCProvider *getBytecode() const {
    return bcProvider_.get();
  }

  hbc::BCProvider *getBytecode() {
    return bcProvider_.get();
  }

  std::shared_ptr<hbc::BCProvider> getBytecodeSharedPtr() {
    return bcProvider_;
  }

  /// \return a constant reference to the function map.
  const std::vector<CodeBlock *> &getFunctionMap() {
    return functionMap_;
  }

  /// \return the CJS module corresponding to \p filename, nullptr on failure.
  CJSModule *getCJSModule(SymbolID filename) {
    auto it = cjsModuleTable_.find(filename);
    return it != cjsModuleTable_.end() ? &cjsModules_[it->second] : nullptr;
  }

  /// \return the CJS module with ID \p index, nullptr on failure.
  CJSModule *getCJSModule(uint32_t index) {
    return index < cjsModules_.size() ? &cjsModules_[index] : nullptr;
  }

  /// \return the sourceURL, or an empty string if none.
  llvm::StringRef getSourceURL() const {
    return sourceURL_;
  }

  /// \return whether this module hides its epilogue from
  /// Runtime.getEpilogues().
  bool hidesEpilogue() const {
    return flags_.hidesEpilogue;
  }

  /// \return any trailing data after the real bytecode.
  llvm::ArrayRef<uint8_t> getEpilogue() const {
    return bcProvider_->getEpilogue();
  }

  /// Called when a new JSFunction is created whose code block points to this.
  void addUser() {
    refCount_++;
  }

  /// Called when a JSFunction that uses this is destroyed.
  /// If refCount_ becomes 0 after removal, this runtime module is deleted.
  void removeUser() {
    assert(refCount_ && "Negative refCount_ in RuntimeModule");
    refCount_--;
    if (refCount_ == 0 && !flags_.persistent) {
      // We don't free persistent modules even when ref count is 0.
      delete this;
    }
  }

  /// Mark the non-weak roots owned by this RuntimeModule.
  void markRoots(SlotAcceptor &acceptor, bool markLongLived);

  /// Mark the weak roots owned by this RuntimeModule.
  void markWeakRoots(SlotAcceptor &acceptor);

  /// \return an estimate of the size of additional memory used by this
  /// RuntimeModule.
  size_t additionalMemorySize() const;

  /// Find the cached hidden class for an object literal, if one exists.
  /// \param keyBufferIndex value of NewObjectWithBuffer instruction.
  /// \param numLiterals number of literals used from key buffer of
  /// NewObjectWithBuffer instruction.
  /// \return the cached hidden class.
  llvm::Optional<Handle<HiddenClass>> findCachedLiteralHiddenClass(
      unsigned keyBufferIndex,
      unsigned numLiterals) const;

  /// Try to cache the sharable hidden class for object literal. Cache will
  /// be skipped if keyBufferIndex is >= 2^24.
  /// \param keyBufferIndex value of NewObjectWithBuffer instruction.
  /// \param clazz the hidden class to cache.
  void tryCacheLiteralHiddenClass(unsigned keyBufferIndex, HiddenClass *clazz);

 private:
  /// Import the string table from the supplied module.
  void importStringIDMap();

  /// Initialize functionMap_, without actually creating the code blocks.
  /// They will be created lazily when needed.
  void initializeFunctionMap();

  // /// Import the CommonJS module table.
  // /// Set every module to uninitialized, except for the first module.
  void importCJSModuleTable();

  /// Map the supplied string to a given \p stringID, register it in the
  /// identifier table, and \return the symbol ID.
  /// Computes the hash of the string when it's not supplied.
  template <typename T>
  SymbolID mapString(llvm::ArrayRef<T> str, StringID stringID) {
    return mapString(str, stringID, hermes::hashString(str));
  }

  /// Map the supplied string to a given \p stringID, register it in the
  /// identifier table, and \return the symbol ID.
  template <typename T>
  SymbolID mapString(llvm::ArrayRef<T> str, StringID stringID, uint32_t hash);

  /// Create a symbol from a given \p stringID, which is an index to the
  /// string table, corresponding to the entry \p entry. If \p mhash is not
  /// None, use it as the hash; otherwise compute the hash from the string
  /// contents. \return the created symbol ID.
  SymbolID createSymbolFromStringID(
      StringID stringID,
      const StringTableEntry &entry,
      OptValue<uint32_t> mhash);

  /// \return a unqiue hash key for object literal hidden class cache.
  /// \param keyBufferIndex value of NewObjectWithBuffer instruction(must be
  /// less than 2^24).
  /// \param numLiterals number of literals used from key buffer of
  /// NewObjectWithBuffer instruction(must be less than 256).
  static uint32_t getLiteralHiddenClassCacheHashKey(
      unsigned keyBufferIndex,
      unsigned numLiterals) {
    assert(
        canGenerateLiteralHiddenClassCacheKey(keyBufferIndex, numLiterals) &&
        "<keyBufferIndex, numLiterals> tuple can't be used as cache key.");
    return ((uint32_t)keyBufferIndex << 8) | numLiterals;
  }

  /// \return whether tuple <keyBufferIndex, numLiterals> can generate a
  /// hidden class literal cache hash key or not.
  /// \param keyBufferIndex value of NewObjectWithBuffer instruction; it must
  /// be less than 2^24 to be used as a cache key.
  /// \param keyBufferIndex value of NewObjectWithBuffer instruction. it must
  /// be less than 256 to be used as a cache key.
  static bool canGenerateLiteralHiddenClassCacheKey(
      uint32_t keyBufferIndex,
      unsigned numLiterals) {
    return (keyBufferIndex & 0xFF000000) == 0 && numLiterals < 256;
  }
};

using RuntimeModuleList = llvm::simple_ilist<RuntimeModule>;

} // namespace vm
} // namespace hermes

#endif
