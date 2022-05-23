/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "stringstorage"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/Support/HashString.h"
#include "hermes/Support/JenkinsHash.h"
#include "hermes/Support/Statistic.h"
#include "hermes/Support/Timer.h"
#include "hermes/Support/UTF8.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/Endian.h"

#include <algorithm>
#include <climits>
#include <deque>

using namespace hermes;
using llvh::ArrayRef;
using llvh::MutableArrayRef;

STATISTIC(StringTableSize, "Size of the packed String table");
STATISTIC(
    StringTableSavings,
    "Byte savings in length of String table versus naive packing");

namespace {

/// Group Name for any timers we create
constexpr const char *const StringTableTimersName = "StringTableBuilder Timers";

constexpr size_t npos = ~size_t(0);

/// Longest string that we'll attempt to pack.
constexpr size_t kMaximumPackableStringLength = 24 * 1024;

/// A helper class responsible for deciding how to "pack" strings, that is, lay
/// out strings in a linear array suitable for ConsecutiveStringStorage. It is
/// templated on the character type (char or char16_t).
template <typename CharT>
class StringPacker {
  StringPacker() = default;

 public:
  /// Represents a single string to be packed.
  struct StringEntry {
    /// Index of the string in the original array.
    uint32_t stringID_;

    /// Text of the string.
    ArrayRef<CharT> chars_;

    /// Position of the string in the output storage.
    /// This is set by the packer.
    size_t offsetInStorage_ = npos;

    // The remaining fields are used only by the optimizing packer.

    /// If parent_ is set, then this string is fully contained within its
    /// parent_, at the given offset.
    StringEntry *parent_ = nullptr;
    size_t offsetInParent_ = npos;

    /// The string that must come after us, if any.
    StringEntry *next_ = nullptr;

    /// The string that must come before us, if any.
    StringEntry *prev_ = nullptr;

    /// The amount that our chars_ overlaps with prev_->chars_.
    size_t overlapAmount_ = 0;

    /// Entries that may not be set as our next_, because they would introduce
    /// a cycle.
    llvh::DenseSet<const StringEntry *> potentialCycles_;

    StringEntry(uint32_t stringID, ArrayRef<CharT> chars)
        : stringID_(stringID), chars_(chars) {}
  };

  /// A Trigram represents three packed characters.
  /// Note that Trigrams are keys in an llvh::DenseSet, which reserves the
  /// values ~0 and ~0+1 as sentinel values. We must not use the MSB.
  using Trigram =
      typename std::conditional<sizeof(CharT) == 1, uint32_t, uint64_t>::type;

  static constexpr size_t TrigramCharCount = 3;

  /// \return a trigram representing the first three characters in \p chars.
  static inline Trigram makeTrigram(const CharT chars[TrigramCharCount]) {
    unsigned shift = CHAR_BIT * sizeof(CharT);
    return (Trigram(chars[0]) << (2 * shift)) |
        (Trigram(chars[1]) << (1 * shift)) | (Trigram(chars[2]) << (0 * shift));
  }

  /// \return a set of all Trigrams of all 3 character prefixes of \p strings.
  static llvh::DenseSet<Trigram> buildPrefixTrigramSet(
      ArrayRef<StringEntry> strings) {
    // A rough experiment showed 8 strings per prefix is a reasonable estimate.
    llvh::DenseSet<Trigram> result(strings.size() / 8);
    for (const StringEntry &entry : strings) {
      ArrayRef<CharT> chars = entry.chars_;
      if (chars.size() >= TrigramCharCount) {
        result.insert(makeTrigram(chars.data()));
      }
    }
    return result;
  }

  /// HashedSuffix is a model of DenseMapInfo suitable for hashing string
  /// suffixes. When iterating over the suffixes, it is N^2 to hash each suffix
  /// separately. We can do better by retaining the hash of the previous suffix
  /// and simply incorporating the next character into it.
  struct HashedSuffix {
    /// The string that has been hashed.
    ArrayRef<CharT> chars_;

