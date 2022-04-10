/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSONPARSER_H
#define HERMES_PARSER_JSONPARSER_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <utility>

#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/pack.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/JSONEmitter.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/FoldingSet.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/Casting.h"

namespace hermes {
namespace parser {

using llvh::StringRef;

class JSONFactory;
class JSONParser;

enum class JSONKind {
  Object,
  Array,
  String,
  Number,
  Boolean,
  Null,
};

/// Converts JSONKind to a char*.
const char *JSONKindToString(JSONKind kind);

/// The base type for all values that can be represented in JSON.
class JSONValue {
  JSONKind const kind_;

  JSONValue(const JSONValue &) = delete;
  JSONValue &operator=(const JSONValue &) = delete;

 public:
  explicit constexpr JSONValue(JSONKind kind) : kind_(kind){};

  JSONKind getKind() const {
    return kind_;
  }

  /// Writes this JSON value into \p emitter.
  void emitInto(JSONEmitter &emitter) const;
};

/// The base type for all values except arrays and objects.
class JSONScalar : public JSONValue {
 public:
  explicit constexpr JSONScalar(JSONKind kind) : JSONValue(kind) {}

  static bool classof(const JSONValue *v) {
    return v->getKind() >= JSONKind::String;
  }
};

class JSONNull : public JSONScalar {
  constexpr JSONNull() : JSONScalar(JSONKind::Null){};

  static JSONNull instance_;

 public:
  static JSONNull *getInstance() {
    return &instance_;
  }

  static bool classof(const JSONValue *v) {
    return v->getKind() == JSONKind::Null;
  }
};

class JSONBoolean : public JSONScalar {
  bool const value_;

  explicit constexpr JSONBoolean(bool value)
      : JSONScalar(JSONKind::Boolean), value_(value){};

  static JSONBoolean true_;
  static JSONBoolean false_;

 public:
  static JSONBoolean *getInstance(bool v) {
    return v ? &true_ : &false_;
  };

  bool getValue() const {
    return value_;
  }
  explicit operator bool() const {
    return getValue();
  }

  static bool classof(const JSONValue *v) {
    return v->getKind() == JSONKind::Boolean;
  }
};

class JSONString : public JSONScalar, public llvh::FoldingSetNode {
  UniqueString *const value_;

 public:
  explicit JSONString(UniqueString *value)
      : JSONScalar(JSONKind::String), value_(value) {}

  UniqueString *stringBase() const {
    return value_;
  }

  const StringRef &str() const {
    return value_->str();
  }
  explicit operator StringRef() const {
    return value_->str();
  }
  const char *c_str() const {
    return value_->c_str();
  }

  static void Profile(llvh::FoldingSetNodeID &id, UniqueString *str) {
    id.AddPointer(str);
  }

  void Profile(llvh::FoldingSetNodeID &id) {
    JSONString::Profile(id, value_);
  }

  static bool classof(const JSONValue *v) {
    return v->getKind() == JSONKind::String;
  }
};

class JSONNumber : public JSONScalar, public llvh::FoldingSetNode {
  double const value_;

 public:
  explicit JSONNumber(double value)
      : JSONScalar(JSONKind::Number), value_(value) {}

  double getValue() const {
    return value_;
  }
  explicit operator double() const {
    return getValue();
  }

  static void Profile(llvh::FoldingSetNodeID &id, double value) {
    id.AddInteger(llvh::DoubleToBits(value));
  }

  void Profile(llvh::FoldingSetNodeID &id) {
    JSONNumber::Profile(id, value_);
  }

  static bool classof(const JSONValue *v) {
    return v->getKind() == JSONKind::Number;
  }
};

/// A descriptor containing a sorted list of names. A JSON object contains
/// an array of values and a pointer to this descriptor.
class JSONHiddenClass {
  size_t size_;
  JSONString *keys_[];

  struct NameComparator {
    bool operator()(StringRef a, JSONString *b) const {
      return a < b->str();
    }
    bool operator()(JSONString *a, StringRef b) const {
      return a->str() < b;
    }
  };

  JSONHiddenClass(const JSONHiddenClass &) = delete;
  JSONHiddenClass &operator=(const JSONHiddenClass &) = delete;

  template <typename FwIt>
  JSONHiddenClass(size_t size, FwIt b, FwIt e) : size_(size) {
    std::copy(b, e, keys_);
  }

  // Allocator
  //
  template <class Allocator>
  static void *operator new(size_t size, Allocator &alloc, size_t count) {
    return alloc.Allocate(
        offsetof(JSONHiddenClass, keys_) + sizeof(keys_[0]) * count,
        alignof(JSONHiddenClass));
  }

  friend class JSONFactory;

 public:
  using iterator = hermes::parser::JSONString *const *;

