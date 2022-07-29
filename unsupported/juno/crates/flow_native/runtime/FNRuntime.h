/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef FNRUNTIME_H
#define FNRUNTIME_H

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct FNString;
struct FNObject;
struct FNClosure;

void *fnMalloc(size_t);

/// Replacement for std::allocator<> based on fnMalloc.
template <typename T>
struct fn_allocator {
  typedef T value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::true_type propagate_on_container_move_assignment;
  typedef std::true_type is_always_equal;

  T *address(T &x) const noexcept {
    return &x;
  }
  const T *address(const T &x) const noexcept {
    return &x;
  }
  T *allocate(size_type n) {
    return (T *)fnMalloc(n * sizeof(T));
  }
  T *allocate(size_type n, const void * /*hint*/) {
    return (T *)fnMalloc(n * sizeof(T));
  }
  void deallocate(T *, size_type) {}
  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }
};

/// Replacement for std::string using fn_allocator.
using fn_string =
    std::basic_string<char, std::char_traits<char>, fn_allocator<char>>;
/// Replacement for std::vector using fn_allocator.
template <typename T>
using fn_vector = std::vector<T, fn_allocator<T>>;

enum class FNType {
  Undefined,
  Null,
  Number,
  Bool,
  String,
  Symbol,
  Object,
  Closure
};

/// A unique string index.
typedef uint32_t FNUniqueString;

struct FNPredefined {
  enum : FNUniqueString {
    // Index 0 is reserved for empty hash table slots.
    _EMPTY,
    // Index 1 is reserved for deleted hash table slots.
    _DELETED,
#define FN_PREDEFINED(n) n,
#include "predefined.def"
    _LAST
  };

  static inline bool isValid(FNUniqueString x) {
    return x > _DELETED;
  }
};

// WARNING: This implementation is TEMPORARY and purely for development
// purposes. It will mostly be deleted once we have real type checking.
class FNValue {
  FNType tag;
  union {
    double num;
    uint64_t u64;
    FNObject *obj;
    const FNString *str;
  } value;

  static_assert(
      sizeof(value) >= sizeof(uintptr_t),
      "Value must be able to fit a pointer.");

 public:
  bool isUndefined() const {
    return tag == FNType::Undefined;
  }
  bool isNull() const {
    return tag == FNType::Null;
  }
  bool isNumber() const {
    return tag == FNType::Number;
  }
  bool isBool() const {
    return tag == FNType::Bool;
  }
  bool isString() const {
    return tag == FNType::String;
  }
  bool isSymbol() const {
    return tag == FNType::Symbol;
  }
  bool isObject() const {
    return tag == FNType::Object;
  }
  bool isClosure() const {
    return tag == FNType::Closure;
  }

  double getNumber() const {
    assert(isNumber());
    return value.num;
  }
  bool getBool() const {
    assert(isBool() || isNumber() || isUndefined() || isNull());
    uint64_t r;
    memcpy(&r, &value, sizeof(r));
    return r;
  }
  const FNString *getString() const {
    assert(isString());
    return value.str;
  }
  FNObject *getObject() const {
    assert(isObject() || isClosure());
    return value.obj;
  }
  inline FNClosure *getClosure() const;

  static FNValue encodeUndefined() {
    FNValue ret;
    ret.tag = FNType::Undefined;
    // Explicitly initialize value so we can reliably test for equality.
    ret.value.u64 = 0;
    return ret;
  }
  static FNValue encodeNull() {
    FNValue ret;
    ret.tag = FNType::Null;
    // Explicitly initialize value so we can reliably test for equality.
    ret.value.u64 = 0;
    return ret;
  }
  static FNValue encodeNumber(double num) {
    FNValue ret;
    ret.tag = FNType::Number;
    ret.value.num = num;
    return ret;
  }
  static FNValue encodeBool(bool b) {
    FNValue ret;
    ret.tag = FNType::Bool;
    ret.value.u64 = b;
    return ret;
  }
  static FNValue encodeString(const FNString *str) {
    FNValue ret;
    ret.tag = FNType::String;
    ret.value.str = str;
    return ret;
  }
  static FNValue encodeObject(FNObject *obj) {
    FNValue ret;
    ret.tag = FNType::Object;
    ret.value.obj = obj;
    return ret;
  }
  static inline FNValue encodeClosure(FNClosure *closure);

  static bool isEqual(FNValue a, FNValue b) {
    // return a.tag == b.tag && memcmp(a.v, b.v, sizeof(a.v)) == 0;
    // NOTE: had to use this clumsier version because old GCC versions don't
    // inline memcmp().
    if (a.tag != b.tag)
      return false;
    uint64_t x, y;
    memcpy(&x, &a.value, sizeof(x));
    memcpy(&y, &b.value, sizeof(y));
    return x == y;
  }

  static const FNString *typeOf(FNValue v);
};

class FNPropMap {
 public:
  using key_type = FNUniqueString;
  using mapped_type = FNValue;
  using value_type = std::pair<FNUniqueString, FNValue>;

  FNPropMap();
  ~FNPropMap() noexcept;