    /// The hash value of chars_.
    JenkinsHash hash_;

    /// DenseMapInfo interface. \return the empty key, deferring to ArrayRef.
    static HashedSuffix getEmptyKey() {
      return {llvh::DenseMapInfo<ArrayRef<CharT>>::getEmptyKey(), 0};
    }

    /// DenseMapInfo interface. \return the tombstone key, deferring to
    /// ArrayRef.
    static HashedSuffix getTombstoneKey() {
      return {llvh::DenseMapInfo<ArrayRef<CharT>>::getTombstoneKey(), 0};
    }

    /// DenseMapInfo interface. \return the hash, which we have cached.
    static unsigned getHashValue(const HashedSuffix &s) {
      return s.hash_;
    }

    /// DenseMapInfo interface. \return whether two suffixes are equal.
    static bool isEqual(const HashedSuffix &lhs, const HashedSuffix &rhs) {
      return lhs.hash_ == rhs.hash_ && lhs.chars_ == rhs.chars_;
    }
  };

  /// Represents an entry in a generalized suffix array.
  struct SuffixArrayEntry {
    /// The suffix represented by this entry.
    ArrayRef<CharT> suffix_;

    /// The list of StringEntries that have this suffix.
    std::vector<StringEntry *> entries_;

    /// Convenience move constructor from a std::pair.
    /// This is used to extract SuffixArrayEntries from our uniquing map.
    SuffixArrayEntry(std::pair<HashedSuffix, std::vector<StringEntry *>> &&kv)
        : suffix_(std::move(kv.first.chars_)), entries_(std::move(kv.second)) {}

    /// \return the character at index pos, or -1 if pos >= our length.
    int extCharAt(size_t pos) const {
      return pos >= suffix_.size() ? -1 : suffix_[pos];
    }
  };

  // Given pointers \p begin and \p end, sort the range [begin, end) starting at
  // the character index \p charIdx. This is a recursive function.
  static void radixQuicksort(
      SuffixArrayEntry *begin,
      SuffixArrayEntry *end,
      size_t charIdx) {
    for (;;) {
      if (end - begin <= 1) {
        // Already sorted.
        return;
      }

      // Partition via Hoare scheme. Invariants are:
      //  [begin, lower) < pivot
      //  [upper, end) > pivot
      // Final state adds:
      //  [lower, upper) == pivot
      int pivotChar = begin->extCharAt(charIdx);
      SuffixArrayEntry *lower = begin;
      SuffixArrayEntry *upper = end;
      for (SuffixArrayEntry *cursor = begin + 1; cursor < upper;) {
        int testChar = cursor->extCharAt(charIdx);
        if (testChar < pivotChar) {
          std::swap(*lower++, *cursor++);
        } else if (testChar > pivotChar) {
          std::swap(*--upper, *cursor);
        } else {
          // testChar == pivotChar
          cursor++;
        }
      }
      // We have partitioned [begin, end) according to the character at charIdx.
      // Sort left and right, and then recurse on our equal range (unless we've
      // exhausted it).
      radixQuicksort(begin, lower, charIdx);
      radixQuicksort(upper, end, charIdx);

      // If we reached the end of the pivot, we're done. Otherwise allow
      // ourselves to loop again to inspect the next character.
      // Note this is effectively a tail call.
      if (pivotChar == -1)
        return;

      begin = lower;
      end = upper;
      charIdx += 1;
    }
  }

