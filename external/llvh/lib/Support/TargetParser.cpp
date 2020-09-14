//===-- TargetParser - Parser for target features ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise hardware features such as
// FPU/CPU/ARCH names as well as specific support such as HDIV, etc.
//
//===----------------------------------------------------------------------===//

#include "llvh/Support/ARMBuildAttributes.h"
#include "llvh/Support/TargetParser.h"
#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/StringSwitch.h"
#include "llvh/ADT/Twine.h"
#include <cctype>

using namespace llvh;
using namespace ARM;
using namespace AArch64;
using namespace AMDGPU;

namespace {

// List of canonical FPU names (use getFPUSynonym) and which architectural
// features they correspond to (use getFPUFeatures).
// FIXME: TableGen this.
// The entries must appear in the order listed in ARM::FPUKind for correct indexing
static const struct {
  const char *NameCStr;
  size_t NameLength;
  ARM::FPUKind ID;
  ARM::FPUVersion FPUVersion;
  ARM::NeonSupportLevel NeonSupport;
  ARM::FPURestriction Restriction;

  StringRef getName() const { return StringRef(NameCStr, NameLength); }
} FPUNames[] = {
#define ARM_FPU(NAME, KIND, VERSION, NEON_SUPPORT, RESTRICTION) \
  { NAME, sizeof(NAME) - 1, KIND, VERSION, NEON_SUPPORT, RESTRICTION },
#include "llvh/Support/ARMTargetParser.def"
};

// List of canonical arch names (use getArchSynonym).
// This table also provides the build attribute fields for CPU arch
// and Arch ID, according to the Addenda to the ARM ABI, chapters
// 2.4 and 2.3.5.2 respectively.
// FIXME: SubArch values were simplified to fit into the expectations
// of the triples and are not conforming with their official names.
// Check to see if the expectation should be changed.
// FIXME: TableGen this.
template <typename T> struct ArchNames {
  const char *NameCStr;
  size_t NameLength;
  const char *CPUAttrCStr;
  size_t CPUAttrLength;
  const char *SubArchCStr;
  size_t SubArchLength;
  unsigned DefaultFPU;
  unsigned ArchBaseExtensions;
  T ID;
  ARMBuildAttrs::CPUArch ArchAttr; // Arch ID in build attributes.

  StringRef getName() const { return StringRef(NameCStr, NameLength); }

  // CPU class in build attributes.
  StringRef getCPUAttr() const { return StringRef(CPUAttrCStr, CPUAttrLength); }

  // Sub-Arch name.
  StringRef getSubArch() const { return StringRef(SubArchCStr, SubArchLength); }
};
ArchNames<ARM::ArchKind> ARCHNames[] = {
#define ARM_ARCH(NAME, ID, CPU_ATTR, SUB_ARCH, ARCH_ATTR, ARCH_FPU, ARCH_BASE_EXT)       \
  {NAME, sizeof(NAME) - 1, CPU_ATTR, sizeof(CPU_ATTR) - 1, SUB_ARCH,       \
   sizeof(SUB_ARCH) - 1, ARCH_FPU, ARCH_BASE_EXT, ARM::ArchKind::ID, ARCH_ATTR},
#include "llvh/Support/ARMTargetParser.def"
};

ArchNames<AArch64::ArchKind> AArch64ARCHNames[] = {
 #define AARCH64_ARCH(NAME, ID, CPU_ATTR, SUB_ARCH, ARCH_ATTR, ARCH_FPU, ARCH_BASE_EXT)       \
   {NAME, sizeof(NAME) - 1, CPU_ATTR, sizeof(CPU_ATTR) - 1, SUB_ARCH,       \
    sizeof(SUB_ARCH) - 1, ARCH_FPU, ARCH_BASE_EXT, AArch64::ArchKind::ID, ARCH_ATTR},
 #include "llvh/Support/AArch64TargetParser.def"
 };


// List of Arch Extension names.
// FIXME: TableGen this.
static const struct {
  const char *NameCStr;
  size_t NameLength;
  unsigned ID;
  const char *Feature;
  const char *NegFeature;

  StringRef getName() const { return StringRef(NameCStr, NameLength); }
} ARCHExtNames[] = {
#define ARM_ARCH_EXT_NAME(NAME, ID, FEATURE, NEGFEATURE) \
  { NAME, sizeof(NAME) - 1, ID, FEATURE, NEGFEATURE },
#include "llvh/Support/ARMTargetParser.def"
},AArch64ARCHExtNames[] = {
#define AARCH64_ARCH_EXT_NAME(NAME, ID, FEATURE, NEGFEATURE) \
  { NAME, sizeof(NAME) - 1, ID, FEATURE, NEGFEATURE },
#include "llvh/Support/AArch64TargetParser.def"
};

// List of HWDiv names (use getHWDivSynonym) and which architectural
// features they correspond to (use getHWDivFeatures).
// FIXME: TableGen this.
static const struct {
  const char *NameCStr;
  size_t NameLength;
  unsigned ID;

  StringRef getName() const { return StringRef(NameCStr, NameLength); }
} HWDivNames[] = {
#define ARM_HW_DIV_NAME(NAME, ID) { NAME, sizeof(NAME) - 1, ID },
#include "llvh/Support/ARMTargetParser.def"
};

// List of CPU names and their arches.
// The same CPU can have multiple arches and can be default on multiple arches.
// When finding the Arch for a CPU, first-found prevails. Sort them accordingly.
// When this becomes table-generated, we'd probably need two tables.
// FIXME: TableGen this.
template <typename T> struct CpuNames {
  const char *NameCStr;
  size_t NameLength;
  T ArchID;
  bool Default; // is $Name the default CPU for $ArchID ?
  unsigned DefaultExtensions;

  StringRef getName() const { return StringRef(NameCStr, NameLength); }
};
CpuNames<ARM::ArchKind> CPUNames[] = {
#define ARM_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT) \
  { NAME, sizeof(NAME) - 1, ARM::ArchKind::ID, IS_DEFAULT, DEFAULT_EXT },
#include "llvh/Support/ARMTargetParser.def"
};

CpuNames<AArch64::ArchKind> AArch64CPUNames[] = {
 #define AARCH64_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT) \
   { NAME, sizeof(NAME) - 1, AArch64::ArchKind::ID, IS_DEFAULT, DEFAULT_EXT },
 #include "llvh/Support/AArch64TargetParser.def"
 };

} // namespace