  size_t size() const {
    return size_;
  }
  iterator begin() const {
    return keys_;
  }
  iterator end() const {
    return keys_ + size_;
  }

  llvh::Optional<size_t> find(StringRef name) {
    auto e = end();
    auto it = std::lower_bound(begin(), e, name, NameComparator{});
    if (it != e && (*it)->str() == name)
      return it - begin();
    else
      return llvh::None;
  }
};

/// A JSON object, which is a map from a string name to a JSONValue* value.
/// The names are stored in a separate JSONHiddenClass object. Multiple
/// instances of JSONObject which happen to have the same keys share one
/// JSONHiddenClass.
class JSONObject : public JSONValue {
  JSONHiddenClass *const hiddenClass_;

  inline JSONValue **values() {
    return Pack<JSONObject, JSONValue *>::values(this);
  }

  inline JSONValue *const *values() const {
    return Pack<JSONObject, JSONValue *>::values(this);
  }

  /// Initialize the object with a sequence of values, one value per member of
  /// the
  /// hidden class.
  template <typename FwId>
  JSONObject(JSONHiddenClass *hiddenClass, FwId b, FwId e)
      : JSONValue(JSONKind::Object), hiddenClass_(hiddenClass) {
    JSONValue *const *oe = std::copy(b, e, values());
    assert(oe - values() == (ptrdiff_t)hiddenClass_->size());
    (void)oe;
  }

  /// Initialize all members of the object with the same value.
  JSONObject(JSONHiddenClass *hiddenClass, JSONValue *fillValue)
      : JSONValue(JSONKind::Object), hiddenClass_(hiddenClass) {
    JSONValue **v = values();
    std::fill(v, v + hiddenClass->size(), fillValue);
  }

  // Allocator

  template <class Allocator>
  void *operator new(size_t size, Allocator &alloc, size_t count) {
    return Pack<JSONObject, JSONValue *>::allocate(alloc, count);
  }

  friend class JSONFactory;

 public:
  JSONHiddenClass *getHiddenClass() const {
    return hiddenClass_;
  }
  size_t size() const {
    return hiddenClass_->size();
  }

  /// Obtain a value, or return nullptr if not found.
  JSONValue *get(StringRef name) const {
    if (auto res = hiddenClass_->find(name))
      return values()[res.getValue()];
    else
      return nullptr;
  }

  /// Obtain a value. If the value is not found, debug builds assert; in release
  /// builds the behavior is undefined.
  JSONValue *at(StringRef name) const {
    if (auto res = hiddenClass_->find(name))
      return values()[res.getValue()];

    assert(false && "name not found");
    return nullptr;
  }

  /// Obtain a value by name, Behavior is undefined if the name is not found.
  JSONValue *operator[](StringRef name) const {
    return values()[hiddenClass_->find(name).getValue()];
  }
  /// Obtain a value by name, Behavior is undefined if the name is not found.
  JSONValue *&operator[](StringRef name) {
    return values()[hiddenClass_->find(name).getValue()];
  }
  /// Obtain a value by index.
  JSONValue *operator[](size_t index) const {
    assert(index < size());
    return values()[index];
  }
  /// Obtain a value by index.
  JSONValue *&operator[](size_t index) {
    assert(index < size());
    return values()[index];
  }

  /// Check for the presence of a key.
  size_t count(StringRef name) const {
    return hiddenClass_->find(name) ? 1 : 0;
  }

  /// Iterator creating the impression that we are storing key/value pairs.
  /// The illusion is not complete as "it->first" doesn't work, but it is better
  /// than nothing.
  class iterator {
    JSONObject *obj_;
    size_t index_;

    iterator(JSONObject *obj, size_t index) : obj_(obj), index_(index) {}
    friend class JSONObject;
    friend class const_iterator;

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair<JSONString *, JSONValue *&>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(const iterator &) = default;
    iterator &operator=(const iterator &) = default;

    value_type operator*() const {
      return value_type{obj_->hiddenClass_->begin()[index_], (*obj_)[index_]};
    }

    iterator &operator++() {
      ++index_;
      return *this;
    }

    iterator &operator--() {
      --index_;
      return *this;
    }

    difference_type operator-(const iterator &x) const {
      assert(obj_ == x.obj_);
      return (difference_type)(index_ - x.index_);
    }

    bool operator==(const iterator &it) const {
      return obj_ == it.obj_ && index_ == it.index_;
    }
    bool operator!=(const iterator &it) const {
      return obj_ != it.obj_ || index_ != it.index_;
    }
  };
  class const_iterator {
    const JSONObject *obj_;
    size_t index_;

