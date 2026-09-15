/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <utility>

/// Defines a new class, called \p NAME representing a constructor config, and
/// an associated builder class.
///
/// The ABI version is encoded in \p VERSION. The fields of the class (along
/// with their types and default values) are encoded in the \p FIELDS parameter,
/// and any logic to be run whilst building the config can be passed as a code
/// block in \p BUILD_BODY.
///
/// Fields may be appended without changing \p VERSION. Any other change that
/// affects the layout, including changing the layout of a field stored by
/// value, requires a new version.
///
/// Example:
///
///   Suppose we wish to define a configuration class called Foo, with the
///   following fields and default values:
///
///       int A = 0;
///       int B = 42;
///       std::string C = "hello";
///
///   Such that the value in A is at most the length of \c C.
///
///   We can do so with the following declaration:
///
///   "    #define FIELDS(F)                     \  "
///   "      F(int, A)                           \  "
///   "      F(int, B, 42)                       \  "
///   "      F(std::string, C, "hello")             "
///   "                                             "
///   "    _HERMES_CTORCONFIG_STRUCT(Foo, 1, FIELDS, { "
///   "        A_ = std::min(A_, C_.length());      "
///   "      });                                    "
///
///   N.B.
///     - The definition of A does not mention any value -- meaning it is
///       default initialised.
///     - References to the fields in the validation logic have a trailling
///       underscore.
///
#define _HERMES_CTORCONFIG_STRUCT(NAME, VERSION, FIELDS, BUILD_BODY)    \
  class NAME {                                                          \
    enum class FieldIndex : uint32_t {                                  \
      FIELDS(_HERMES_CTORCONFIG_FIELD_INDEX) count,                     \
    };                                                                  \
                                                                        \
    static constexpr uint32_t kVersion = VERSION;                       \
    static constexpr uint32_t kNumFields =                              \
        static_cast<uint32_t>(FieldIndex::count);                       \
                                                                        \
    uint32_t version_{kVersion};                                        \
    uint32_t numFields_{kNumFields};                                    \
    FIELDS(_HERMES_CTORCONFIG_FIELD_DECL)                               \
                                                                        \
    bool containsField(FieldIndex field) const {                        \
      return version_ == kVersion &&                                    \
          numFields_ > static_cast<uint32_t>(field);                    \
    }                                                                   \
                                                                        \
    void copyFieldsFrom(const NAME &other) {                            \
      FIELDS(_HERMES_CTORCONFIG_COPY_FIELD)                             \
    }                                                                   \
                                                                        \
    void moveFieldsFrom(NAME &other) {                                  \
      FIELDS(_HERMES_CTORCONFIG_MOVE_FIELD)                             \
    }                                                                   \
                                                                        \
   public:                                                              \
    NAME() = default;                                                   \
                                                                        \
    NAME(const NAME &other) {                                           \
      copyFieldsFrom(other);                                            \
    }                                                                   \
                                                                        \
    NAME &operator=(const NAME &other) {                                \
      copyFieldsFrom(other);                                            \
      version_ = kVersion;                                              \
      numFields_ = kNumFields;                                          \
      return *this;                                                     \
    }                                                                   \
                                                                        \
    NAME(NAME &&other) {                                                \
      moveFieldsFrom(other);                                            \
    }                                                                   \
                                                                        \
    NAME &operator=(NAME &&other) {                                     \
      if (this != &other) {                                             \
        moveFieldsFrom(other);                                          \
      }                                                                 \
      version_ = kVersion;                                              \
      numFields_ = kNumFields;                                          \
      return *this;                                                     \
    }                                                                   \
                                                                        \
    class Builder;                                                      \
    friend Builder;                                                     \
                                                                        \
    /** Return the ABI version recorded in this config. */              \
    uint32_t getVersion() const {                                       \
      return version_;                                                  \
    }                                                                   \
                                                                        \
    /** Return the recorded field count, excluding metadata. */         \
    uint32_t getNumFields() const {                                     \
      return numFields_;                                                \
    }                                                                   \
                                                                        \
    /** Return the ABI version expected by this definition. */          \
    static constexpr uint32_t getCurrentVersion() {                     \
      return kVersion;                                                  \
    }                                                                   \
                                                                        \
    /** Return the declared field count, excluding metadata. */         \
    static constexpr uint32_t getCurrentNumFields() {                   \
      return kNumFields;                                                \
    }                                                                   \
                                                                        \
    FIELDS(_HERMES_CTORCONFIG_GETTER)                                   \
                                                                        \
    /* returns a Builder that starts with the current config. */        \
    inline Builder rebuild() const;                                     \
                                                                        \
   private:                                                             \
    inline void doBuild(const Builder &builder);                        \
  };                                                                    \
                                                                        \
  class NAME::Builder {                                                 \
    NAME config_;                                                       \
                                                                        \
    FIELDS(_HERMES_CTORCONFIG_FIELD_EXPLICIT_BOOL_DECL)                 \
                                                                        \
   public:                                                              \
    Builder() = default;                                                \
                                                                        \
    explicit Builder(const NAME &config) : config_(config) {}           \
                                                                        \
    inline const NAME build() {                                         \
      config_.doBuild(*this);                                           \
      return config_;                                                   \
    }                                                                   \
                                                                        \
    /* The explicitly set fields of \p newconfig update                 \
     * the corresponding fields of \p this. */                          \
    inline Builder update(const NAME::Builder &newConfig);              \
                                                                        \
    FIELDS(_HERMES_CTORCONFIG_SETTER)                                   \
    FIELDS(_HERMES_CTORCONFIG_FIELD_EXPLICIT_BOOL_ACCESSOR)             \
  };                                                                    \
                                                                        \
  NAME::Builder NAME::rebuild() const {                                 \
    return Builder(*this);                                              \
  }                                                                     \
                                                                        \
  NAME::Builder NAME::Builder::update(const NAME::Builder &newConfig) { \
    FIELDS(_HERMES_CTORCONFIG_UPDATE)                                   \
    return *this;                                                       \
  }                                                                     \
                                                                        \
  void NAME::doBuild(const NAME::Builder &builder) {                    \
    (void)builder;                                                      \
    BUILD_BODY                                                          \
  }

