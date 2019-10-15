/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Sorting.h"

#include "hermes/Support/Compiler.h"

#include "llvm/Support/MathExtras.h"

#include <cstdio>

namespace hermes {
namespace vm {

SortModel::~SortModel(){};

namespace {

/**
 * @param base the beginning of the logical array
 */
LLVM_NODISCARD ExecutionStatus
heapFixDown(SortModel *sm, uint32_t base, uint32_t begin, uint32_t end) {
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
      res = sm->less(j, j + 1);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if (*res) {
        ++j;
      }
    }
    // If the child is greater than us, exchange places
    res = sm->less(i, j);
    if (res == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*res) {
      break;
    }

    if (sm->swap(i, j) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    i = j;
  }

  return ExecutionStatus::RETURNED;
}

ExecutionStatus heapSort(SortModel *sm, uint32_t begin, uint32_t end) {
  if (LLVM_UNLIKELY(end - begin <= 1)) {
    return ExecutionStatus::RETURNED;
  }

  // "heapify"
  uint32_t start = (end - begin - 2) / 2 + begin;
  do {
    if (heapFixDown(sm, begin, start, end) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  } while (start-- != begin);

  while (end - begin > 1) {
    --end;
    if (sm->swap(begin, end) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (heapFixDown(sm, begin, begin, end) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return ExecutionStatus::RETURNED;
}

ExecutionStatus insertionSort(SortModel *sm, uint32_t begin, uint32_t end) {
  CallResult<bool> res{false};
  if (begin == end) {
    return ExecutionStatus::RETURNED;
  }

  for (uint32_t i = begin + 1; i != end; ++i) {
    for (uint32_t j = i; j != begin; --j) {
      res = sm->less(j, j - 1);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!*res) {
        break;
      }
      if (sm->swap(j, j - 1) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }
  return ExecutionStatus::RETURNED;
}

// Must be at lest 3, for "median of three" to work
const uint32_t INSERTION_THRESHOLD = 6;

/// Performs the partition for quickSort between elements [l,r].
/// The pivot must be at element [l+1].
/// \return the new index of the pivot.
CallResult<uint32_t> quickSortPartition(SortModel *sm, uint32_t l, uint32_t r) {
  CallResult<bool> res{false};
  // Now [l] <= [l+1] <= [r]
  // [l+1] is our pivot and [r] is a sentinel
  uint32_t pivot = l + 1;

  uint32_t i = pivot, j = r + 1;
  // The pivot is at [l+1]
  while (true) {
    while (true) {
      ++i;
      res = sm->less(i, pivot);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!*res) {
        break;
      }
    }
    while (true) {
      --j;
      res = sm->less(pivot, j);
      if (res == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!*res) {
        break;
      }
    }
    if (i >= j) {
      break;
    }
    if (sm->swap(i, j) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  // put the pivot in its final position
  if (j != pivot) {
    if (sm->swap(pivot, j) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return j;
}

ExecutionStatus doQuickSort(SortModel *sm, int limit, uint32_t l, uint32_t r) {
  CallResult<bool> res{false};
quicksort_top:
  if (limit <= 0) {
    // Bail to heap sort
    return heapSort(sm, l, r + 1);
  }

  // Median-of-three
  // Place the middle element at [l+1]
  if (sm->swap(l + 1, l + ((r - l) >> 1)) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // Sort, [l], [l+1], [r]
  res = sm->less(r, l + 1);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    if (sm->swap(r, l + 1) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  res = sm->less(l + 1, l);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    if (sm->swap(l + 1, l) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  res = sm->less(r, l + 1);
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    if (sm->swap(r, l + 1) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  // Now [l] <= [l+1] <= [r]
  // [l+1] is our pivot and [r] is a sentinel
  CallResult<uint32_t> partitionResult{quickSortPartition(sm, l, r)};
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
      if (doQuickSort(sm, limit - 1, l, j - 1) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      if (insertionSort(sm, l, j) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    if (rSize > INSERTION_THRESHOLD) {
      l = j + 1;
      --limit;
      goto quicksort_top;
    } else {
      if (insertionSort(sm, j + 1, r + 1) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }

  } else {
    if (rSize > INSERTION_THRESHOLD) {
      if (doQuickSort(sm, limit - 1, j + 1, r) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    } else {
      if (insertionSort(sm, j + 1, r + 1) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }

    if (lSize > INSERTION_THRESHOLD) {
      r = j - 1;
      --limit;
      goto quicksort_top;
    } else {
      if (insertionSort(sm, l, j) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  return ExecutionStatus::RETURNED;
}

} // namespace

ExecutionStatus quickSort(SortModel *sm, uint32_t begin, uint32_t end) {
  if (end - begin > INSERTION_THRESHOLD) {
    return doQuickSort(sm, llvm::Log2_32(end - begin) * 2, begin, end - 1);
  } else {
    return insertionSort(sm, begin, end);
  }
}

} // namespace vm
} // namespace hermes