// ======================================================= //
// Information by ID
// ======================================================= //

StringRef ARM::getFPUName(unsigned FPUKind) {
  if (FPUKind >= ARM::FK_LAST)
    return StringRef();
  return FPUNames[FPUKind].getName();
}

FPUVersion ARM::getFPUVersion(unsigned FPUKind) {
  if (FPUKind >= ARM::FK_LAST)
    return FPUVersion::NONE;
  return FPUNames[FPUKind].FPUVersion;
}

ARM::NeonSupportLevel ARM::getFPUNeonSupportLevel(unsigned FPUKind) {
  if (FPUKind >= ARM::FK_LAST)
    return ARM::NeonSupportLevel::None;
  return FPUNames[FPUKind].NeonSupport;
}

ARM::FPURestriction ARM::getFPURestriction(unsigned FPUKind) {
  if (FPUKind >= ARM::FK_LAST)
    return ARM::FPURestriction::None;
  return FPUNames[FPUKind].Restriction;
}

unsigned llvh::ARM::getDefaultFPU(StringRef CPU, ArchKind AK) {
  if (CPU == "generic")
    return ARCHNames[static_cast<unsigned>(AK)].DefaultFPU;

  return StringSwitch<unsigned>(CPU)
#define ARM_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT) \
    .Case(NAME, DEFAULT_FPU)
#include "llvh/Support/ARMTargetParser.def"
    .Default(ARM::FK_INVALID);
}

unsigned llvh::ARM::getDefaultExtensions(StringRef CPU, ArchKind AK) {
  if (CPU == "generic")
    return ARCHNames[static_cast<unsigned>(AK)].ArchBaseExtensions;

  return StringSwitch<unsigned>(CPU)
#define ARM_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT) \
    .Case(NAME, ARCHNames[static_cast<unsigned>(ARM::ArchKind::ID)]\
            .ArchBaseExtensions | DEFAULT_EXT)
#include "llvh/Support/ARMTargetParser.def"
    .Default(ARM::AEK_INVALID);
}

bool llvh::ARM::getHWDivFeatures(unsigned HWDivKind,
                                 std::vector<StringRef> &Features) {

  if (HWDivKind == ARM::AEK_INVALID)
    return false;

  if (HWDivKind & ARM::AEK_HWDIVARM)
    Features.push_back("+hwdiv-arm");
  else
    Features.push_back("-hwdiv-arm");

  if (HWDivKind & ARM::AEK_HWDIVTHUMB)
    Features.push_back("+hwdiv");
  else
    Features.push_back("-hwdiv");

  return true;
}

bool llvh::ARM::getExtensionFeatures(unsigned Extensions,
                                     std::vector<StringRef> &Features) {

  if (Extensions == ARM::AEK_INVALID)
    return false;

  if (Extensions & ARM::AEK_CRC)
    Features.push_back("+crc");
  else
    Features.push_back("-crc");

  if (Extensions & ARM::AEK_DSP)
    Features.push_back("+dsp");
  else
    Features.push_back("-dsp");

  if (Extensions & ARM::AEK_FP16FML)
    Features.push_back("+fp16fml");
  else
    Features.push_back("-fp16fml");

  if (Extensions & ARM::AEK_RAS)
    Features.push_back("+ras");
  else
    Features.push_back("-ras");

  if (Extensions & ARM::AEK_DOTPROD)
    Features.push_back("+dotprod");
  else
    Features.push_back("-dotprod");

  return getHWDivFeatures(Extensions, Features);
}

bool llvh::ARM::getFPUFeatures(unsigned FPUKind,
                               std::vector<StringRef> &Features) {

  if (FPUKind >= ARM::FK_LAST || FPUKind == ARM::FK_INVALID)
    return false;

  // fp-only-sp and d16 subtarget features are independent of each other, so we
  // must enable/disable both.
  switch (FPUNames[FPUKind].Restriction) {
  case ARM::FPURestriction::SP_D16:
    Features.push_back("+fp-only-sp");
    Features.push_back("+d16");
    break;
  case ARM::FPURestriction::D16:
    Features.push_back("-fp-only-sp");
    Features.push_back("+d16");
    break;
  case ARM::FPURestriction::None:
    Features.push_back("-fp-only-sp");
    Features.push_back("-d16");
    break;
  }

  // FPU version subtarget features are inclusive of lower-numbered ones, so
  // enable the one corresponding to this version and disable all that are
  // higher. We also have to make sure to disable fp16 when vfp4 is disabled,
  // as +vfp4 implies +fp16 but -vfp4 does not imply -fp16.
  switch (FPUNames[FPUKind].FPUVersion) {
  case ARM::FPUVersion::VFPV5:
    Features.push_back("+fp-armv8");
    break;
  case ARM::FPUVersion::VFPV4:
    Features.push_back("+vfp4");
    Features.push_back("-fp-armv8");
    break;
  case ARM::FPUVersion::VFPV3_FP16:
    Features.push_back("+vfp3");
    Features.push_back("+fp16");
    Features.push_back("-vfp4");
    Features.push_back("-fp-armv8");
    break;
  case ARM::FPUVersion::VFPV3:
    Features.push_back("+vfp3");
    Features.push_back("-fp16");
    Features.push_back("-vfp4");
    Features.push_back("-fp-armv8");
    break;
  case ARM::FPUVersion::VFPV2:
    Features.push_back("+vfp2");
    Features.push_back("-vfp3");
    Features.push_back("-fp16");
    Features.push_back("-vfp4");
    Features.push_back("-fp-armv8");
    break;
  case ARM::FPUVersion::NONE:
    Features.push_back("-vfp2");
    Features.push_back("-vfp3");
    Features.push_back("-fp16");
    Features.push_back("-vfp4");
    Features.push_back("-fp-armv8");
    break;
  }

  // crypto includes neon, so we handle this similarly to FPU version.
  switch (FPUNames[FPUKind].NeonSupport) {
  case ARM::NeonSupportLevel::Crypto:
    Features.push_back("+neon");
    Features.push_back("+crypto");
    break;
  case ARM::NeonSupportLevel::Neon:
    Features.push_back("+neon");
    Features.push_back("-crypto");
    break;
  case ARM::NeonSupportLevel::None:
    Features.push_back("-neon");
    Features.push_back("-crypto");
    break;
  }

  return true;
}