/// Helper Macros

#define _HERMES_CTORCONFIG_FIELD_INDEX(CX, TYPE, NAME, ...) NAME,

#define _HERMES_CTORCONFIG_FIELD_DECL(CX, TYPE, NAME, ...) \
  TYPE NAME##_{__VA_ARGS__};

#define _HERMES_CTORCONFIG_COPY_FIELD(CX, TYPE, NAME, ...) \
  NAME##_ = other.get##NAME();

#define _HERMES_CTORCONFIG_MOVE_FIELD(CX, TYPE, NAME, ...) \
  if (other.containsField(FieldIndex::NAME)) {             \
    NAME##_ = std::move(other.NAME##_);                    \
  } else {                                                 \
    NAME##_ = getDefault##NAME();                          \
  }

/// This ignores the first and trailing arguments, and defines a member
/// indicating whether field NAME was set explicitly.
#define _HERMES_CTORCONFIG_FIELD_EXPLICIT_BOOL_DECL(CX, TYPE, NAME, ...) \
  bool NAME##Explicit_{false};

/// This defines an accessor for the "Explicit_" fields defined above.
#define _HERMES_CTORCONFIG_FIELD_EXPLICIT_BOOL_ACCESSOR(CX, TYPE, NAME, ...) \
  bool has##NAME() const {                                                   \
    return NAME##Explicit_;                                                  \
  }

/// Placeholder token for fields whose defaults are not constexpr, to make the
/// listings more readable.
#define HERMES_NON_CONSTEXPR

#define _HERMES_CTORCONFIG_GETTER(CX, TYPE, NAME, ...) \
  inline TYPE get##NAME() const {                      \
    if (!containsField(FieldIndex::NAME)) {            \
      return getDefault##NAME();                       \
    }                                                  \
    return NAME##_;                                    \
  }                                                    \
  static CX TYPE getDefault##NAME() {                  \
    /* Instead of parens around TYPE (non-standard) */ \
    using TypeAsSingleToken = TYPE;                    \
    return TypeAsSingleToken{__VA_ARGS__};             \
  }

#define _HERMES_CTORCONFIG_SETTER(CX, TYPE, NAME, ...)   \
  inline auto with##NAME(TYPE NAME) -> decltype(*this) { \
    config_.NAME##_ = std::move(NAME);                   \
    NAME##Explicit_ = true;                              \
    return *this;                                        \
  }

#define _HERMES_CTORCONFIG_BUILDER_GETTER(CX, TYPE, NAME, ...) \
  TYPE get##NAME() const {                                     \
    return config_.NAME##_;                                    \
  }

#define _HERMES_CTORCONFIG_UPDATE(CX, TYPE, NAME, ...) \
  if (newConfig.has##NAME()) {                         \
    with##NAME(newConfig.config_.get##NAME());         \
  }