    const_iterator(const JSONObject *obj, size_t index)
        : obj_(obj), index_(index) {}
    friend class JSONObject;

   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair<JSONString *, JSONValue *>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    const_iterator(const const_iterator &) = default;
    const_iterator &operator=(const const_iterator &) = default;

    explicit const_iterator(const JSONObject::iterator &it)
        : obj_(it.obj_), index_(it.index_) {}
    const_iterator &operator=(const JSONObject::iterator &it) {
      obj_ = it.obj_;
      index_ = it.index_;
      return *this;
    }

    value_type operator*() const {
      return value_type{obj_->hiddenClass_->begin()[index_], (*obj_)[index_]};
    }

    const_iterator &operator++() {
      ++index_;
      return *this;
    }

    const_iterator &operator--() {
      --index_;
      return *this;
    }

    difference_type operator-(const const_iterator &x) const {
      assert(obj_ == x.obj_);
      return (difference_type)(index_ - x.index_);
    }

    bool operator==(const const_iterator &it) const {
      return obj_ == it.obj_ && index_ == it.index_;
    }
    bool operator!=(const const_iterator &it) const {
      return obj_ != it.obj_ || index_ != it.index_;
    }
  };

  iterator begin() {
    return iterator(this, 0);
  }
  iterator end() {
    return iterator(this, size());
  }
  const_iterator begin() const {
    return const_iterator(this, 0);
  }
  const_iterator end() const {
    return const_iterator(this, size());
  }

  iterator find(StringRef name) {
    if (auto res = hiddenClass_->find(name))
      return iterator(this, res.getValue());
    else
      return end();
  }
  const_iterator find(StringRef name) const {
    if (auto res = hiddenClass_->find(name))
      return const_iterator(this, res.getValue());
    else
      return end();
  }

  static bool classof(const JSONValue *v) {
    return v->getKind() == JSONKind::Object;
  }
};

class JSONArray : public JSONValue {
  size_t const size_;

  inline JSONValue **values() {
    return Pack<JSONArray, JSONValue *>::values(this);
  }

  inline JSONValue *const *values() const {
    return Pack<JSONArray, JSONValue *>::values(this);
  }

  /// Initialize the array with a sequence of values.
  template <class FwdIt>
  JSONArray(size_t size, FwdIt b, FwdIt e)
      : JSONValue(JSONKind::Array), size_(size) {
    std::copy(b, e, values());
  }

  /// Initialize all elements of the array with the same value.
  JSONArray(size_t size, JSONValue *fillValue)
      : JSONValue(JSONKind::Array), size_(size) {
    JSONValue **v = values();
    std::fill(v, v + size_, fillValue);
  }

  // Allocator

  template <class Allocator>
  inline void *operator new(size_t size, Allocator &alloc, size_t count) {
    return Pack<JSONArray, JSONValue *>::allocate(alloc, count);
  }

  friend class JSONFactory;

 public:
  using iterator = const hermes::parser::JSONValue *const *;

  size_t size() const {
    return size_;
  }

  iterator begin() const {
    return values();
  };
  iterator end() const {
    return values() + size_;
  }

  const JSONValue *at(size_t pos) const {
    assert(pos < size_);
    return values()[pos];
  };

  const JSONValue *operator[](size_t pos) const {
    assert(pos < size_);
    return values()[pos];
  }

  static bool classof(const JSONValue *v) {
    return v->getKind() == JSONKind::Array;
  }
};

/// This class owns all the objects, takes care of uniquing, and so on.
/// It needs to be configured with a function for creating unique string
/// literals (which usually comes from JSLexer).
class JSONFactory {
 public:
  using Allocator = hermes::BumpPtrAllocator;

  /// Uniquely identifies a hidden class. The first element of the pair is the
  /// number
  /// of hidden class members while the second element is a pointer to an
  /// alphabetically
  /// sorted array of their names.
  using HiddenClassKey = std::pair<size_t, JSONString *const *>;

  /// A single property.
  using Prop = std::pair<JSONString *, JSONValue *>;

  struct LessHiddenClassKey {
    bool operator()(const HiddenClassKey &a, const HiddenClassKey &b) const;
  };

 private:
  Allocator &allocator_;

  /// If a StringTable was not supplied to us from outside, store our own
  /// table here.
  std::unique_ptr<StringTable> ownStrTab_;
  /// The StringLIteralTable to use: either supplied to us, or points to our own
  /// copy.
  StringTable &strTab_;

  // Unique the strings and numbers as there are likely to be many duplicates.
  llvh::FoldingSet<JSONString> strings_;
  llvh::FoldingSet<JSONNumber> numbers_;

  std::map<HiddenClassKey, JSONHiddenClass *, LessHiddenClassKey>
      hiddenClasses_;

 public:
  explicit JSONFactory(Allocator &allocator, StringTable *strTab = nullptr);