StringRef llvh::ARM::getArchName(ArchKind AK) {
  return ARCHNames[static_cast<unsigned>(AK)].getName();
}

StringRef llvh::ARM::getCPUAttr(ArchKind AK) {
  return ARCHNames[static_cast<unsigned>(AK)].getCPUAttr();
}

StringRef llvh::ARM::getSubArch(ArchKind AK) {
  return ARCHNames[static_cast<unsigned>(AK)].getSubArch();
}

unsigned llvh::ARM::getArchAttr(ArchKind AK) {
  return ARCHNames[static_cast<unsigned>(AK)].ArchAttr;
}

StringRef llvh::ARM::getArchExtName(unsigned ArchExtKind) {
  for (const auto AE : ARCHExtNames) {
    if (ArchExtKind == AE.ID)
      return AE.getName();
  }
  return StringRef();
}

StringRef llvh::ARM::getArchExtFeature(StringRef ArchExt) {
  if (ArchExt.startswith("no")) {
    StringRef ArchExtBase(ArchExt.substr(2));
    for (const auto AE : ARCHExtNames) {
      if (AE.NegFeature && ArchExtBase == AE.getName())
        return StringRef(AE.NegFeature);
    }
  }
  for (const auto AE : ARCHExtNames) {
    if (AE.Feature && ArchExt == AE.getName())
      return StringRef(AE.Feature);
  }

  return StringRef();
}

StringRef llvh::ARM::getHWDivName(unsigned HWDivKind) {
  for (const auto D : HWDivNames) {
    if (HWDivKind == D.ID)
      return D.getName();
  }
  return StringRef();
}

StringRef llvh::ARM::getDefaultCPU(StringRef Arch) {
  ArchKind AK = parseArch(Arch);
  if (AK == ARM::ArchKind::INVALID)
    return StringRef();

  // Look for multiple AKs to find the default for pair AK+Name.
  for (const auto CPU : CPUNames) {
    if (CPU.ArchID == AK && CPU.Default)
      return CPU.getName();
  }

  // If we can't find a default then target the architecture instead
  return "generic";
}

StringRef llvh::AArch64::getFPUName(unsigned FPUKind) {
  return ARM::getFPUName(FPUKind);
}

ARM::FPUVersion AArch64::getFPUVersion(unsigned FPUKind) {
  return ARM::getFPUVersion(FPUKind);
}

ARM::NeonSupportLevel AArch64::getFPUNeonSupportLevel(unsigned FPUKind) {
  return ARM::getFPUNeonSupportLevel( FPUKind);
}

ARM::FPURestriction AArch64::getFPURestriction(unsigned FPUKind) {
  return ARM::getFPURestriction(FPUKind);
}

unsigned llvh::AArch64::getDefaultFPU(StringRef CPU, ArchKind AK) {
  if (CPU == "generic")
    return AArch64ARCHNames[static_cast<unsigned>(AK)].DefaultFPU;

  return StringSwitch<unsigned>(CPU)
#define AARCH64_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT) \
    .Case(NAME, DEFAULT_FPU)
#include "llvh/Support/AArch64TargetParser.def"
    .Default(ARM::FK_INVALID);
}

unsigned llvh::AArch64::getDefaultExtensions(StringRef CPU, ArchKind AK) {
  if (CPU == "generic")
    return AArch64ARCHNames[static_cast<unsigned>(AK)].ArchBaseExtensions;

  return StringSwitch<unsigned>(CPU)
#define AARCH64_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT)       \
  .Case(NAME,                                                                  \
        AArch64ARCHNames[static_cast<unsigned>(AArch64::ArchKind::ID)] \
            .ArchBaseExtensions | \
            DEFAULT_EXT)
#include "llvh/Support/AArch64TargetParser.def"
    .Default(AArch64::AEK_INVALID);
}

AArch64::ArchKind llvh::AArch64::getCPUArchKind(StringRef CPU) {
  if (CPU == "generic")
    return AArch64::ArchKind::ARMV8A;

  return StringSwitch<AArch64::ArchKind>(CPU)
#define AARCH64_CPU_NAME(NAME, ID, DEFAULT_FPU, IS_DEFAULT, DEFAULT_EXT) \
  .Case(NAME, AArch64::ArchKind:: ID)
#include "llvh/Support/AArch64TargetParser.def"
    .Default(AArch64::ArchKind::INVALID);
}