  /// \return a generalized suffix array over the given \p strings.
  /// Only suffixes that begin with an element of \p prefixesOfInterest (or are
  /// shorter than TrigramCharCount) are included.
  static std::vector<SuffixArrayEntry> buildSuffixArray(
      MutableArrayRef<StringEntry> strings,
      const llvh::DenseSet<Trigram> &prefixesOfInterest) {
    // Unique each suffix, that is, for each suffix that begins with
    // prefixOfInterest construct the list of strings containing it. A rough
    // test showed 8 suffixes per string is a reasonable initial capacity.
    llvh::DenseMap<HashedSuffix, std::vector<StringEntry *>, HashedSuffix>
        suffixMap(8 * strings.size());
    for (StringEntry &entry : strings) {
      size_t charsSize = entry.chars_.size();
      // Skip excessively long strings.
      if (charsSize > kMaximumPackableStringLength) {
        continue;
      }
      const CharT *chars = entry.chars_.data();
      JenkinsHash hash = 0;
      size_t i = charsSize;
      while (i--) {
        hash = updateJenkinsHash(hash, chars[i]);
        if (i + TrigramCharCount <= charsSize &&
            !prefixesOfInterest.count(makeTrigram(&chars[i])))
          continue;
        ArrayRef<CharT> suffix(&chars[i], charsSize - i);
        suffixMap[HashedSuffix{suffix, hash}].push_back(&entry);
      }
    }

    // Move our suffixes to an array, and sort it.
    std::vector<SuffixArrayEntry> result;
    if (!suffixMap.empty()) {
      result.reserve(suffixMap.size());
      std::move(suffixMap.begin(), suffixMap.end(), std::back_inserter(result));
      SuffixArrayEntry *entries = &result[0];
      radixQuicksort(entries, entries + result.size(), 0);
    }
    return result;
  }

  /// Overlap represents an overlap relationship. There is an overlap
  /// relationship between src and dst if some suffix of src is equal to a
  /// prefix of dst. For example, in the strings "splitpea" and "peasoup", we
  /// have src = "splitpea" and dst = "peasoup" with overlap amount 3. Note we
  /// don't track the amount of overlap here; that's stored externally.
  /// Also note overlap is directed: there is no overlap from "peasoup" to
  /// "splitpea" because no suffix of "peasoup" is a prefix of "splitpea".
  struct Overlap {
    ArrayRef<StringEntry *> srcs_;
    StringEntry *dst_;
  };

  /// A list of Overlaps, indexed by the amount of overlap.
  /// For example, WeightIndexedOverlaps[3] is the list of Overlaps with
  /// overlap amount 3.
  using WeightIndexedOverlaps = std::vector<std::vector<Overlap>>;