  Allocator &getAllocator() {
    return allocator_;
  }
  StringTable &getStringTable() {
    return strTab_;
  }

  // Methods for creating JSON objects. Numbers, strings and hidden classes are
  // "uniqued" - creating the same value multiple times returns a pointer to the
  // same object.

  JSONString *getString(UniqueString *lit);
  JSONString *getString(StringRef str);
  JSONNumber *getNumber(double value);
  static JSONBoolean *getBoolean(bool v) {
    return JSONBoolean::getInstance(v);
  }
  static JSONNull *getNull() {
    return JSONNull::getInstance();
  }

  /// Lookup or create a hidden class identified by the key \p key.
  JSONHiddenClass *getHiddenClass(const HiddenClassKey &key);

  /// Sort a sequence of properties in-place, so they can be passed directly
  /// to \c newObject(). If there are duplicates, return a pointer to the first
  /// duplicate name, otherwise return null.
  static JSONString *sortProps(Prop *from, Prop *to);

  /// Create an object described by the sequence of properties. The properties
  /// are sorted first, and if there are duplicates the operation aborts and
  /// returns nullptr.
  /// \param propsAreSorted indicates that the properties are already sorted
  ///   and checked for duplicates, so it doesn't need to be done again.
  JSONObject *newObject(Prop *from, Prop *to, bool propsAreSorted = false);

  /// Create a new object of a particular hidden class, initializing it with a
  /// sequence of values. The values must be in the same order as the elements
  /// of the hidden class, which are sorted alphabetically.
  template <typename FwIt>
  JSONObject *newObject(JSONHiddenClass *hiddenClass, FwIt b, FwIt e) const {
    return new (allocator_, hiddenClass->size()) JSONObject(hiddenClass, b, e);
  }
  /// Create a new object of a particular hidden class, setting all elements to
  /// the same value.
  JSONObject *newObject(
      JSONHiddenClass *hiddenClass,
      JSONValue *fillValue = JSONNull::getInstance()) const {
    return new (allocator_, hiddenClass->size())
        JSONObject(hiddenClass, fillValue);
  }

  /// Create a new array of given size, initializing it with a sequence of
  /// values. The number of values provided must be the same as the size of the
  /// array.
  template <class FwIt>
  JSONArray *newArray(size_t size, FwIt b, FwIt e) const {
    return new (allocator_, size) JSONArray(size, b, e);
  }
  /// Create a new array of given size, initializing all elements with the same
  /// value.
  JSONArray *newArray(
      size_t size,
      JSONValue *fillValue = JSONNull::getInstance()) const {
    return new (allocator_, size) JSONArray(size, fillValue);
  }
};

class JSONParser {
 private:
  JSONFactory &factory_;
  JSLexer lexer_;
  SourceErrorManager &sm_;

 public:
  JSONParser(
      JSONFactory &factory,
      std::unique_ptr<llvh::MemoryBuffer> input,
      SourceErrorManager &sm,
      bool convertSurrogates = false);

  JSONParser(
      JSONFactory &factory,
      StringRef input,
      SourceErrorManager &sm,
      bool convertSurrogates = false)
      : JSONParser(
            factory,
            llvh::MemoryBuffer::getMemBuffer(input, "json"),
            sm,
            convertSurrogates) {}

  JSONParser(
      JSONFactory &factory,
      llvh::MemoryBufferRef input,
      SourceErrorManager &sm,
      bool showColors = true,
      bool convertSurrogates = false)
      : JSONParser(factory, llvh::MemoryBuffer::getMemBuffer(input), sm) {}

  /// Parse the supplied input. On error the result will be empty and the error
  /// would have been reported to the SourceMgr.
  llvh::Optional<JSONValue *> parse();

  void error(const llvh::Twine &msg) {
    sm_.error(lexer_.getCurToken()->getSourceRange(), msg, Subsystem::Parser);
  }

 private:
  llvh::Optional<JSONValue *> parseValue();
  llvh::Optional<JSONValue *> parseArray();
  llvh::Optional<JSONValue *> parseObject();
};

/// A holder class for a JSONValue backed by a shared allocator.
class JSONSharedValue {
 public:
  using Allocator = hermes::BumpPtrAllocator;

 private:
  const JSONValue *value_;
  std::shared_ptr<const Allocator> allocator_;

 public:
  JSONSharedValue(
      const JSONValue *value,
      std::shared_ptr<const Allocator> allocator)
      : value_(value), allocator_(std::move(allocator)) {}

  const JSONValue *operator*() const {
    return value_;
  }

  const JSONValue *operator->() const {
    return value_;
  }
};

} // namespace parser
} // namespace hermes

#endif // HERMES_PARSER_JSONPARSER_H