bool llvh::AArch64::getExtensionFeatures(unsigned Extensions,
                                     std::vector<StringRef> &Features) {

  if (Extensions == AArch64::AEK_INVALID)
    return false;

  if (Extensions & AArch64::AEK_FP)
    Features.push_back("+fp-armv8");
  if (Extensions & AArch64::AEK_SIMD)
    Features.push_back("+neon");
  if (Extensions & AArch64::AEK_CRC)
    Features.push_back("+crc");
  if (Extensions & AArch64::AEK_CRYPTO)
    Features.push_back("+crypto");
  if (Extensions & AArch64::AEK_DOTPROD)
    Features.push_back("+dotprod");
  if (Extensions & AArch64::AEK_FP16FML)
    Features.push_back("+fp16fml");
  if (Extensions & AArch64::AEK_FP16)
    Features.push_back("+fullfp16");
  if (Extensions & AArch64::AEK_PROFILE)
    Features.push_back("+spe");
  if (Extensions & AArch64::AEK_RAS)
    Features.push_back("+ras");
  if (Extensions & AArch64::AEK_LSE)
    Features.push_back("+lse");
  if (Extensions & AArch64::AEK_RDM)
    Features.push_back("+rdm");
  if (Extensions & AArch64::AEK_SVE)
    Features.push_back("+sve");
  if (Extensions & AArch64::AEK_RCPC)
    Features.push_back("+rcpc");

  return true;
}

bool llvh::AArch64::getFPUFeatures(unsigned FPUKind,
                               std::vector<StringRef> &Features) {
  return ARM::getFPUFeatures(FPUKind, Features);
}

bool llvh::AArch64::getArchFeatures(AArch64::ArchKind AK,
                                    std::vector<StringRef> &Features) {
  if (AK == AArch64::ArchKind::ARMV8_1A)
    Features.push_back("+v8.1a");
  if (AK == AArch64::ArchKind::ARMV8_2A)
    Features.push_back("+v8.2a");
  if (AK == AArch64::ArchKind::ARMV8_3A)
    Features.push_back("+v8.3a");
  if (AK == AArch64::ArchKind::ARMV8_4A)
    Features.push_back("+v8.4a");
  if (AK == AArch64::ArchKind::ARMV8_5A)
    Features.push_back("+v8.5a");

  return AK != AArch64::ArchKind::INVALID;
}

StringRef llvh::AArch64::getArchName(ArchKind AK) {
  return AArch64ARCHNames[static_cast<unsigned>(AK)].getName();
}

StringRef llvh::AArch64::getCPUAttr(ArchKind AK) {
  return AArch64ARCHNames[static_cast<unsigned>(AK)].getCPUAttr();
}

StringRef llvh::AArch64::getSubArch(ArchKind AK) {
  return AArch64ARCHNames[static_cast<unsigned>(AK)].getSubArch();
}

unsigned llvh::AArch64::getArchAttr(ArchKind AK) {
  return AArch64ARCHNames[static_cast<unsigned>(AK)].ArchAttr;
}

StringRef llvh::AArch64::getArchExtName(unsigned ArchExtKind) {
  for (const auto &AE : AArch64ARCHExtNames)
    if (ArchExtKind == AE.ID)
      return AE.getName();
  return StringRef();
}

StringRef llvh::AArch64::getArchExtFeature(StringRef ArchExt) {
  if (ArchExt.startswith("no")) {
    StringRef ArchExtBase(ArchExt.substr(2));
    for (const auto &AE : AArch64ARCHExtNames) {
      if (AE.NegFeature && ArchExtBase == AE.getName())
        return StringRef(AE.NegFeature);
    }
  }

  for (const auto &AE : AArch64ARCHExtNames)
    if (AE.Feature && ArchExt == AE.getName())
      return StringRef(AE.Feature);
  return StringRef();
}

StringRef llvh::AArch64::getDefaultCPU(StringRef Arch) {
  AArch64::ArchKind AK = parseArch(Arch);
  if (AK == ArchKind::INVALID)
    return StringRef();

  // Look for multiple AKs to find the default for pair AK+Name.
  for (const auto &CPU : AArch64CPUNames)
    if (CPU.ArchID == AK && CPU.Default)
      return CPU.getName();

  // If we can't find a default then target the architecture instead
  return "generic";
}

unsigned llvh::AArch64::checkArchVersion(StringRef Arch) {
  if (Arch.size() >= 2 && Arch[0] == 'v' && std::isdigit(Arch[1]))
    return (Arch[1] - 48);
  return 0;
}

// ======================================================= //
// Parsers
// ======================================================= //

static StringRef getHWDivSynonym(StringRef HWDiv) {
  return StringSwitch<StringRef>(HWDiv)
      .Case("thumb,arm", "arm,thumb")
      .Default(HWDiv);
}

static StringRef getFPUSynonym(StringRef FPU) {
  return StringSwitch<StringRef>(FPU)
      .Cases("fpa", "fpe2", "fpe3", "maverick", "invalid") // Unsupported
      .Case("vfp2", "vfpv2")
      .Case("vfp3", "vfpv3")
      .Case("vfp4", "vfpv4")
      .Case("vfp3-d16", "vfpv3-d16")
      .Case("vfp4-d16", "vfpv4-d16")
      .Cases("fp4-sp-d16", "vfpv4-sp-d16", "fpv4-sp-d16")
      .Cases("fp4-dp-d16", "fpv4-dp-d16", "vfpv4-d16")
      .Case("fp5-sp-d16", "fpv5-sp-d16")
      .Cases("fp5-dp-d16", "fpv5-dp-d16", "fpv5-d16")
      // FIXME: Clang uses it, but it's bogus, since neon defaults to vfpv3.
      .Case("neon-vfpv3", "neon")
      .Default(FPU);
}

static StringRef getArchSynonym(StringRef Arch) {
  return StringSwitch<StringRef>(Arch)
      .Case("v5", "v5t")
      .Case("v5e", "v5te")
      .Case("v6j", "v6")
      .Case("v6hl", "v6k")
      .Cases("v6m", "v6sm", "v6s-m", "v6-m")
      .Cases("v6z", "v6zk", "v6kz")
      .Cases("v7", "v7a", "v7hl", "v7l", "v7-a")
      .Case("v7r", "v7-r")
      .Case("v7m", "v7-m")
      .Case("v7em", "v7e-m")
      .Cases("v8", "v8a", "v8l", "aarch64", "arm64", "v8-a")
      .Case("v8.1a", "v8.1-a")
      .Case("v8.2a", "v8.2-a")
      .Case("v8.3a", "v8.3-a")
      .Case("v8.4a", "v8.4-a")
      .Case("v8.5a", "v8.5-a")
      .Case("v8r", "v8-r")
      .Case("v8m.base", "v8-m.base")
      .Case("v8m.main", "v8-m.main")
      .Default(Arch);
}

