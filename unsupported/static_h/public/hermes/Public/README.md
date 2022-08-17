The Public directory contains type declarations made available to both the
Hermes runtime and the Hermes API. The idea is to give both projects a shared
vocabulary of types to reduce the need for bridging and glue code.

The Public directory may not use types from the Hermes, VM, or API layers, or
LLVM types. It may use std types and its own types only.

# Configuration

Most Runtime constructor parameters are passed in through configuration structs
created using the macros defined in `CtorConfig.h`.  Below is an example:

```
#define FOO_FIELDS(F)          \
  F(size_t, Bar, 0)            \
  F(std::string, Baz, "Hello") \
  /* FOO_FIELDS END */

_HERMES_CTORCONFIG_STRUCT(FooConfig, FOO_FIELDS, {
  Bar_ = std::min(Bar_, Baz_.size());
});
```

This defines a `FooConfig` struct, with two fields, with the following types,
names and default values:

```
size_t Bar_ = 0;
std::string Baz_ = "Hello";
```

`FooConfig`'s default constructor produces a config with all default values.

## Accessors

Each parameter has an associated getter, in this example `FooConfig` has:

```
size_t FooConfig::getBar() const;
std::string FooConfig::getBaz() const;
```

## Building

Each config also has a corresponding `Builder` type -- in this case
`FooConfig::Builder` -- which is the preferred way to override defaults:

```
auto fooConfig =
  FooConfig::Builder()
    .withBar(10)
    .withBaz("Hello, world!")
    .build();
```

This defines a new config, `fooConfig`, where `Bar = 10` and `Baz = "Hello,
world!"`.  Note that

 - every field is optional,
 - if omitted a field retains its default value,
 - and the underlying `FooConfig` is obtained through a call to
   `FooConfig::Builder::build()`.

It is possible to query a builder on whether a given field has been overridden:
A call to `bool FooConfig::Builder::hasBar() const` returns true on a given
builder if and only if the builder has had `withBar(...)` called on it
previously.

## Validation and Consistency

The config definition can also include arbitrary logic that is run when the
config is built (The last parameter to `_HERMES_CTORCONFIG_STRUCT`).  This is
typically used to enforce constraints between parameters.  If this is not
required, an empty block can be supplied (i.e. `{}`).  In the example it
ensures the value of `Bar` never exceeds the length of the string in `Baz`.

The body of the logic has mutable access to all the variables in the config.
However, note that **their names are suffixed with underscores.**.  It also
provides access to the `builder` (useful for querying whether the fields were
explicitly overridden or not).  The body should not return anything.

**NB** The default values of the config are assumed to be unchanged by the
supplied logic.  As a result, the logic is not run as part of the config's
default constructor.

## Rebuild

It is possible to modify the contents of an existing config, but only by
converting it back into a builder, using

```
FooConfig::Builder FooConfig::rebuild() const;
```

which returns a fresh builder with fields populated from the config it was
called on.

**NB** Fields in the fresh builder are not considered overridden, even if their
values diverge from the config's defaults.
