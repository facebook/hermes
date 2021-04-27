---
id: optimizer
title: Design of the Optimizer
---

### Introduction

This document describes the high-level design of the Hermes optimizer. The
Hermes optimizer transforms the Hermes IR into a more efficient representation
that preserves the original semantics of the program. The IR.md document describes
the design of the Hermes IR.

### Key concepts

This section describes a few key concepts and ideas:

  - The optimizer is responsible for optimizing the IR. IRGen and BytecodeGen
    are not the right place for implementing optimizations. The parts of the
    compiler that translate from one representation to another are inherently
    complex because they require the understanding of the semantics of both
    representations. Moreover, translators are not designed like optimizers.
    They do not have good access to analysis and do not allow the separation
    of the optimizer from the translation, which makes debugging more
    difficult.

  - Passes: Optimizations are organized in passes. There are two kinds of
    passes: function passes and module passes. Function passes can modify only
    the functions that they operate on, while module passes operate on the whole
    module.  Function passes are allowed to read the whole module but only
    touch the current function.

  - Analysis: Analyses are caches in front of a computation of some property.
    For example, the dominator analysis is a cache that helps reduce compile
    time by removing the need to recompute the dominator tree for each
    function. Analyses are all about caching and invalidating pre-computed
    properties.

  - Optimizations do one thing: Optimizations are designed to be simple and
    this means that they do only one thing. For example, the common
    subexpression elimination optimization does not delete dead code
    "on the way" just because it can.

  - Optimizations are predictable: Sometimes there are several legal
    representations of the program, but the optimizer should never
    randomize the output of the compiler. Randomization of the output
    happens when the output depends on runtime information such as the order
    of elements in a set or map. Randomizing the output of the compiler
    makes it very difficult to write tests and reproduce bugs. LLVM has
    data structures that provide guaranteed order - use them!

  - Write compile-time efficient algorithms: The compile time of a compiler is a
    very important metric and we attempt to minimize compile time as much as
    possible. Do not write exponential algorithms (or polynomial algorithm with
    a high degree). If you are writing a quadratic algorithm make sure to
    implement a sliding-window or other techniques that will allows to limit
    the quadratic search to a small subset of the graph. Always assume that
    there exist a function with hundreds of consecutive basic blocks or a basic
    block with thousands of instructions. If you are writing a "solver" then
    you are probably doing it wrong.

  - There are three kinds of transformations: canonicalization,
    simplification and lowering. Make sure that you know exactly what kind of
    transformation you are doing and why. Canonicalizations are transformations that
    expose opportunities for other transformations.  Re-association (reducing tree
    height, placing constants on the RHS, etc.) is a canonicalization because it
    organizes things in predictable patterns and makes the life of future
    optimizations simpler by reducing the number of possible inputs. Inlining is
    another example of effective canonicalization because it exposes opportunities
    for optimizations in the caller function (by providing more information).
    Another example is loop rotation, which is a canonical representation of all
    loops.  In canonicalization we strive to clean up the program as much as
    possible and reach a pure representation of the program.  Simplification is what
    we normally think of as optimizations, like removing redundancy by deleting dead
    code and optimizing arithmetic, etc.  Canonicalization can allow simplification
    that can allow more canonicalization.  For example, de-virtualization unblocks
    inlining that may allow some transformations that enable more de-virtualization.
    Lowering transformations are the opposite of canonicalization. In Lowering
    transformations we generate patterns that are closer to the target
    representation. We may not be able to recover from lowering transformations. One
    example for lowering transformation is loop strength reduction where the
    optimizer transforms loop indices into non-consecutive accesses that fit with
    the hardware instruction set. Another example is loop versioning where the body
    of the loop is duplicated and versioned multiple times .