// MArch is expected to be of the form (arm|thumb)?(eb)?(v.+)?(eb)?, but
// (iwmmxt|xscale)(eb)? is also permitted. If the former, return
// "v.+", if the latter, return unmodified string, minus 'eb'.
// If invalid, return empty string.
StringRef llvh::ARM::getCanonicalArchName(StringRef Arch) {
  size_t offset = StringRef::npos;
  StringRef A = Arch;
  StringRef Error = "";

  // Begins with "arm" / "thumb", move past it.
  if (A.startswith("arm64"))
    offset = 5;
  else if (A.startswith("arm"))
    offset = 3;
  else if (A.startswith("thumb"))
    offset = 5;
  else if (A.startswith("aarch64")) {
    offset = 7;
    // AArch64 uses "_be", not "eb" suffix.
    if (A.find("eb") != StringRef::npos)
      return Error;
    if (A.substr(offset, 3) == "_be")
      offset += 3;
  }

  // Ex. "armebv7", move past the "eb".
  if (offset != StringRef::npos && A.substr(offset, 2) == "eb")
    offset += 2;
  // Or, if it ends with eb ("armv7eb"), chop it off.
  else if (A.endswith("eb"))
    A = A.substr(0, A.size() - 2);
  // Trim the head
  if (offset != StringRef::npos)
    A = A.substr(offset);

  // Empty string means offset reached the end, which means it's valid.
  if (A.empty())
    return Arch;

  // Only match non-marketing names
  if (offset != StringRef::npos) {
    // Must start with 'vN'.
    if (A.size() >= 2 && (A[0] != 'v' || !std::isdigit(A[1])))
      return Error;
    // Can't have an extra 'eb'.
    if (A.find("eb") != StringRef::npos)
      return Error;
  }

  // Arch will either be a 'v' name (v7a) or a marketing name (xscale).
  return A;
}

unsigned llvh::ARM::parseHWDiv(StringRef HWDiv) {
  StringRef Syn = getHWDivSynonym(HWDiv);
  for (const auto D : HWDivNames) {
    if (Syn == D.getName())
      return D.ID;
  }
  return ARM::AEK_INVALID;
}

unsigned llvh::ARM::parseFPU(StringRef FPU) {
  StringRef Syn = getFPUSynonym(FPU);
  for (const auto F : FPUNames) {
    if (Syn == F.getName())
      return F.ID;
  }
  return ARM::FK_INVALID;
}

// Allows partial match, ex. "v7a" matches "armv7a".
ARM::ArchKind ARM::parseArch(StringRef Arch) {
  Arch = getCanonicalArchName(Arch);
  StringRef Syn = getArchSynonym(Arch);
  for (const auto A : ARCHNames) {
    if (A.getName().endswith(Syn))
      return A.ID;
  }
  return ARM::ArchKind::INVALID;
}

unsigned llvh::ARM::parseArchExt(StringRef ArchExt) {
  for (const auto A : ARCHExtNames) {
    if (ArchExt == A.getName())
      return A.ID;
  }
  return ARM::AEK_INVALID;
}

ARM::ArchKind llvh::ARM::parseCPUArch(StringRef CPU) {
  for (const auto C : CPUNames) {
    if (CPU == C.getName())
      return C.ArchID;
  }
  return ARM::ArchKind::INVALID;
}

void llvh::ARM::fillValidCPUArchList(SmallVectorImpl<StringRef> &Values) {
  for (const CpuNames<ARM::ArchKind> &Arch : CPUNames) {
    if (Arch.ArchID != ARM::ArchKind::INVALID)
      Values.push_back(Arch.getName());
  }
}

void llvh::AArch64::fillValidCPUArchList(SmallVectorImpl<StringRef> &Values) {
  for (const CpuNames<AArch64::ArchKind> &Arch : AArch64CPUNames) {
    if (Arch.ArchID != AArch64::ArchKind::INVALID)
      Values.push_back(Arch.getName());
  }
}

// ARM, Thumb, AArch64
ARM::ISAKind ARM::parseArchISA(StringRef Arch) {
  return StringSwitch<ARM::ISAKind>(Arch)
      .StartsWith("aarch64", ARM::ISAKind::AARCH64)
      .StartsWith("arm64", ARM::ISAKind::AARCH64)
      .StartsWith("thumb", ARM::ISAKind::THUMB)
      .StartsWith("arm", ARM::ISAKind::ARM)
      .Default(ARM::ISAKind::INVALID);
}

// Little/Big endian
ARM::EndianKind ARM::parseArchEndian(StringRef Arch) {
  if (Arch.startswith("armeb") || Arch.startswith("thumbeb") ||
      Arch.startswith("aarch64_be"))
    return ARM::EndianKind::BIG;

  if (Arch.startswith("arm") || Arch.startswith("thumb")) {
    if (Arch.endswith("eb"))
      return ARM::EndianKind::BIG;
    else
      return ARM::EndianKind::LITTLE;
  }

  if (Arch.startswith("aarch64"))
    return ARM::EndianKind::LITTLE;

  return ARM::EndianKind::INVALID;
}

