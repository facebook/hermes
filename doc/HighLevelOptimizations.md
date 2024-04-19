---
id: high-level-optimizations
title: High Level Optimizations
---

JS authors frequently use high level library functions which they expect to perform efficiently.
They call these in conjunction with other library calls to write code which has similarities to
functional programming languages.

Executing this kind of code in Hermes can be slow due to the compiler not having proper visibility
into the JS library. However, we can get some information about what is being called in certain
scenarios, such as with `-fstatic-builtins` or in actual typed SH code.

This document is a place to put some ideas on how to make specific scenarios more efficient
in the future. It is possible we'll just happen to optimize some cases here as a result of good
type information in SH and a solid optimization pipeline.

# Spreading and modifying existing structures

An example of some sample code found in input to Hermes to clone a Map and add an element:
```
return new Map([...Array.from(origMap), [id, obj]]);
```

1. Iterates `origMap` and creates a new array `%tmp1`, element by element.
2. Creates a new array `%tmp2`, iterates `%tmp1`, and copies the elements.
3. Adds the new array `[id, obj]` to `%tmp2`.
4. Iterates `%tmp2` and populates a newly created `Map`.

Some potential ways to simplify things:

1. `...Array.from(a)` is the same as `...a` provided that the array iterator has not been modified.
2. `new Map([...oldMap, [x, y]])` is the same as `const %tmp = new Map(oldMap); %tmp.set(x, y)`
  provided that the `Map` iterator has not been modified.

If we can enforce that iterators haven't been modified, then we can do step 1 potentially even in
legacy code, and step 2 in typed code.
