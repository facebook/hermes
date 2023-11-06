/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// NativeContext is an extension of Context that holds information about
/// native types, uniqued native function signatures, and native externs.
///
/// It is separate class to keep contextual separation about the AST-related
/// data held by Context and to avoid adding dependencies to the AST.
///
/// NativeContext is used by the type checker and by the backend.
//===----------------------------------------------------------------------===//

#ifndef HERMES_AST_NATIVECONTEXT_H
#define HERMES_AST_NATIVECONTEXT_H

#include "hermes/ADT/OwningFoldingSet.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/Triple.h"
#include "llvh/Support/SMLoc.h"

namespace llvh {
class raw_ostream;
}

namespace hermes {

class UniqueString;

/// The set of actual machine types with known sizes like 16-bit int, etc.
/// Native C types map to these machine types depending on the target. For
/// example on a 32-bit target, long map may to int32, while on a 64-bit target
/// it may map to int64. (Note that in practice there are more factors in play,
/// like the OS ABI, the compiler, etc.)
enum class MachineType : uint8_t {
#define NATIVE_TYPE(name, cstr) name,
#define NATIVE_CTYPE(name, cstr)
#include "NativeTypes.def"
  _last,
};

/// The set of native types that can be used in native signatures.
enum class NativeCType : uint8_t {
#define NATIVE_VOID(name, cstr) c_##name,
#define NATIVE_TYPE(name, cstr) name,
#define NATIVE_CTYPE(name, cstr) c_##name,
#include "NativeTypes.def"
  _last,
};

/// Return the string representation of a NativeCType.
const char *nativeCTypeName(NativeCType t);

/// Format a NativeCType into a stream.
llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, NativeCType t);

/// Description of the native arch we are compiling for.
struct MachineDesc {
  /// The machine type category: integer, floating point or pointer.
  enum class Category : uint8_t {
    Int,
    FP,
    Ptr,
  };

  /// Descriptor of a machine type. Its size, alignment, category and whether
  /// it is signed.
  struct MTD {
    uint8_t size;
    uint8_t align;
    Category cat;
    bool sign;
  };

  /// Description of every machine type.
  MTD mtype[(unsigned)MachineType::_last];
  /// Mapping of C types to machine types.
  MachineType ctype2m[(unsigned)NativeCType::_last];

  /// \return the descriptor of the specified machine type.
  inline const MTD &getMTD(MachineType mt) const {
    assert(mt < MachineType::_last && "Invalid machine type");
    return mtype[(unsigned)mt];
  }

  /// Map a C type to the corresponding machine type.
  inline MachineType mapCType(NativeCType ctype) const {
    assert(ctype < NativeCType::_last && "Invalid ctype");
    return ctype2m[(unsigned)ctype];
  }
};

/// Return a MachineDesc from an LLVM triple.
MachineDesc machineDescFromTriple(const llvh::Triple &triple);

/// Description of the target we are compiling for. Machine architecture and
/// StaticH model.
/// NOTE: this is not used yet, but will be very soon.
struct TargetDescription {
  /// Whether a 64-bit target uses 32-bit pointers.
  bool compressedPointers;
  /// Whether the target boxes doubles in the heap instead of storing them
  /// inline.
  bool boxedDoubles;
  /// Whether the target has a contiguous heap. This affects decoding
  bool contiguousHeap;

  /// The machine descriptor with type sizes and mapping.
  MachineDesc md;
};

/// A native function signature.
class NativeSignature : public llvh::FoldingSetNode {
  /// The type of the result.
  NativeCType result_;
  /// The type of the parameters. Note that void is not allowed here.
  llvh::SmallVector<NativeCType, 4> params_;
  /// Whether the function should be passed the runtime as the first argument
  /// (which must be a pointer).
  bool passRuntime_;

 public:
  explicit NativeSignature(
      NativeCType result,
      llvh::ArrayRef<NativeCType> params,
      bool passRuntime)
      : result_(result),
        params_(params.begin(), params.end()),
        passRuntime_(passRuntime) {}

  NativeCType result() const {
    return result_;
  }

  llvh::ArrayRef<NativeCType> params() const {
    return params_;
  }

  /// \return whether the function should be passed the runtime as the first
  ///     argument.
  bool passRuntime() const {
    return passRuntime_;
  }