// Profile A/R/M
ARM::ProfileKind ARM::parseArchProfile(StringRef Arch) {
  Arch = getCanonicalArchName(Arch);
  switch (parseArch(Arch)) {
  case ARM::ArchKind::ARMV6M:
  case ARM::ArchKind::ARMV7M:
  case ARM::ArchKind::ARMV7EM:
  case ARM::ArchKind::ARMV8MMainline:
  case ARM::ArchKind::ARMV8MBaseline:
    return ARM::ProfileKind::M;
  case ARM::ArchKind::ARMV7R:
  case ARM::ArchKind::ARMV8R:
    return ARM::ProfileKind::R;
  case ARM::ArchKind::ARMV7A:
  case ARM::ArchKind::ARMV7VE:
  case ARM::ArchKind::ARMV7K:
  case ARM::ArchKind::ARMV8A:
  case ARM::ArchKind::ARMV8_1A:
  case ARM::ArchKind::ARMV8_2A:
  case ARM::ArchKind::ARMV8_3A:
  case ARM::ArchKind::ARMV8_4A:
  case ARM::ArchKind::ARMV8_5A:
    return ARM::ProfileKind::A;
  case ARM::ArchKind::ARMV2:
  case ARM::ArchKind::ARMV2A:
  case ARM::ArchKind::ARMV3:
  case ARM::ArchKind::ARMV3M:
  case ARM::ArchKind::ARMV4:
  case ARM::ArchKind::ARMV4T:
  case ARM::ArchKind::ARMV5T:
  case ARM::ArchKind::ARMV5TE:
  case ARM::ArchKind::ARMV5TEJ:
  case ARM::ArchKind::ARMV6:
  case ARM::ArchKind::ARMV6K:
  case ARM::ArchKind::ARMV6T2:
  case ARM::ArchKind::ARMV6KZ:
  case ARM::ArchKind::ARMV7S:
  case ARM::ArchKind::IWMMXT:
  case ARM::ArchKind::IWMMXT2:
  case ARM::ArchKind::XSCALE:
  case ARM::ArchKind::INVALID:
    return ARM::ProfileKind::INVALID;
  }
  llvm_unreachable("Unhandled architecture");
}

// Version number (ex. v7 = 7).
unsigned llvh::ARM::parseArchVersion(StringRef Arch) {
  Arch = getCanonicalArchName(Arch);
  switch (parseArch(Arch)) {
  case ARM::ArchKind::ARMV2:
  case ARM::ArchKind::ARMV2A:
    return 2;
  case ARM::ArchKind::ARMV3:
  case ARM::ArchKind::ARMV3M:
    return 3;
  case ARM::ArchKind::ARMV4:
  case ARM::ArchKind::ARMV4T:
    return 4;
  case ARM::ArchKind::ARMV5T:
  case ARM::ArchKind::ARMV5TE:
  case ARM::ArchKind::IWMMXT:
  case ARM::ArchKind::IWMMXT2:
  case ARM::ArchKind::XSCALE:
  case ARM::ArchKind::ARMV5TEJ:
    return 5;
  case ARM::ArchKind::ARMV6:
  case ARM::ArchKind::ARMV6K:
  case ARM::ArchKind::ARMV6T2:
  case ARM::ArchKind::ARMV6KZ:
  case ARM::ArchKind::ARMV6M:
    return 6;
  case ARM::ArchKind::ARMV7A:
  case ARM::ArchKind::ARMV7VE:
  case ARM::ArchKind::ARMV7R:
  case ARM::ArchKind::ARMV7M:
  case ARM::ArchKind::ARMV7S:
  case ARM::ArchKind::ARMV7EM:
  case ARM::ArchKind::ARMV7K:
    return 7;
  case ARM::ArchKind::ARMV8A:
  case ARM::ArchKind::ARMV8_1A:
  case ARM::ArchKind::ARMV8_2A:
  case ARM::ArchKind::ARMV8_3A:
  case ARM::ArchKind::ARMV8_4A:
  case ARM::ArchKind::ARMV8_5A:
  case ARM::ArchKind::ARMV8R:
  case ARM::ArchKind::ARMV8MBaseline:
  case ARM::ArchKind::ARMV8MMainline:
    return 8;
  case ARM::ArchKind::INVALID:
    return 0;
  }
  llvm_unreachable("Unhandled architecture");
}

StringRef llvh::ARM::computeDefaultTargetABI(const Triple &TT, StringRef CPU) {
  StringRef ArchName =
      CPU.empty() ? TT.getArchName() : ARM::getArchName(ARM::parseCPUArch(CPU));

  if (TT.isOSBinFormatMachO()) {
    if (TT.getEnvironment() == Triple::EABI ||
        TT.getOS() == Triple::UnknownOS ||
        llvh::ARM::parseArchProfile(ArchName) == ARM::ProfileKind::M)
      return "aapcs";
    if (TT.isWatchABI())
      return "aapcs16";
    return "apcs-gnu";
  } else if (TT.isOSWindows())
    // FIXME: this is invalid for WindowsCE.
    return "aapcs";

  // Select the default based on the platform.
  switch (TT.getEnvironment()) {
  case Triple::Android:
  case Triple::GNUEABI:
  case Triple::GNUEABIHF:
  case Triple::MuslEABI:
  case Triple::MuslEABIHF:
    return "aapcs-linux";
  case Triple::EABIHF:
  case Triple::EABI:
    return "aapcs";
  default:
    if (TT.isOSNetBSD())
      return "apcs-gnu";
    if (TT.isOSOpenBSD())
      return "aapcs-linux";
    return "aapcs";
  }
}

StringRef llvh::AArch64::getCanonicalArchName(StringRef Arch) {
  return ARM::getCanonicalArchName(Arch);
}

unsigned llvh::AArch64::parseFPU(StringRef FPU) {
  return ARM::parseFPU(FPU);
}

// Allows partial match, ex. "v8a" matches "armv8a".
AArch64::ArchKind AArch64::parseArch(StringRef Arch) {
  Arch = getCanonicalArchName(Arch);
  if (checkArchVersion(Arch) < 8)
    return ArchKind::INVALID;

  StringRef Syn = getArchSynonym(Arch);
  for (const auto A : AArch64ARCHNames) {
    if (A.getName().endswith(Syn))
      return A.ID;
  }
  return ArchKind::INVALID;
}

