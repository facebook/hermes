/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Footprint.h"

#if defined(__MACH__)
#include <mach/mach.h>
#include <cstdint>

namespace hermes {
namespace vm {
namespace detail {

size_t regionFootprint(char *start, char *end) {
  const task_t self = mach_task_self();

  vm_address_t vAddr = reinterpret_cast<vm_address_t>(start);
  vm_size_t vSz = static_cast<vm_size_t>(end - start);
  vm_region_extended_info_data_t info;
  mach_msg_type_number_t fields = VM_REGION_EXTENDED_INFO_COUNT;
  mach_port_t unused;

  auto ret = vm_region_64(
      self,
      &vAddr,
      &vSz,
      VM_REGION_EXTENDED_INFO,
      // The expected contents, and requisite size of this struct depend on the
      // previous and next parameters to this function respectively. We cast it
      // to a "generic" info type to indicate this.
      reinterpret_cast<vm_region_info_t>(&info),
      &fields,
      &unused);

  return ret == KERN_SUCCESS ? info.pages_dirtied : SIZE_MAX;
}

} // namespace detail
} // namespace vm
} // namespace hermes

#else

#include <hermes/Support/OSCompat.h>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace hermes {
namespace vm {
namespace detail {

/// Helper input stream manipulator to skip a single character.
static std::istream &skip(std::istream &is) {
  return is.ignore();
}

size_t regionFootprint(char *start, char *end) {
  auto rStart = reinterpret_cast<uintptr_t>(start);
  auto rEnd = reinterpret_cast<uintptr_t>(end);

  char label[] = "Rss:";

  std::ifstream smaps("/proc/self/smaps");

  while (smaps) {
    std::string firstToken;
    smaps >> firstToken;

    // Ignore the rest of the line.
    smaps.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (firstToken.find_last_of(':') != std::string::npos) {
      // We are inside an entry, rather than at the start of one, so we should
      // ignore this line.
      continue;
    }

    // The first token should be the mapping's virtual address range if this is
    // the first line of a mapping's entry, so we extract it.
    std::stringstream ris(firstToken);
    uintptr_t mStart, mEnd;
    ris >> std::hex >> mStart >> skip /* - */ >> mEnd;

    // The working assumption is that the kernel will not split a single memory
    // region allocated by \p mmap across multiple entries in the smaps output.
    if (mStart <= rStart && rEnd <= mEnd) {
      // Found the start of the section pertaining to our memory map
      break;
    }
  }

  while (smaps) {
    std::string line;
    std::getline(smaps, line);

    if (line.compare(0, sizeof(label) - 1, label) != 0) {
      continue;
    }

    std::stringstream lis(line);
    lis.ignore(line.length(), ' '); // Pop the label

    size_t rss;
    std::string unit;
    lis >> std::skipws >> rss >> unit;

    assert(unit == "kB");
    return rss * 1024 / hermes::oscompat::page_size();
  }

  return SIZE_MAX;
}

} // namespace detail
} // namespace vm
} // namespace hermes

#endif
