---
id: coding-standards
title: Coding Standards
---

## Hermes Coding Standards

This document provides guidance for the kind of code that should go in to
the Hermes project. The rules in this document will allow us to scale the
project and ensure that the code base remains readable and maintainable.

### Language Features

Hermes uses C++14.

Exceptions or RTTI are prohibited, except where needed for
interfacing with other software that requires them.

GCC language extensions are discouraged for compatibility with MSVC, or they
should be conditionally enabled.

### Code Style

TODO: This should be sorted and expanded.

- Every declaration must have a doc-comment.

Naming

- Member variables use a "\_" suffix unless they are public, where it is
acceptable to omit the suffix.
- Classes use pascal case (`MyClass`).
- Functions and methods use camel case (`myFunc`).
- Variables generally use camel case (`myVar`). We also allow "abbreviated
case": "basic block" can be abbreviated as `BB`, "instruction" can be
abbreviated as `I`, etc.
- Constants use either capitalized snake case (`MY_CONST`) or camel case with
"k" prefix (`kMyConst`).

Misc

- `struct` should be used only when it is a true POD - there are no constructors
or destructors of any kind. For everything else use `class`.
- Inlining: Only completely trivial (one line) methods can be defined in the
class body. The rest should be declared as `inline` both at the declaration and
the definition.

### Small incremental changes

The project is developed using small incremental changes. These changes can be
small bug fixes or minor tweaks. Other times, these changes are small steps
along the path to reaching larger stated goals. Long-term development branches
suffer from many problems, including the lack of visibility, difficulty of code
review, lack of testing of the branch and merge difficulty.

Commits that go into the project need to be reviewable. This means that commits
need to be relatively small, well documented and self contained.

### Add tests

Functional changes to the compiler need to include a testcase. Unit tests and
regression tests are critical to the qualification of the compiler. Every bug
fix needs to include a testcase.

Reduce test cases as much as possible! It is unacceptable to commit big programs
because they do not describe the essence of the failure, they are fragile, and
they slow testing down. Tests need to be short and focused.

### Format your code

clang-format is required to enforce code style and formatting.
Commits that only change the formatting of code should go in independent of
functional changes.

### Commit messages

Here are some guidelines about the format of the commit message:

Separate the commit message into a single-line title and a separate body that
describes the change. Make the title short (80 chars) and readable.  In changes
that are restricted to a specific part of the code, include a [tag] at the start
of the line in square brackets—for example, “[docs] ... ”.

If the commit fixes an issue in the bug tracking system, include a link or a
task number.

When reverting a change make sure to add a short note that describes why the
patch is being reverted.


### Code review

The project relies heavily on code review to maintain the software quality.

Review other people’s changes! Anybody is allowed to review code and comment
on patches.

All changes, by all developers, must be reviewed before they are committed to
the repository. Smaller changes (if the developer is the de-facto owner of the
code base) can be reviewed after being committed.
