---
id: vm
title: VM
---


## Hermes VM

In which the general design of the Hermes VM is explained.

## Value Representation

The VM uses a class called `HermesValue` to encapsulate JS values efficiently,
preserving their type while still allowing them to fit in a register.
NaN-tagging is used to store different types of values;
we store values in the lower bits of a `uint64_t`.
Thus, when the `uint64_t` is interpreted as a `double`,
tagged `NaN` values can hold non-`double` types.

### Strings

`StringPrimitive` is used to store immutable UTF16 encoded strings,
and `StringPrimitive *` can be stored in `HermesValue` to make JS String values.

## Runtime

The `Runtime` class is the primary driver of the VM.
It contains the current environment and heap, as well as the code to execute.
`Runtime` is used to execute `RuntimeModule`s,
which are constructed from `BytecodeModule`s using `Runtime::runModule()`.

### Runtime Module

TODO: Explain the ownership model of the RuntimeModule here.

### Runtime Identifiers

The `Runtime` contains an `IdentifierTable`,
which is used for getting unique IDs for strings.
The table is used to go from `StringPrimitive` to `IdentifierID` and back.
It's prepopulated with some "predefined strings",
the set of strings that are required by built in functions,
which can be seen in `PredefinedStrings.def`.

### Garbage Collection

Currently, the VM uses `SemiSpaceGC` for its garbage collection needs.
The garbage collector allocates two sections of memory;
on collection, it moves all live cells from one section to another,
with the exception of values stored as `PinnedHermesValue`
(mainly used for global objects stored in the `Runtime` itself).
The garbage collector is precise
(it knows what `HermesValue`s are valid pointers to objects in the JS heap).

TODO: Elaborate on the garbage collector requirements and future plans.

The garbage collector moves objects to different place on the heap,
invalidating `HermesValue`s, so there are a couple "scoped" value classes.
`Handle<>` and `Handle<T>` are garbage collector-aware handles;
they are moved if a collection occurs in between two successive accesses.
So, to ensure correctness in the VM,
use the handles instead of passing raw `HermesValue` between functions.

A `GCScope` is used to keep track of all the current `HermesValue` handles.
Any `GCScope` must be constructed on the stack,
whence it tracks any scoped handles that are used until it falls out of scope.
The `GCScope` allocates space in chunks,
and when it is destroyed (falls out of scope) it frees any chunks it allocated.
The `GCScope` is used to internally generate `PinnedHermesValue`s,
which are then stored in `Handle<>` and `Handle<T>`.

#### Rules for using handles

1. A function that can perform an allocation (even if it doesn't do it every
   time) or calls a function that does, must accept and return only handles
   (for GC-managed objects).
2. A function that accepts or returns handles is allowed (and can be assumed
   to) allocate more handles, but the upper bound of allocated handles must be
   static.
3. The number of handles in a given GCScope should have a static upper limit.

The motivation for these rules should be self-explanatory.  The practical
implication of rule 2 and 3 is that recursion and loops that allocate handles
in every iteration must be treated specially.  In case of recursion a new
GCScope should be defined in each recurrence (is that the correct term?).  In
case of a loop, there are a couple of possibilities:

- in loops that are expected to be low iteration and not performance critical,
  a new GCScope can be defined in the body of the loop.
- otherwise a GCScope::Marker should be used to flush the allocated handles of
  the previous iteration.
- mutable handles can be used to avoid allocating a new handle on every
  iteration.

## Object Model

Currently the object model is a VTable-based scheme,
in which all possible JS values inherit from a base garbage collector VTable.
These are called "cells", and all the cells are defined in `CellKinds.def`.
Objects have a special `ObjectVTable`.

### Objects

Each JS object is represented by `Object` (or a class derived from `Object`).
JS objects have a set of name/value pairs, and some optional "indexed storage".
Read more about how `Object` works in `ObjectModel.h`.
The Runtime contains a global object which is used to store in global scope.

### Arrays

Arrays, the `arguments` object, etc. inherit from Object directly,
but simply provide their own implementations of `*OwnIndexed` using the VTable.

### Functions

Functions and native functions inherit from `Callable`.
This allows them to call `executeCall*` to run functions using the internal API.

### Boxed Primitives

The `PrimitiveBox` class is used to contain Booleans, Strings, and Numbers,
when they are constructed using their respective JS constructors.
`JSString` is a `PrimitiveBox` that is used for `String` objects, etc.

## REPL

The HermesVM provides a REPL in `bin/hermes-repl`,
which calls through to the `eval()` global function in the `Runtime`.