  /// Given a list of StringEntries, determine overlapping strings as follows:
  /// 1. Set rightString.parent_ to an entry that contains rightString, if any.
  /// 2. If an entry leftString overlaps with \p rightString, add an Overlap
  /// leftString->rightString to \p overlaps
  static void computeOverlapsAndParentForEntry(
      StringEntry *rightString,
      ArrayRef<SuffixArrayEntry> suffixArray,
      WeightIndexedOverlaps *overlaps) {
    // This is a subtle function. We want to compute Overlaps, indexed by
    // overlap amount, and simultaneously identify parents. Iterate over
    // rightString's prefixes. Perform a binary search on our suffix array to
    // find suffixes that have that prefix. If any suffix is exactly equal to
    // that prefix, we have Arcs (suffix owners, rightString). See the
    // documentation "StringPacking.md" for an extended discussion.
    using std::partition_point;
    ArrayRef<CharT> rightChars = rightString->chars_;
    auto lower = suffixArray.begin();
    auto upper = suffixArray.end();
    for (size_t index = 0, rightCharsLen = rightChars.size();
         index < rightCharsLen;
         index++) {
      CharT testChar = rightChars[index];
      // Binary search on [lower, upper) to find all suffixes that have
      // testChar at index.
      lower = partition_point(lower, upper, [=](const SuffixArrayEntry &ent) {
        return ent.extCharAt(index) < testChar;
      });
      upper = partition_point(lower, upper, [=](const SuffixArrayEntry &ent) {
        return ent.extCharAt(index) == testChar;
      });
      assert(lower <= upper && "Binary search crossed the streams");
      if (lower == upper) {
        // No suffixes remaining.
        break;
      }

      // [lower, upper) is now the range of suffixes that start with
      // rightString.slice(0, overlapAmount).
      size_t overlapAmount = index + 1;
      if (overlapAmount < rightCharsLen) {
        // Not the last iteration. We are looking for a prefix match.
        // *lower is a suffix that shares a prefix of length overlapAmount with
        // rightString. If it is equal to that prefix, then we found a suffix
        // equal to the prefix we're looking for. It is sufficient to test for
        // equality by checking the length.
        if (lower->suffix_.size() == overlapAmount) {
          // Add an Overlap at index overlapAmount.
          if (overlaps->size() <= overlapAmount) {
            overlaps->resize(overlapAmount + 1);
          }
          Overlap ov = {lower->entries_, rightString};
          (*overlaps)[overlapAmount].push_back(ov);
        }
      } else {
        // overlapAmount == rightCharsLen. Final iteration! [lower, upper) is
        // now the range of suffixes that have rightEntry as a prefix.
        // That means that rightEntry is wholly contained within some string.
        // Of course it is wholly contained within itself; if it's also
        // contained within some OTHER string, we found a parent.
        // For compressibility, choose the parent with the lowest stringID.
        for (auto cursor = lower; cursor < upper; ++cursor) {
          for (StringEntry *parent : cursor->entries_) {
            // Can't parent ourselves.
            if (parent == rightString)
              continue;

            // Don't parent if we have an existing parent with a lower ID.
            // This means that we prefer parents that tend to end up early in
            // the string table.
            if (rightString->parent_ &&
                rightString->parent_->stringID_ < parent->stringID_)
              continue;

            // We found a parent.
            // rightEntry is a prefix of one of parent's suffixes.
            // Therefore rightEntry appears in the parent at the same offset of
            // the suffix within the parent.
            // A parent should always be longer than its child; otherwise we
            // must have duplicate strings.
            assert(
                parent->chars_.size() > rightString->chars_.size() &&
                "non-unique strings passed to StringPacker");
            rightString->parent_ = parent;
            rightString->offsetInParent_ =
                parent->chars_.size() - cursor->suffix_.size();
          }
        }
      }
    }
  }

  /// For each entry, compute its overlaps and parent as per
  /// computeOverlapsOrParentForEntry.
  /// \p return the list of Overlaps indexed by weight (amount of overlap).
  static WeightIndexedOverlaps computeOverlapsAndParents(
      MutableArrayRef<StringEntry> stringEntries,
      ArrayRef<SuffixArrayEntry> suffixArray) {
    WeightIndexedOverlaps result;
    for (StringEntry &entry : stringEntries) {
      computeOverlapsAndParentForEntry(&entry, suffixArray, &result);
    }
    return result;
  }

  /// Indicate if we can add the edge from \p src to \p dst to our Hamiltonian
  /// path, that is, whether we can take advantage of the overlap between src
  /// and dst by positioning dst to overlap a suffix of src.
  static bool canOverlap(const StringEntry *src, const StringEntry *dst) {
    // Are we trying to overlap ourself?
    if (src == dst)
      return false;

    // Are src or dst going to be substrings of another string?
    if (src->parent_ || dst->parent_)
      return false;

    // Did we already pick a string to come after src or before dst?
    if (src->next_ || dst->prev_)
      return false;

    // Would forming src->dst create a cycle?
    if (src->potentialCycles_.count(dst))
      return false;

    // This edge is OK!
    return true;
  }