  /// Return a pointer to the pair with the specified key, or nullptr.
  value_type *findOrNull(FNUniqueString key) {
    auto [found, it] = lookup(key);
    return found ? it : nullptr;
  }

  /// Erase the specified entry. Do nothing if pos is nullptr.
  void erase(value_type *pos);

  /// Overwrite the value, if the key is present, or insert a new key/value
  /// pair.
  void assign(FNUniqueString key, FNValue value);

 private:
  using size_type = uint32_t;
  static constexpr size_type kSmallSize = 4;

  bool isSmall() const {
    return capacity_ == kSmallSize;
  }

  /// Search the hash table for \p key. If found, return true and the
  /// and a pointer to the pair. If not found, return false and a pointer
  /// to the pair where it ought to be inserted.
  std::pair<bool, value_type *> lookup(FNUniqueString key);

 private:
  /// Capacity of the hash table. Always a power of 2.
  size_type capacity_ = kSmallSize;
  /// Number of occupied slots, including deleted.
  size_type occupiedSlots_ = 0;
  /// We need to grow the table if occupiedSlots_ would exced this limit.
  size_type limit_ = kSmallSize / 4 * 3;
  /// The table itself.
  value_type *data_;
  /// "Inline" storage for small tables.
  alignas(value_type) char small_[sizeof(value_type) * kSmallSize];
};

struct FNString {
  fn_string str;

  void *operator new(size_t sz) {
    return fnMalloc(sz);
  }
};
struct FNObject {
  FNPropMap props{};
  FNObject *parent{};

  FNValue getByName(FNUniqueString key);
  void putByName(FNUniqueString key, FNValue val);
  FNValue getByVal(FNValue key);
  void putByVal(FNValue key, FNValue val);

  void *operator new(size_t sz) {
    return fnMalloc(sz);
  }
};
struct FNClosure : public FNObject {
  explicit FNClosure(void (*func)(void), void *env) : func(func), env(env) {
    props.assign(
        FNPredefined::prototype, FNValue::encodeObject(new FNObject()));
  }

  void (*func)(void);
  void *env;
};
struct FNArray : public FNObject {
  explicit FNArray(fn_vector<FNValue> arr) : arr(std::move(arr)) {}

  fn_vector<FNValue> arr;
};

inline FNClosure *FNValue::getClosure() const {
  assert(isClosure());
  return static_cast<FNClosure *>(value.obj);
}
inline FNValue FNValue::encodeClosure(FNClosure *closure) {
  FNValue ret;
  ret.tag = FNType::Closure;
  ret.value.obj = closure;
  return ret;
}

FNObject *global();

int32_t truncateToInt32SlowPath(double d);

/// Convert a double to a 32-bit integer according to ES5.1 section 9.5.
/// It can also be used for converting to an unsigned integer, which has the
/// same bit pattern.
/// NaN and Infinity are always converted to 0. The rest of the numbers are
/// converted to a (conceptually) infinite-width integer and the low 32 bits of
/// the integer are then returned.
int32_t truncateToInt32(double d);
inline int32_t truncateToInt32(double d) {
  // Check of the value can be converted to integer without loss. We want to
  // use the widest available integer because this conversion will be much
  // faster than the bit-twiddling slow path.
  intmax_t fast = (intmax_t)d;
  if (fast == d)
    return (int32_t)fast;
  return truncateToInt32SlowPath(d);
}

/// A table of unique strings.
class FNStringTable {
 public:
  FNStringTable();
  ~FNStringTable() noexcept;

  /// Return an existing unique string index or create a new one.
  FNUniqueString uniqueString(std::string_view s);

  // Return the FNString corresponding to a unique string index;
  const FNString *fnString(FNUniqueString index) {
    assert(
        index > FNPredefined::_DELETED && index < strings_.size() &&
        "Invalid unique string index");
    return strings_[index];
  }

 private:
  std::unordered_map<std::string_view, FNUniqueString> map_{};
  fn_vector<const FNString *> strings_{};
};

/// Global unique string table per runtime.
extern FNStringTable g_fnStringTable;

/// Map from a compile-time string index to a runtime unique string index.
/// Ordinarily this would be per compile-time unit.
extern fn_vector<FNUniqueString> g_fnCompilerStrings;

/// Obtain the unique string of an existing compiler string.
inline FNUniqueString fnCompilerUniqueString(unsigned i) {
  assert(i < g_fnCompilerStrings.size() && "Invalid compiler string index");
  return g_fnCompilerStrings[i];
}

/// Obtain the FNString of an existing compiler string.
inline const FNString *fnCompilerFNString(unsigned i) {
  return g_fnStringTable.fnString(fnCompilerUniqueString(i));
}

/// Register a new, possibly non-unique, compiler string.
/// The expected parameter enforces that the compiler notion of indexes is
/// correct.
inline void fnAddCompilerString(std::string_view str, unsigned expected) {
  g_fnCompilerStrings.push_back(g_fnStringTable.uniqueString(str));
  assert(
      expected == g_fnCompilerStrings.size() - 1 &&
      "unexpected compiler string index");
  (void)expected;
}

#endif // FNRUNTIME_H
