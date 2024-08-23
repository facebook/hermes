// Broken - Lightweight unit testing for C++
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org>

#ifndef BROKEN_H_INCLUDED
#define BROKEN_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

// Hide everything when using Doxygen. Ideally this can be protected by a macro,
// but there is not globally and widely used one across multiple projects.

//! \cond

// Broken - API
// ============

namespace BrokenAPI {

//! Entry point of a unit test defined by `UNIT` macro.
typedef void (*Entry)(void);

enum Flags : unsigned {
  kFlagFinished = 0x1
};

struct Unit;

bool hasArg(const char* name) noexcept;

//! Register a new unit test (called automatically by `AutoUnit` and `UNIT`).
void addUnit(Unit* unit) noexcept;

//! Set output file to a `file`.
void setOutputFile(FILE* file) noexcept;

//! Initialize `Broken` framework.
//!
//! Returns `true` if `run()` should be called.
int run(int argc, const char* argv[], Entry onBeforeRun = nullptr, Entry onAfterRun = nullptr);

//! Log message, adds automatically new line if not present.
void info(const char* fmt, ...) noexcept;

//! Called on `EXPECT()` failure.
void fail(const char* file, int line, const char* expression, const char* fmt, ...) noexcept;

//! Test defined by `UNIT` macro.
struct Unit {
  Entry entry;
  const char* name;
  int priority;
  unsigned flags;
  Unit* next;
};

//! Automatic unit registration by using static initialization.
class AutoUnit : public Unit {
public:
  inline AutoUnit(Entry entry_, const char* name_, int priority_ = 0, int dummy_ = 0) noexcept {
    // Not used, only to trick `UNIT()` macro.
    (void)dummy_;

    this->entry = entry_;
    this->name = name_;
    this->priority = priority_;
    this->flags = 0;
    this->next = nullptr;
    addUnit(this);
  }
};

class Failure {
public:
  const char* _file = nullptr;
  const char* _expression = nullptr;
  int _line = 0;
  bool _handled = false;

  inline Failure(const char* file, int line, const char* expression) noexcept
    : _file(file),
      _expression(expression),
      _line(line) {}

  inline ~Failure() noexcept {
    if (!_handled)
      fail(_file, _line, _expression, nullptr);
  }

  template<typename... Args>
  inline void message(const char* fmt, Args&&... args) noexcept {
    fail(_file, _line, _expression, fmt, std::forward<Args>(args)...);
    _handled = true;
  }
};

template<typename Result>
static inline bool check(Result&& result) noexcept { return !!result; }

template<typename LHS, typename RHS>
static inline bool checkEq(LHS&& lhs, RHS&& rhs) noexcept { return lhs == rhs; }

template<typename LHS, typename RHS>
static inline bool checkNe(LHS&& lhs, RHS&& rhs) noexcept { return lhs != rhs; }

template<typename LHS, typename RHS>
static inline bool checkGt(LHS&& lhs, RHS&& rhs) noexcept { return lhs >  rhs; }

template<typename LHS, typename RHS>
static inline bool checkGe(LHS&& lhs, RHS&& rhs) noexcept { return lhs >= rhs; }

template<typename LHS, typename RHS>
static inline bool checkLt(LHS&& lhs, RHS&& rhs) noexcept { return lhs <  rhs; }

template<typename LHS, typename RHS>
static inline bool checkLe(LHS&& lhs, RHS&& rhs) noexcept { return lhs <= rhs; }

template<typename Result>
static inline bool checkTrue(Result&& result) noexcept { return !!result; }

template<typename Result>
static inline bool checkFalse(Result&& result) noexcept { return !result; }

template<typename Result>
static inline bool checkNull(Result&& result) noexcept { return result == nullptr; }

template<typename Result>
static inline bool checkNotNull(Result&& result) noexcept { return result != nullptr; }

} // {BrokenAPI}

// Broken - Macros
// ===============

//! Internal macro used by `UNIT()`.
#define BROKEN_UNIT_INTERNAL(NAME, PRIORITY) \
  static void unit_##NAME##_entry(void); \
  static ::BrokenAPI::AutoUnit unit_##NAME##_autoinit(unit_##NAME##_entry, #NAME, PRIORITY); \
  static void unit_##NAME##_entry(void)

//! \def UNIT(NAME [, PRIORITY])
//!
//! Define a unit test with an optional priority.
//!
//! `NAME` can only contain ASCII characters, numbers and underscore. It has the same rules as identifiers in C and C++.
//!
//! `PRIORITY` specifies the order in which unit tests are run. Lesses value increases the priority. At the moment all
//!units are first sorted by priority and then by name - this makes the run always deterministic.
#define UNIT(NAME, ...) BROKEN_UNIT_INTERNAL(NAME, __VA_ARGS__ + 0)

//! #define INFO(FORMAT [, ...])
//!
//! Informative message printed to `stdout`.
#define INFO(...) ::BrokenAPI::info(__VA_ARGS__)

#define BROKEN_EXPECT_INTERNAL(file, line, expression, result) \
  for (bool _testInternalResult = (result); !_testInternalResult; _testInternalResult = true) \
    ::BrokenAPI::Failure(file, line, expression)

#define EXPECT(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT(" #__VA_ARGS__ ")", !!(__VA_ARGS__))
#define EXPECT_EQ(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_EQ(" #__VA_ARGS__ ")", ::BrokenAPI::checkEq(__VA_ARGS__))
#define EXPECT_NE(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_NE(" #__VA_ARGS__ ")", ::BrokenAPI::checkNe(__VA_ARGS__))
#define EXPECT_GT(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_GT(" #__VA_ARGS__ ")", ::BrokenAPI::checkGt(__VA_ARGS__))
#define EXPECT_GE(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_GE(" #__VA_ARGS__ ")", ::BrokenAPI::checkGe(__VA_ARGS__))
#define EXPECT_LT(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_LT(" #__VA_ARGS__ ")", ::BrokenAPI::checkLt(__VA_ARGS__))
#define EXPECT_LE(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_LE(" #__VA_ARGS__ ")", ::BrokenAPI::checkLe(__VA_ARGS__))
#define EXPECT_TRUE(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_TRUE(" #__VA_ARGS__ ")", ::BrokenAPI::checkTrue(__VA_ARGS__))
#define EXPECT_FALSE(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_FALSE(" #__VA_ARGS__ ")", ::BrokenAPI::checkFalse(__VA_ARGS__))
#define EXPECT_NULL(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_NULL(" #__VA_ARGS__ ")", ::BrokenAPI::checkNull(__VA_ARGS__))
#define EXPECT_NOT_NULL(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_NOT_NULL(" #__VA_ARGS__ ")", ::BrokenAPI::checkNotNull(__VA_ARGS__))

//! \endcond

#endif // BROKEN_H_INCLUDED