  /// Plan how we are going to lay out our strings by applying the weightiest
  /// Overlaps. That is, for each entry, determine the (at most 1) entry
  /// to come before it, and the (at most 1) entry to come after it.
  /// This uses the greedy heuristic. Walk our Overlaps from greatest to least
  /// amount, and plan to layout src directly before dst if we can.
  /// We must be careful to not produce a cycle like `a->b->c->a`.
  /// This is equivalent to computing Hamiltonian path in the graph of strings
  /// while attempting to maximize the weight of its edges.
  static void planLayout(const WeightIndexedOverlaps &overlapsByWeight) {
    size_t overlapAmount = overlapsByWeight.size();
    while (overlapAmount--) {
      for (const Overlap &overlap : overlapsByWeight[overlapAmount]) {
        StringEntry *dst = overlap.dst_;
        if (dst->prev_ || dst->parent_) {
          // dst is already spoken for, no need to consider it further.
          continue;
        }
        for (StringEntry *src : overlap.srcs_) {
          if (canOverlap(src, dst)) {
            // Apply the Overlap.
            src->next_ = dst;
            dst->prev_ = src;
            dst->overlapAmount_ = overlapAmount;

            // Traverse to the start and end of our new chain, and mark the fact
            // that end->start is prohibited because it would produce a cycle.
            StringEntry *end = dst;
            while (end->next_)
              end = end->next_;
            StringEntry *start = src;
            while (start->prev_)
              start = start->prev_;
            end->potentialCycles_.insert(start);

            // We picked an entry to come before dst, so we're done with dst.
            break;
          }
        }
      }
    }
  }

  /// Given a string \p entry, lay it out if it has not already been laid out,
  /// appending any data necessaery to the \p storage blob.
  /// "Laying out" an entry means positioning it within the storage, by setting
  /// its offsetInStorage_ field. Laying out an entry may mean laying out other
  /// entries: its parent, or its prev and next entries.
  static void layoutIfNeeded(StringEntry *entry, std::vector<CharT> *storage) {
    if (entry->offsetInStorage_ != npos) {
      return; // already layed out
    }

    // Empty string is trivial
    if (entry->chars_.empty()) {
      entry->offsetInStorage_ = 0;
      return;
    }

    // If we have a parent, lay it out position ourself within it
    if (entry->parent_) {
      assert(entry->parent_ != entry && "entry cannot parent itself");
      assert(entry->offsetInParent_ != npos && "parent with no offset");
      assert(
          entry->prev_ == nullptr && entry->next_ == nullptr &&
          "Cannot have a parent and be in the path");
      layoutIfNeeded(entry->parent_, storage);
      entry->offsetInStorage_ =
          entry->parent_->offsetInStorage_ + entry->offsetInParent_;
      return;
    }

    // Go to the beginning and then layout forwards
    StringEntry *cursor = entry;
    while (cursor->prev_) {
      cursor = cursor->prev_;
    }
    assert(cursor->overlapAmount_ == 0 && "Cannot have overlap with no prev");
    while (cursor) {
      const auto &str = cursor->chars_;
      assert(
          cursor->offsetInStorage_ == npos &&
          "Cannot have already laid out an entry in this path");
      assert(
          cursor->overlapAmount_ <= str.size() &&
          "overlap cannot be larger than string size");
      assert(
          cursor->overlapAmount_ <= storage->size() &&
          "overlap cannot exceed the storage laid out so far ");
      cursor->offsetInStorage_ = storage->size() - cursor->overlapAmount_;
      storage->insert(
          storage->end(), str.begin() + cursor->overlapAmount_, str.end());
      cursor = cursor->next_;
    }
  }

  /// Packs \p strings in an optimized way.
  /// \return a storage blob, with the property that each entry appears at its
  /// offsetInStorage_ within the blob.
  static std::vector<CharT> optimizingPackStrings(
      MutableArrayRef<StringEntry> strings) {
    auto prefixSet = buildPrefixTrigramSet(strings);
    auto suffixes = buildSuffixArray(strings, prefixSet);
    auto overlaps = computeOverlapsAndParents(strings, suffixes);
    planLayout(overlaps);
    std::vector<CharT> storage;
    for (StringEntry &entry : strings) {
      layoutIfNeeded(&entry, &storage);
    }
    return storage; // efficient move
  }

