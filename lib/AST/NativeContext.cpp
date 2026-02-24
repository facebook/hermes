/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/NativeContext.h"

#include "llvh/ADT/Hashing.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {

static const char *nativeCTypeNames[] = {
#define NATIVE_VOID(name, cstr) cstr,
#define NATIVE_HV(name, cstr) cstr,
#define NATIVE_TYPE(name, cstr) cstr,
#include "hermes/AST/NativeTypes.def"
};

const char *nativeCTypeName(NativeCType t) {
  assert((unsigned)t < (unsigned)NativeCType::_last && "Invalid NativeCType");
  return nativeCTypeNames[(unsigned)t];
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, NativeCType t) {
  return OS << nativeCTypeName(t);
}

int NativeSignature::compare(const NativeSignature *other) const {
  if (other == this)
    return 0;
  if (result_ != other->result_) {
    int tmp = (int)result_ - (int)other->result_;
    return (int)(tmp > 0) - (int)(tmp < 0);
  }
  if (params_.size() != other->params_.size()) {
    return params_.size() < other->params_.size() ? -1
        : params_.size() == other->params_.size() ? 0
                                                  : 1;
  }
  for (size_t i = 0, e = params_.size(); i != e; ++i) {
    if (params_[i] != other->params_[i]) {
      int tmp = (int)params_[i] - (int)other->params_[i];
      return (int)(tmp > 0) - (int)(tmp < 0);
    }
  }
  return 0;
}

unsigned NativeSignature::hash() const {
  return llvh::hash_combine(
      result_,
      params_.size(),
      llvh::hash_combine_range(params_.begin(), params_.end()));
}

/// Generate a string representation of the signature.
llvh::raw_ostream &NativeSignature::format(
    llvh::raw_ostream &OS,
    const char *name) const {
  OS << result_ << ' ';
  if (name)
    OS << name;
  else
    OS << "(*)";
  OS << '(';
  if (params_.empty()) {
    OS << "void";
  } else {
    bool first = true;
    for (auto pt : params_) {
      if (first)
        first = false;
      else
        OS << ", ";
      OS << pt;
    }
  }
  OS << ')';
  return OS;
}

void NativeSignature::Profile(
    llvh::FoldingSetNodeID &ID,
    NativeCType result,
    llvh::ArrayRef<NativeCType> params) {
  ID.AddInteger((uint8_t)result);
  for (auto pt : params)
    ID.AddInteger((uint8_t)pt);
}

void NativeSignature::Profile(llvh::FoldingSetNodeID &ID) const {
  Profile(ID, result_, params_);
}

/// A helper to normalize the contents of \c NativeSettings, specifically the
/// target triple.
static NativeSettings &&normalizeNativeSettings(NativeSettings &&ns) {
  if (ns.targetTriple.str().empty())
    ns.targetTriple = llvh::Triple(llvh::sys::getProcessTriple());
  else
    ns.targetTriple = llvh::Triple(ns.targetTriple.normalize());
  return std::move(ns);
}

NativeContext::NativeContext(NativeSettings ns)
    : settings(normalizeNativeSettings(std::move(ns))),
      md(machineDescFromTriple(settings.targetTriple)) {}

NativeContext::~NativeContext() = default;

NativeSignature *NativeContext::getSignature(
    NativeCType result,
    llvh::ArrayRef<NativeCType> params) {
  return signatures_.getOrEmplace(result, params).first;
}

NativeExtern *NativeContext::getExtern(
    UniqueString *name,
    NativeSignature *signature,
    llvh::SMLoc loc,
    bool declared,
    UniqueString *include) {
  llvh::FoldingSetNodeID ID;
  NativeExtern::Profile(ID, name);
  void *insertPos;
  if (auto *ext = externMap_.FindNodeOrInsertPos(ID, insertPos)) {
    if (declared)
      ext->setDeclared(true);
    if (include)
      ext->setInclude(include);
    return ext;
  }
  auto *res = externMap_.InsertNode(
      std::make_unique<NativeExtern>(name, signature, loc, declared, include),
      insertPos);
  externVector_.push_back(res);
  return res;
}