  /// Compare two signatures and return -1, 0 or +1. Note that the ordering is
  /// arbitrary, but consistent.
  int compare(const NativeSignature *other) const;
  /// Return a hash of the signature.
  unsigned hash() const;

  /// Format the signature to a stream, optionally with a name. The result is
  /// valid C.
  llvh::raw_ostream &format(llvh::raw_ostream &OS, const char *name = nullptr)
      const;

  static void Profile(
      llvh::FoldingSetNodeID &ID,
      NativeCType result,
      llvh::ArrayRef<NativeCType> params,
      bool passRuntime);

  void Profile(llvh::FoldingSetNodeID &ID) const;
};

inline llvh::raw_ostream &operator<<(
    llvh::raw_ostream &OS,
    const NativeSignature &ns) {
  return ns.format(OS);
}

/// An external native function declaration. External functions are keyed by
/// name only.
class NativeExtern : public llvh::FoldingSetNode {
  /// The name of the external function.
  UniqueString *name_;
  /// The signature of the external function.
  NativeSignature *signature_;
  /// Optional declaration location.
  llvh::SMLoc loc_;
  /// Whether the function is already in the emitted C source in system headers
  /// and such.
  bool declared_;
  /// Optional include file name.
  UniqueString *include_;
  /// Whether the function should be passed the runtime as the first argument.
  bool passRuntime_;

 public:
  /// \param declared whether the function is already declared in the emitted C
  ///     source.
  /// \param include optional the name of the file to include to declare the
  ///     function.
  NativeExtern(
      UniqueString *name,
      NativeSignature *signature,
      llvh::SMLoc loc,
      bool declared,
      UniqueString *include,
      bool passRuntime)
      : name_(name),
        signature_(signature),
        loc_(loc),
        declared_(declared),
        include_(include),
        passRuntime_(passRuntime) {}

  UniqueString *name() const {
    return name_;
  }

  NativeSignature *signature() const {
    return signature_;
  }

  llvh::SMLoc loc() const {
    return loc_;
  }

  bool declared() const {
    return declared_;
  }

  void setDeclared(bool declared) {
    declared_ = declared;
  }

  UniqueString *include() const {
    return include_;
  }

  void setInclude(UniqueString *include) {
    include_ = include;
  }

  bool passRuntime() const {
    return passRuntime_;
  }

  void setPassRuntime(bool passRuntime) {
    passRuntime_ = passRuntime;
  }

  static void Profile(llvh::FoldingSetNodeID &ID, UniqueString *name) {
    ID.AddPointer(name);
  }

  void Profile(llvh::FoldingSetNodeID &ID) const {
    Profile(ID, name_);
  }
};

struct NativeSettings {
  llvh::Triple targetTriple{};

  /// Emit the Static Hermes native stack check.
  bool emitCheckNativeStack = true;
};

/// Holder for information related to native compilation, deliberately kept
/// separate from \c hermes::Context.
class NativeContext {
  OwningFoldingSet<NativeSignature> signatures_{};
  /// Native externs keyed by name.
  OwningFoldingSet<NativeExtern> externMap_{};
  /// Native externs in insertion order.
  std::vector<NativeExtern *> externVector_{};

 public:
  const NativeSettings settings;
  const MachineDesc md;

  NativeContext(NativeSettings ns);
  ~NativeContext();

  /// Get a uniqued signature with the specified result and parameters.
  NativeSignature *getSignature(
      NativeCType result,
      llvh::ArrayRef<NativeCType> params,
      bool passRuntime);

  /// Get a uniqued version of this extern by name. If it already exists,
  /// there is no guarantee that the signature of the returned extern matches
  /// the supplied signature.
  ///
  /// \param declared whether the function is already declared in the emitted C
  ///     source.
  /// \param include optional the name of the file to include to declare the
  ///     function.
  /// \param passRuntime whether the function should be passed the runtime as
  ///     the first argument.
  NativeExtern *getExtern(
      UniqueString *name,
      NativeSignature *signature,
      llvh::SMLoc loc,
      bool declared,
      UniqueString *include,
      bool passRuntime);

  /// Obtain an extern, which must exist, by name.
  /// \return a non-null result.
  NativeExtern *getExistingExtern(UniqueString *name);

  /// Return the vector of externs in insertion order.
  const llvh::ArrayRef<NativeExtern *> getAllExterns() const {
    return externVector_;
  }
};

} // namespace hermes

#endif