  /// Packs \p strings naively, in their original order.
  /// \return a storage blob with the property that each string appears at its
  /// offsetInStorage_ within the blob.
  static std::vector<CharT> fastPackStrings(
      MutableArrayRef<StringEntry> strings) {
    std::vector<CharT> result;
    for (StringEntry &str : strings) {
      str.offsetInStorage_ = result.size();
      result.insert(result.end(), str.chars_.begin(), str.chars_.end());
    }
    // Note efficient move-semantics on return.
    return result;
  }

#ifndef NDEBUG
  /// Validate a string packing. asserts that each string in \p strings does
  /// indeed appear at its claimed offset within \p storage.
  static void validateStringPacking(
      ArrayRef<StringEntry> strings,
      ArrayRef<CharT> storage) {
    for (const auto &entry : strings) {
      auto offset = entry.offsetInStorage_;
      auto size = entry.chars_.size();
      assert(
          offset + size >= offset && offset + size <= storage.size() &&
          "Invalid offset or size for string entry");
      assert(
          entry.chars_ == storage.slice(offset, size) &&
          "String does not appear at claimed offset in storage");
    }
  }
#endif
};

/// Class responsible for building a string table and associated storage blob.
/// This is constructed from a list of StringRefs. Strings which are in ASCII
/// are separated from strings that require a Unicode representation.
/// Internally we represent strings as StringEntries, which associates some
/// metadata about the string for use by the StringPacker.
class StringTableBuilder {
 private:
  /// We work in ArrayRefs, but we need something to own the storage for our u16
  /// strings. Each u16 string is owned by a vector<char16_t>.
  /// Note this cannot be a vector-of-vectors because vector invalidates
  /// its contents (i.e. strings are moved). deque and list will both work since
  /// push_back() is guaranteed to not invalidate references to the elements.
  std::deque<std::vector<char16_t>> u16StringStorage_;

 public:
  // Entries which are in ASCII.
  std::vector<StringPacker<unsigned char>::StringEntry> asciiStrings_;

  // Entries which contain non-ASCII characters.
  std::vector<StringPacker<char16_t>::StringEntry> u16Strings_;

  /// Constructs from a list of strings, given as a pair of iterators: begin
  /// and end.  Note that we do not always copy the underlying string data so
  /// the resulting builder must not outlive these strings.  In delta
  /// optimizing mode, only new strings are added here and packed.
  template <typename I>
  StringTableBuilder(I begin, I end) {
    // Generate and store a StringEntry for each string.
    // Remember the index of each string in our StringEntry, so that we can
    // later output the table in the correct order.
    // Place the entry in our ASCII entries if possible; otherwise we have to
    // convert to u16.
    // In delta optimizing mode, the ids of new strings in the string table
    // are adjusted to reflect that they appear after the old strings.
    uint32_t index = 0;
    for (auto it = begin; it != end; ++it) {
      auto &str = *it;
      static_assert(sizeof(str.data()[0]) == 1, "strings must be UTF8");
      const unsigned char *begin = (const unsigned char *)str.data();
      const unsigned char *end = begin + str.size();
      if (isAllASCII(begin, end)) {
        ArrayRef<unsigned char> astr(begin, end);
        asciiStrings_.emplace_back(index, astr);
      } else {
        u16StringStorage_.emplace_back();
        std::vector<char16_t> &ustr = u16StringStorage_.back();
        convertUTF8WithSurrogatesToUTF16(
            std::back_inserter(ustr), (const char *)begin, (const char *)end);
        u16Strings_.emplace_back(index, ustr);
      }
      index++;
    }
  }

  /// \return the size our strings would occupy with no packing applied.
  size_t unpackedSize() const {
    size_t result = 0;
    for (const auto &entry : asciiStrings_) {
      result += sizeof(char) * entry.chars_.size();
    }
    for (const auto &entry : u16Strings_) {
      result += sizeof(char16_t) * entry.chars_.size();
    }
    return result;
  }