NativeExtern *NativeContext::getExistingExtern(UniqueString *name) {
  llvh::FoldingSetNodeID ID;
  NativeExtern::Profile(ID, name);
  void *insertPos;
  auto *ext = externMap_.FindNodeOrInsertPos(ID, insertPos);
  assert(ext && "extern which must exist not found");
  if (LLVM_UNLIKELY(!ext))
    abort();
  return ext;
}

using Cls = MachineDesc::Category;

/// LP64: long and ptr are 64-bit. Char is signed. Types aligned to sizeof.
static constexpr MachineDesc s_LP64 = {
    .mtype =
        {
            // i8
            {.size = 1, .align = 1, .cat = Cls::Int, .sign = true},
            // u8
            {.size = 1, .align = 1, .cat = Cls::Int, .sign = false},
            // i16
            {.size = 2, .align = 2, .cat = Cls::Int, .sign = true},
            // u16
            {.size = 2, .align = 2, .cat = Cls::Int, .sign = false},
            // i32
            {.size = 4, .align = 4, .cat = Cls::Int, .sign = true},
            // u32
            {.size = 4, .align = 4, .cat = Cls::Int, .sign = false},
            // i64
            {.size = 8, .align = 8, .cat = Cls::Int, .sign = true},
            // u64
            {.size = 8, .align = 8, .cat = Cls::Int, .sign = false},
            // f32
            {.size = 4, .align = 4, .cat = Cls::FP, .sign = true},
            // f64
            {.size = 8, .align = 8, .cat = Cls::FP, .sign = true},
            // ptr
            {.size = 8, .align = 8, .cat = Cls::Ptr, .sign = false},
        },
    .ctype2m =
        {
            // i8
            MachineType::i8,
            // u8
            MachineType::u8,
            // i16
            MachineType::i16,
            // u16
            MachineType::u16,
            // i32
            MachineType::i32,
            // u32
            MachineType::u32,
            // i64
            MachineType::i64,
            // u64
            MachineType::u64,
            // f32
            MachineType::f32,
            // f64
            MachineType::f64,
            // ptr
            MachineType::ptr,
            // bool
            MachineType::u8,
            // char
            MachineType::i8,
            // schar
            MachineType::i8,
            // uchar
            MachineType::u8,
            // short
            MachineType::i16,
            // ushort
            MachineType::u16,
            // int
            MachineType::i32,
            // uint
            MachineType::u32,
            // long
            MachineType::i64,
            // ulong
            MachineType::u64,
            // longlong
            MachineType::i64,
            // ulonglong
            MachineType::u64,
            // intptr
            MachineType::i64,
            // uintptr
            MachineType::u64,
            // ptrdiff_t
            MachineType::i64,
            // size_t
            MachineType::u64,
            // ssize_t
            MachineType::i64,
            // float
            MachineType::f32,
            // double
            MachineType::f64,
            // SHLegacyValue
            MachineType::u64,
            // void (just use anything)
            MachineType::i8,
        }
    //
};

