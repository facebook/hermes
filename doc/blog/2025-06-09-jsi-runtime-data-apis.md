# Introducing JSI's New Runtime Data APIs

*June 9, 2025 · tmikov*

We noticed a common pattern emerging in React Native (RN) libraries - a map from `jsi::Runtime *` to some per-runtime data. This approach has proven clumsy and problematic, because of unclear map ownership and lifetime. To address this, we recently introduced two new methods to `jsi::Runtime`: `setRuntimeData()` and `getRuntimeData()`.

```cpp
/// Stores the pointer \p data with the \p uuid in the runtime. This can be
/// used to store some custom data within the runtime. When the runtime is
/// destroyed, or if an entry at an existing key is overwritten, the runtime
/// will release its ownership of the held object.
void setRuntimeData(const UUID& uuid, const std::shared_ptr<void>& data);

/// Returns the data associated with the \p uuid in the runtime. If there's no
/// data associated with the uuid, return a null pointer.
std::shared_ptr<void> getRuntimeData(const UUID& uuid);
```

The `setRuntimeData()` method allows storage of a `std::shared_ptr<void>` associated with a UUID, with the runtime managing ownership and releasing it upon destruction or overwrite. The `getRuntimeData()` method retrieves this data for a given UUID, returning a null pointer if none exists. This approach eliminates the need for custom maps and eliminates any uncertainty about ownership and lifetime.

An immediate practical application would be storing cached instances of `jsi::PropNameID`, which are runtime-specific uniqued strings.

**Why UUIDs?** UUIDs are 128-bit values designed to be globally unique, even without central coordination. They are necessary here to avoid conflicts between independent libraries using the same runtime. You could use one for all your per-runtime data, or multiple for different things - it is up to you.