AArch64::ArchExtKind llvh::AArch64::parseArchExt(StringRef ArchExt) {
  for (const auto A : AArch64ARCHExtNames) {
    if (ArchExt == A.getName())
      return static_cast<ArchExtKind>(A.ID);
  }
  return AArch64::AEK_INVALID;
}

AArch64::ArchKind llvh::AArch64::parseCPUArch(StringRef CPU) {
  for (const auto C : AArch64CPUNames) {
    if (CPU == C.getName())
      return C.ArchID;
  }
  return ArchKind::INVALID;
}

// ARM, Thumb, AArch64
ARM::ISAKind AArch64::parseArchISA(StringRef Arch) {
  return ARM::parseArchISA(Arch);
}

// Little/Big endian
ARM::EndianKind AArch64::parseArchEndian(StringRef Arch) {
  return ARM::parseArchEndian(Arch);
}

// Profile A/R/M
ARM::ProfileKind AArch64::parseArchProfile(StringRef Arch) {
  return ARM::parseArchProfile(Arch);
}

// Version number (ex. v8 = 8).
unsigned llvh::AArch64::parseArchVersion(StringRef Arch) {
  return ARM::parseArchVersion(Arch);
}

bool llvh::AArch64::isX18ReservedByDefault(const Triple &TT) {
  return TT.isAndroid() || TT.isOSDarwin() || TT.isOSFuchsia() ||
         TT.isOSWindows();
}

namespace {

struct GPUInfo {
  StringLiteral Name;
  StringLiteral CanonicalName;
  AMDGPU::GPUKind Kind;
  unsigned Features;
};

constexpr GPUInfo R600GPUs[26] = {
  // Name       Canonical    Kind        Features
  //            Name
  {{"r600"},    {"r600"},    GK_R600,    FEATURE_NONE },
  {{"rv630"},   {"r600"},    GK_R600,    FEATURE_NONE },
  {{"rv635"},   {"r600"},    GK_R600,    FEATURE_NONE },
  {{"r630"},    {"r630"},    GK_R630,    FEATURE_NONE },
  {{"rs780"},   {"rs880"},   GK_RS880,   FEATURE_NONE },
  {{"rs880"},   {"rs880"},   GK_RS880,   FEATURE_NONE },
  {{"rv610"},   {"rs880"},   GK_RS880,   FEATURE_NONE },
  {{"rv620"},   {"rs880"},   GK_RS880,   FEATURE_NONE },
  {{"rv670"},   {"rv670"},   GK_RV670,   FEATURE_NONE },
  {{"rv710"},   {"rv710"},   GK_RV710,   FEATURE_NONE },
  {{"rv730"},   {"rv730"},   GK_RV730,   FEATURE_NONE },
  {{"rv740"},   {"rv770"},   GK_RV770,   FEATURE_NONE },
  {{"rv770"},   {"rv770"},   GK_RV770,   FEATURE_NONE },
  {{"cedar"},   {"cedar"},   GK_CEDAR,   FEATURE_NONE },
  {{"palm"},    {"cedar"},   GK_CEDAR,   FEATURE_NONE },
  {{"cypress"}, {"cypress"}, GK_CYPRESS, FEATURE_FMA  },
  {{"hemlock"}, {"cypress"}, GK_CYPRESS, FEATURE_FMA  },
  {{"juniper"}, {"juniper"}, GK_JUNIPER, FEATURE_NONE },
  {{"redwood"}, {"redwood"}, GK_REDWOOD, FEATURE_NONE },
  {{"sumo"},    {"sumo"},    GK_SUMO,    FEATURE_NONE },
  {{"sumo2"},   {"sumo"},    GK_SUMO,    FEATURE_NONE },
  {{"barts"},   {"barts"},   GK_BARTS,   FEATURE_NONE },
  {{"caicos"},  {"caicos"},  GK_CAICOS,  FEATURE_NONE },
  {{"aruba"},   {"cayman"},  GK_CAYMAN,  FEATURE_FMA  },
  {{"cayman"},  {"cayman"},  GK_CAYMAN,  FEATURE_FMA  },
  {{"turks"},   {"turks"},   GK_TURKS,   FEATURE_NONE }
};

// This table should be sorted by the value of GPUKind
// Don't bother listing the implicitly true features
constexpr GPUInfo AMDGCNGPUs[32] = {
  // Name         Canonical    Kind        Features
  //              Name
  {{"gfx600"},    {"gfx600"},  GK_GFX600,  FEATURE_FAST_FMA_F32},
  {{"tahiti"},    {"gfx600"},  GK_GFX600,  FEATURE_FAST_FMA_F32},
  {{"gfx601"},    {"gfx601"},  GK_GFX601,  FEATURE_NONE},
  {{"hainan"},    {"gfx601"},  GK_GFX601,  FEATURE_NONE},
  {{"oland"},     {"gfx601"},  GK_GFX601,  FEATURE_NONE},
  {{"pitcairn"},  {"gfx601"},  GK_GFX601,  FEATURE_NONE},
  {{"verde"},     {"gfx601"},  GK_GFX601,  FEATURE_NONE},
  {{"gfx700"},    {"gfx700"},  GK_GFX700,  FEATURE_NONE},
  {{"kaveri"},    {"gfx700"},  GK_GFX700,  FEATURE_NONE},
  {{"gfx701"},    {"gfx701"},  GK_GFX701,  FEATURE_FAST_FMA_F32},
  {{"hawaii"},    {"gfx701"},  GK_GFX701,  FEATURE_FAST_FMA_F32},
  {{"gfx702"},    {"gfx702"},  GK_GFX702,  FEATURE_FAST_FMA_F32},
  {{"gfx703"},    {"gfx703"},  GK_GFX703,  FEATURE_NONE},
  {{"kabini"},    {"gfx703"},  GK_GFX703,  FEATURE_NONE},
  {{"mullins"},   {"gfx703"},  GK_GFX703,  FEATURE_NONE},
  {{"gfx704"},    {"gfx704"},  GK_GFX704,  FEATURE_NONE},
  {{"bonaire"},   {"gfx704"},  GK_GFX704,  FEATURE_NONE},
  {{"gfx801"},    {"gfx801"},  GK_GFX801,  FEATURE_FAST_FMA_F32|FEATURE_FAST_DENORMAL_F32},
  {{"carrizo"},   {"gfx801"},  GK_GFX801,  FEATURE_FAST_FMA_F32|FEATURE_FAST_DENORMAL_F32},
  {{"gfx802"},    {"gfx802"},  GK_GFX802,  FEATURE_FAST_DENORMAL_F32},
  {{"iceland"},   {"gfx802"},  GK_GFX802,  FEATURE_FAST_DENORMAL_F32},
  {{"tonga"},     {"gfx802"},  GK_GFX802,  FEATURE_FAST_DENORMAL_F32},
  {{"gfx803"},    {"gfx803"},  GK_GFX803,  FEATURE_FAST_DENORMAL_F32},
  {{"fiji"},      {"gfx803"},  GK_GFX803,  FEATURE_FAST_DENORMAL_F32},
  {{"polaris10"}, {"gfx803"},  GK_GFX803,  FEATURE_FAST_DENORMAL_F32},
  {{"polaris11"}, {"gfx803"},  GK_GFX803,  FEATURE_FAST_DENORMAL_F32},
  {{"gfx810"},    {"gfx810"},  GK_GFX810,  FEATURE_FAST_DENORMAL_F32},
  {{"stoney"},    {"gfx810"},  GK_GFX810,  FEATURE_FAST_DENORMAL_F32},
  {{"gfx900"},    {"gfx900"},  GK_GFX900,  FEATURE_FAST_FMA_F32|FEATURE_FAST_DENORMAL_F32},
  {{"gfx902"},    {"gfx902"},  GK_GFX902,  FEATURE_FAST_FMA_F32|FEATURE_FAST_DENORMAL_F32},
  {{"gfx904"},    {"gfx904"},  GK_GFX904,  FEATURE_FAST_FMA_F32|FEATURE_FAST_DENORMAL_F32},
  {{"gfx906"},    {"gfx906"},  GK_GFX906,  FEATURE_FAST_FMA_F32|FEATURE_FAST_DENORMAL_F32},
};

const GPUInfo *getArchEntry(AMDGPU::GPUKind AK, ArrayRef<GPUInfo> Table) {
  GPUInfo Search = { {""}, {""}, AK, AMDGPU::FEATURE_NONE };

  auto I = std::lower_bound(Table.begin(), Table.end(), Search,
    [](const GPUInfo &A, const GPUInfo &B) {
      return A.Kind < B.Kind;
    });

  if (I == Table.end())
    return nullptr;
  return I;
}

} // namespace