/// ILP32: long and ptr are 32-bit. Char is signed. Types aligned to
/// min(sizeof, 4).
static constexpr MachineDesc s_ILP32 = {
    .mtype =
        {
            // i8
            {.size = 1, .align = 1, .cat = Cls::Int, .sign = true},
            // u8
            {.size = 1, .align = 1, .cat = Cls::Int, .sign = false},
            // i16
            {.size = 2, .align = 2, .cat = Cls::Int, .sign = true},
            // u16
            {.size = 2, .align = 2, .cat = Cls::Int, .sign = false},
            // i32
            {.size = 4, .align = 4, .cat = Cls::Int, .sign = true},
            // u32
            {.size = 4, .align = 4, .cat = Cls::Int, .sign = false},
            // i64
            {.size = 8, .align = 4, .cat = Cls::Int, .sign = true},
            // u64
            {.size = 8, .align = 4, .cat = Cls::Int, .sign = false},
            // f32
            {.size = 4, .align = 4, .cat = Cls::FP, .sign = true},
            // f64
            {.size = 8, .align = 4, .cat = Cls::FP, .sign = true},
            // ptr
            {.size = 4, .align = 4, .cat = Cls::Ptr, .sign = false},
        },
    .ctype2m =
        {
            // i8
            MachineType::i8,
            // u8
            MachineType::u8,
            // i16
            MachineType::i16,
            // u16
            MachineType::u16,
            // i32
            MachineType::i32,
            // u32
            MachineType::u32,
            // i64
            MachineType::i64,
            // u64
            MachineType::u64,
            // f32
            MachineType::f32,
            // f64
            MachineType::f64,
            // ptr
            MachineType::ptr,
            // bool
            MachineType::u8,
            // char
            MachineType::i8,
            // schar
            MachineType::i8,
            // uchar
            MachineType::u8,
            // short
            MachineType::i16,
            // ushort
            MachineType::u16,
            // int
            MachineType::i32,
            // uint
            MachineType::u32,
            // long
            MachineType::i32,
            // ulong
            MachineType::u32,
            // longlong
            MachineType::i64,
            // ulonglong
            MachineType::u64,
            // intptr
            MachineType::i32,
            // uintptr
            MachineType::u32,
            // ptrdiff_t
            MachineType::i32,
            // size_t
            MachineType::u32,
            // ssize_t
            MachineType::i32,
            // float
            MachineType::f32,
            // double
            MachineType::f64,
            // SHLegacyValue
            MachineType::u64,
            // void (just use anything)
            MachineType::i8,
        }
    //
};

static bool isSignedCharDefault(const llvh::Triple &Triple) {
  switch (Triple.getArch()) {
    default:
      return true;

    case llvh::Triple::aarch64:
      //    case llvh::Triple::aarch64_32:
    case llvh::Triple::aarch64_be:
    case llvh::Triple::arm:
    case llvh::Triple::armeb:
    case llvh::Triple::thumb:
    case llvh::Triple::thumbeb:
      if (Triple.isOSDarwin() || Triple.isOSWindows())
        return true;
      return false;

    case llvh::Triple::ppc:
    case llvh::Triple::ppc64:
      if (Triple.isOSDarwin())
        return true;
      return false;

    case llvh::Triple::hexagon:
      //    case llvh::Triple::ppcle:
    case llvh::Triple::ppc64le:
    case llvh::Triple::riscv32:
    case llvh::Triple::riscv64:
    case llvh::Triple::systemz:
    case llvh::Triple::xcore:
      return false;
  }
}

MachineDesc machineDescFromTriple(const llvh::Triple &triple) {
  const bool is64 = triple.isArch64Bit();
  MachineDesc md = is64 ? s_LP64 : s_ILP32;
  if (!isSignedCharDefault(triple))
    md.ctype2m[(unsigned)NativeCType::c_char] = MachineType::u8;
  if (is64) {
    // Check for LLP64.
    if (triple.isOSWindows()) {
      // Make long 32-bit.
      md.ctype2m[(unsigned)NativeCType::c_long] = MachineType::i32;
      md.ctype2m[(unsigned)NativeCType::c_ulong] = MachineType::u32;
    }
  } else {
    // On ARM, 8-byte values are aligned to 8.
    if (triple.isARM() || triple.isThumb()) {
      md.mtype[(unsigned)MachineType::i64].align = 8;
      md.mtype[(unsigned)MachineType::u64].align = 8;
      md.mtype[(unsigned)MachineType::f64].align = 8;
    }
  }
  return md;
}

} // namespace hermes