  /// "Pack" our strings into the given storages. This means setting the
  /// offsetInStorage_ field of each string entry, and then generating a
  /// corresponding storage blob such that each string is at that offset in the
  /// blob. Returns packed \p asciiStorage and \p u16Storage, by reference.
  /// If \p optimize is set, attempt to find a more efficient packing by reusing
  /// substrings and prefix-suffix overlaps.
  void packIntoStorage(
      std::vector<unsigned char> *asciiStorage,
      std::vector<char16_t> *u16Storage,
      bool optimize) {
    NamedRegionTimer timer(
        "",
        optimize ? "optimizingPackStrings" : "fastPackStrings",
        "",
        StringTableTimersName,
        AreStatisticsEnabled());
    // Note these assignments use efficient move-assignment, not copying.
    if (optimize) {
      *asciiStorage =
          StringPacker<unsigned char>::optimizingPackStrings(asciiStrings_);
      *u16Storage = StringPacker<char16_t>::optimizingPackStrings(u16Strings_);
    } else {
      *asciiStorage =
          StringPacker<unsigned char>::fastPackStrings(asciiStrings_);
      *u16Storage = StringPacker<char16_t>::fastPackStrings(u16Strings_);
    }

#ifndef NDEBUG
    // Ensure that our packing was correct.
    StringPacker<unsigned char>::validateStringPacking(
        asciiStrings_, *asciiStorage);
    StringPacker<char16_t>::validateStringPacking(u16Strings_, *u16Storage);
#endif
  }

  /// Given that our strings have been packed into some storage, builds a string
  /// table from our stored string entries.
  /// \return a string table representing the offset and length of each string.
  /// Offsets are taken from the offsetInStorage_ property of each string entry.
  /// u16 string offsets are adjusted by \p u16OffsetAdjust and have their size
  /// negated to indicate they are Unicode. If a string is in the
  /// \p identifiers, we also mark the second most significant bit.
  std::vector<StringTableEntry> generateStringTable(
      ArrayRef<unsigned char> storage,
      size_t u16OffsetAdjust) {
    // Each of our StringEntries remembers its original index in the initial
    // array. Create a table large enough, and set the corresponding index in
    // the table.
    std::vector<StringTableEntry> table;
    table.resize(asciiStrings_.size() + u16Strings_.size());

    for (const auto &asciiStr : asciiStrings_) {
      table.at(asciiStr.stringID_) = {
          static_cast<uint32_t>(asciiStr.offsetInStorage_),
          static_cast<uint32_t>(asciiStr.chars_.size()),
          false /* isUTF16 */};
    }
    for (const auto &u16Str : u16Strings_) {
      table.at(u16Str.stringID_) = {
          static_cast<uint32_t>(
              u16Str.offsetInStorage_ * sizeof(char16_t) + u16OffsetAdjust),
          static_cast<uint32_t>(u16Str.chars_.size()),
          true /* isUTF16 */};
    }
    return table;
  }

  /// Appends \p u16Storage to the given byte array \p output.
  /// \return the offset of the u16 storage in that byte array.
  static size_t appendU16Storage(
      ArrayRef<char16_t> u16Storage,
      std::vector<unsigned char> *output) {
    using namespace llvh::support;
    static_assert(sizeof(char16_t) == 2, "sizeof char16_t unexpectedly not 2");
    if (u16Storage.empty()) {
      // Nothing to do, don't even bother aligning.
      return 0;
    }

    // Ensure 2-byte alignment.
    if (output->size() % sizeof(char16_t)) {
      output->push_back('\0');
    }

    // Make space, and write as little endian.
    size_t offset = output->size();
    output->resize(output->size() + sizeof(char16_t) * u16Storage.size());
    unsigned char *cursor = &output->at(offset);
    for (char16_t s : u16Storage) {
      endian::write<char16_t, little, 0>(cursor, s);
      cursor += sizeof(s);
    }
    return offset;
  }
};

} // namespace