StringRef llvh::AMDGPU::getArchNameAMDGCN(GPUKind AK) {
  if (const auto *Entry = getArchEntry(AK, AMDGCNGPUs))
    return Entry->CanonicalName;
  return "";
}

StringRef llvh::AMDGPU::getArchNameR600(GPUKind AK) {
  if (const auto *Entry = getArchEntry(AK, R600GPUs))
    return Entry->CanonicalName;
  return "";
}

AMDGPU::GPUKind llvh::AMDGPU::parseArchAMDGCN(StringRef CPU) {
  for (const auto C : AMDGCNGPUs) {
    if (CPU == C.Name)
      return C.Kind;
  }

  return AMDGPU::GPUKind::GK_NONE;
}

AMDGPU::GPUKind llvh::AMDGPU::parseArchR600(StringRef CPU) {
  for (const auto C : R600GPUs) {
    if (CPU == C.Name)
      return C.Kind;
  }

  return AMDGPU::GPUKind::GK_NONE;
}

unsigned AMDGPU::getArchAttrAMDGCN(GPUKind AK) {
  if (const auto *Entry = getArchEntry(AK, AMDGCNGPUs))
    return Entry->Features;
  return FEATURE_NONE;
}

unsigned AMDGPU::getArchAttrR600(GPUKind AK) {
  if (const auto *Entry = getArchEntry(AK, R600GPUs))
    return Entry->Features;
  return FEATURE_NONE;
}

void AMDGPU::fillValidArchListAMDGCN(SmallVectorImpl<StringRef> &Values) {
  // XXX: Should this only report unique canonical names?
  for (const auto C : AMDGCNGPUs)
    Values.push_back(C.Name);
}

void AMDGPU::fillValidArchListR600(SmallVectorImpl<StringRef> &Values) {
  for (const auto C : R600GPUs)
    Values.push_back(C.Name);
}

AMDGPU::IsaVersion AMDGPU::getIsaVersion(StringRef GPU) {
  if (GPU == "generic")
    return {7, 0, 0};

  AMDGPU::GPUKind AK = parseArchAMDGCN(GPU);
  if (AK == AMDGPU::GPUKind::GK_NONE)
    return {0, 0, 0};

  switch (AK) {
  case GK_GFX600: return {6, 0, 0};
  case GK_GFX601: return {6, 0, 1};
  case GK_GFX700: return {7, 0, 0};
  case GK_GFX701: return {7, 0, 1};
  case GK_GFX702: return {7, 0, 2};
  case GK_GFX703: return {7, 0, 3};
  case GK_GFX704: return {7, 0, 4};
  case GK_GFX801: return {8, 0, 1};
  case GK_GFX802: return {8, 0, 2};
  case GK_GFX803: return {8, 0, 3};
  case GK_GFX810: return {8, 1, 0};
  case GK_GFX900: return {9, 0, 0};
  case GK_GFX902: return {9, 0, 2};
  case GK_GFX904: return {9, 0, 4};
  case GK_GFX906: return {9, 0, 6};
  default:        return {0, 0, 0};
  }
}
