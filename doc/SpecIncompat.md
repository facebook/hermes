# Incompatibilities With The JavaScript Spec

This documents aims to collect and explain deliberate differences between Static Hermes and the JavaScript spec.

## Arguments Object

### "Mapped" Arguments Object in Loose Mode

In loose mode, updates to the arguments object are not reflected in the function parameter values and vice versa.
```javascript
function foo(param) {
    arguments[0] = 10;
    print(param, arguments[0]); // Prints different values in SH
}
```

Motivation: this is a very rare case. Implementing it correctly is expensive and would impose cost on the majority of usages which do not depend on this behavior. Optimizing it, so it doesn't regress legitimate cases is possible, but complex.

This is "implementable", but with very low priority.

### Assigning to Arguments in Loose Mode

Assignment to `arguments` is prohibited in loose mode.
```javascript
function foo() {
    arguments = 0; // Allowed by the spec, but prohibited in Static Hermes
}
```

Motivation: this is a very rare case, not generally useful. Implementing it correctly is possible, but complex, with very little benefit.

This is "implementable", but with very low priority.

### Aliasing Arguments With a Var Declaration in Loose Mode

In loose mode declaring `var arguments` does not alias the arguments object. Instead it simply creates a new declaration that *shadows* the arguments object, similar to `let`.
```javascript
function foo() {
    var arguments;
    print(arguments); // Prints undefined in Static Hermes
}
```

Motivation: similar to assigning to `arguments`, this is a very rare case, with no uses that we are aware of.

This is "implementable", but with very low priority.

## `String.prototype.localeCompare` Ignores the Locale

`localeCompare` ignores its `locales` and `options` arguments. Its ordering is the DUCET root collation of Unicode Technical Standard #10, not the host locale's.

```javascript
// Swedish sorts "z" before "ä"; the root collation does not.
print("ä".localeCompare("z", "sv")); // Prints -1 in Static Hermes
```

ECMA-262 permits this: without ECMA-402, `localeCompare` need only be a consistent comparison function. It is still worth knowing about, and it is a change from earlier versions, which forwarded to ICU and so did pick up the host locale on Linux and Windows.

Two further properties of the ordering:

- It is plain DUCET, not the CLDR root that ICU and the platform collators use. CLDR retailors the root in a few places, so some pairs sort differently on Android and Apple, which still use their platform collators.
- Comparison stops at the tertiary level; there is no identical level. Canonically equivalent strings compare equal, as do strings differing only by a completely ignorable character such as U+0000.

Motivation: locale-tailored collation means shipping CLDR tailoring data and a locale database. The root collation table is about 110 KB, needs no system library, and removes the last dependency on ICU for string comparison.

## Full Scoped Function Promotion Semantics in Loose Mode

Static Hermes implements most of the scoped function promotion semantics in loose mode, but some corner cases are not spec compliant yet.

```javascript
function g() {
    {
        function f() { return 1; }
        {
            function f() { return 2; }
        }
    }
    print(f());
}
g();
```

Apparently the spec requires this example to print "1", because the second `f()` shouldn't be promoted to function scope. Shermes prints "2". In our (quick and incomplete) tests, other major engines also print "2".

Motivation: these are very rare cases. Loose mode itself is rare. While we intend to address them, the priority is low.
