This tests performance when accessing properties in a parent class via
instances of different child classes. The same function is called instances
of different child classes, but only accessed fields in their common parent
class.

## many.js

This is the JS version of the code.

## many-sh-1.js

This is the SH version of the code, with type annotation. It can be compiled
and executed with the following command:
```
shermes -typed many-sh-1.js && ./a.out
```

## many-sh-2.js

Is a version of the benchmark where we have prevented inlinling of the `bench()`
function by returning it, which creates a second reference. The goal is to
examine the quality of the code in the non-inlined function.

A problem immediately become apparent: the types of `N` and `res` are not
`number`, which forces emitting of slower out-of-line instructions.

Another problem is that `N` is not treated as a constant.


The reason for this are two-fold:
- We still don't fully type variables. That code is coming soon.
- Type inference can't do its job, because the variables are initialized too
late - in the middle of the function. That means there is a period of time
where both are initialized to `undefined` (without TDZ; TDZ doesn't change
the picture here). We can't provde that `bench()` isn't called at that time
and doesn't see the `undefined`.

## many-sh-3.js

It improves on `many-sh-2.js` by moving the initialization of `res` and `N` to
the top. Now `N` is finally treated as a constant, and we don't see `undefined`
as a valid type for `res`.

However another problem becomes apparent: we still infer that `res` can be a
string.

## many-sh-4.js

It improves on the previous one by marking `bench()` as inline. This allows us
to inline the invocation, while also keeping the body around. This is slightly
faster, but still nowhere as performant as the fully inlined version.