namespace hermes {
namespace hbc {

template <typename I>
ConsecutiveStringStorage::ConsecutiveStringStorage(
    I begin,
    I end,
    bool optimize) {
  // Prepare to build our string table.
  // Generate storage for our ASCII and u16 strings.
  StringTableBuilder builder(begin, end);
  std::vector<unsigned char> asciiStorage;
  std::vector<char16_t> u16Storage;
  builder.packIntoStorage(&asciiStorage, &u16Storage, optimize);

  // Append the u16 storage to the ASCII storage, to form our combined storage.
  storage_.insert(storage_.end(), asciiStorage.begin(), asciiStorage.end());
  auto u16Offset = StringTableBuilder::appendU16Storage(u16Storage, &storage_);

  // Build our table over the storage.
  strTable_ = builder.generateStringTable(storage_, u16Offset);

  // Record some stats.
  size_t unpackedSize = builder.unpackedSize();
  StringTableSize += unpackedSize;
  StringTableSavings += unpackedSize - storage_.size();
}

template ConsecutiveStringStorage::ConsecutiveStringStorage(
    StringSetVector::const_iterator begin,
    StringSetVector::const_iterator end,
    bool optimize);

template ConsecutiveStringStorage::ConsecutiveStringStorage(
    StringSetVector::iterator begin,
    StringSetVector::iterator end,
    bool optimize);

template ConsecutiveStringStorage::ConsecutiveStringStorage(
    ArrayRef<llvh::StringRef>::const_iterator begin,
    ArrayRef<llvh::StringRef>::const_iterator end,
    bool optimize);

uint32_t ConsecutiveStringStorage::getEntryHash(size_t i) const {
  ensureTableValid();
  ensureStorageValid();

  auto &entry = strTable_[i];
  uint32_t length = entry.getLength();
  assert(
      entry.getOffset() + (entry.isUTF16() ? length * 2 : length) <=
          storage_.size() &&
      "entry past end");
  const unsigned char *data = storage_.data() + entry.getOffset();
  if (entry.isUTF16()) {
    const char16_t *u16data = reinterpret_cast<const char16_t *>(data);
    return hermes::hashString(ArrayRef<char16_t>{u16data, length});
  } else {
    return hermes::hashString(ArrayRef<char>{(const char *)data, length});
  }
}

void ConsecutiveStringStorage::appendStorage(ConsecutiveStringStorage &&rhs) {
  ensureTableValid();
  ensureStorageValid();
  // If we have not yet been written, just acquire the rhs.
  if (strTable_.empty()) {
    *this = std::move(rhs);
    return;
  }
  // Offset incoming string entries by the size of our storage, and append the
  // incoming storage. Don't bother to offset if the string is empty; this
  // ensures that the empty string doesn't get pushed to strange places.
  uint32_t storageDelta = storage_.size();
  strTable_.reserve(strTable_.size() + rhs.strTable_.size());
  for (const StringTableEntry &entry : rhs.strTable_) {
    uint32_t length = entry.getLength();
    uint32_t offset = entry.getOffset() + (length ? storageDelta : 0);
    strTable_.emplace_back(offset, length, entry.isUTF16());
  }
  storage_.insert(storage_.end(), rhs.storage_.begin(), rhs.storage_.end());
}

llvh::StringRef ConsecutiveStringStorage::getStringAtIndex(
    uint32_t idx,
    std::string &utf8ConversionStorage) const {
  ensureTableValid();
  ensureStorageValid();
  assert(idx < strTable_.size() && "getStringAtIndex: invalid index");
  return getStringFromEntry(strTable_[idx], storage_, utf8ConversionStorage);
}

llvh::StringRef getStringFromEntry(
    const StringTableEntry &entry,
    llvh::ArrayRef<unsigned char> storage,
    std::string &utf8ConversionStorage) {
  uint32_t offset = entry.getOffset();
  uint32_t length = entry.getLength();
  assert(
      offset + length <= storage.size() && offset + length >= offset &&
      "Invalid entry");
  if (!entry.isUTF16()) {
    return StringRef{(const char *)storage.data() + offset, length};
  } else {
    const char16_t *s =
        reinterpret_cast<const char16_t *>(storage.data() + offset);
    llvh::ArrayRef<char16_t> u16String(s, length);
    convertUTF16ToUTF8WithSingleSurrogates(utf8ConversionStorage, u16String);
    return utf8ConversionStorage;
  }
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
