/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/Sorting.h"

#include "hermes/Support/Compiler.h"

#include "llvh/Support/MathExtras.h"

#include <algorithm>
#include <vector>

namespace hermes {
namespace vm {

SortModel::~SortModel() = default;

namespace {

/// Helper function to make sort algorithm stable
/// If [i] < [j], return true
/// If [j] < [i], return false
/// If [i] == [j], compare their original index
CallResult<bool> _less(
    SortModel *sm,
    const std::vector<uint32_t> &index,
    uint32_t i,
    uint32_t j) {
  auto res = sm->compare(i, j);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(i < index.size() && "OOB should be impossible");
  assert(j < index.size() && "OOB should be impossible");
  return (*res != 0) ? (*res < 0) : (index[i] < index[j]);
}

/// Helper function to swap both items and their indices
ExecutionStatus
_swap(SortModel *sm, std::vector<uint32_t> &index, uint32_t i, uint32_t j) {
  if (sm->swap(i, j) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(i < index.size() && "OOB should be impossible");
  assert(j < index.size() && "OOB should be impossible");
  std::swap(index[i], index[j]);
  return ExecutionStatus::RETURNED;
}

/**
 * @param base the beginning of the logical array
 */
LLVM_NODISCARD ExecutionStatus heapFixDown(
    SortModel *sm,
    std::vector<uint32_t> &index,
    uint32_t base,
    uint32_t begin,
    uint32_t end) {
  CallResult<bool> res{false};
  if (LLVM_UNLIKELY(end - begin <= 1)) {
    return ExecutionStatus::RETURNED;
  }

  uint32_t lastGood = base + (end - base - 2) / 2;
  uint32_t i = begin;

  while (i <= lastGood) {
    uint32_t j = (i - base) * 2 + 1 + base;
    // Find the greater of the two children
    if (j + 1 < end) {
      res = _less(sm, index, j, j + 1);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if (*res) {
        ++j;
      }
    }
    // If the child is greater than us, exchange places
    res = _less(sm, index, i, j);
    if (res == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*res) {
      break;
    }

    if (_swap(sm, index, i, j) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    i = j;
  }

  return ExecutionStatus::RETURNED;
}

ExecutionStatus heapSort(
    SortModel *sm,
    std::vector<uint32_t> &index,
    uint32_t begin,
    uint32_t end) {
  if (LLVM_UNLIKELY(end - begin <= 1)) {
    return ExecutionStatus::RETURNED;
  }

  // "heapify"
  uint32_t start = (end - begin - 2) / 2 + begin;
  do {
    if (heapFixDown(sm, index, begin, start, end) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  } while (start-- != begin);

  while (end - begin > 1) {
    --end;
    if (_swap(sm, index, begin, end) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (heapFixDown(sm, index, begin, begin, end) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return ExecutionStatus::RETURNED;
}

ExecutionStatus insertionSort(
    SortModel *sm,
    std::vector<uint32_t> &index,
    uint32_t begin,
    uint32_t end) {
  CallResult<bool> res{false};
  if (begin == end) {
    return ExecutionStatus::RETURNED;
  }

  for (uint32_t i = begin + 1; i != end; ++i) {
    for (uint32_t j = i; j != begin; --j) {
      res = _less(sm, index, j, j - 1);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!*res) {
        break;
      }
      if (_swap(sm, index, j, j - 1) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }
  return ExecutionStatus::RETURNED;
}

// Must be at lest 3, for "median of three" to work
const uint32_t INSERTION_THRESHOLD = 6;

/// Performs the partition for quickSort between elements [l,r].
/// Pre-condition: [l] <= [l+1] <= [r] and [l+1] is the pivot.
/// This routine is guaranteed to complete even with an inconsistent comparison
/// routine because it always makes forward progress and doesn't rely on
/// sentinels.
/// \return the new index of the pivot.
CallResult<uint32_t> quickSortPartition(
    SortModel *sm,
    std::vector<uint32_t> &index,
    uint32_t l,
    uint32_t r) {
  CallResult<bool> res{false};
  // Now [l] <= [l+1] <= [r]
  // [l+1] is our pivot.
  uint32_t pivot = l + 1;

  uint32_t i = pivot + 1, j = r;
  for (;;) {
    for (; i <= j; ++i) {
      res = _less(sm, index, i, pivot);
      if (res == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      if (!*res)
        break;
    }
    assert(i <= r + 1 && "i is out of range");

    for (; i <= j; --j) {
      res = _less(sm, index, pivot, j);
      if (res == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      if (!*res)
        break;
    }
    assert(j > l && "j is out of range");

    if (i >= j)
      break;
    if (_swap(sm, index, i, j) == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    ++i;
    --j;
  }

  // [j] <= [pivot], we can put the pivot in its final position
  if (j != pivot) {
    if (_swap(sm, index, pivot, j) == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
  }

  return j;
}

ExecutionStatus doQuickSort(
    SortModel *sm,
    std::vector<uint32_t> &index,
    int limit,
    uint32_t l,
    uint32_t r) {
  CallResult<bool> res{false};
quicksort_top:
  if (limit <= 0) {
    // Bail to heap sort
    return heapSort(sm, index, l, r + 1);
  }

  // Median-of-three
  // Place the middle element at [l+1]
  if (_swap(sm, index, l + 1, l + ((r - l) >> 1)) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // Sort, [l], [l+1], [r]
  res = _less(sm, index, r, l + 1);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    if (_swap(sm, index, r, l + 1) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  res = _less(sm, index, l + 1, l);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    if (_swap(sm, index, l + 1, l) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  res = _less(sm, index, r, l + 1);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    if (_swap(sm, index, r, l + 1) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  // Now [l] <= [l+1] <= [r]
  // [l+1] is our pivot and [r] is a sentinel
  CallResult<uint32_t> partitionResult{quickSortPartition(sm, index, l, r)};
  if (partitionResult == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t j = *partitionResult;

  // To limit the stack size, recurse for the smaller partition and do
  // tail-recursion for the bigger one
  uint32_t lSize = j - l;
  uint32_t rSize = r - j;
  if (lSize <= rSize) {
    if (lSize > INSERTION_THRESHOLD) {
      if (doQuickSort(sm, index, limit - 1, l, j - 1) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      if (insertionSort(sm, index, l, j) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    if (rSize > INSERTION_THRESHOLD) {
      l = j + 1;
      --limit;
      goto quicksort_top;
    } else {
      if (insertionSort(sm, index, j + 1, r + 1) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }

  } else {
    if (rSize > INSERTION_THRESHOLD) {
      if (doQuickSort(sm, index, limit - 1, j + 1, r) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      if (insertionSort(sm, index, j + 1, r + 1) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    if (lSize > INSERTION_THRESHOLD) {
      r = j - 1;
      --limit;
      goto quicksort_top;
    } else {
      if (insertionSort(sm, index, l, j) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  return ExecutionStatus::RETURNED;
}

} // namespace

ExecutionStatus quickSort(SortModel *sm, uint32_t begin, uint32_t end) {
  if (begin >= end)
    return ExecutionStatus::RETURNED;
  uint32_t len = end - begin;
  std::vector<uint32_t> index(len); // Array of original indices of items
  for (uint32_t i = 0; i < len; ++i) {
    index[i] = i;
  }

  if (len > INSERTION_THRESHOLD) {
    return doQuickSort(sm, index, llvh::Log2_32(len) * 2, begin, end - 1);
  } else {
    return insertionSort(sm, index, begin, end);
  }
}

} // namespace vm
} // namespace hermes
