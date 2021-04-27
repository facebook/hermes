Known Issues

1. "intl402/Collator/ignore-invalid-unicode-ext-values.js" is failing because Array.sort is not stable.

To reproduce, repeat this statement multiple times:

console.log(testArray.sort(new Intl.Collator().compare));

2. Spec requires the object returned by Intl.Collator.prototype.resolvedOptions() to be in specific order. Because we use JavaHashMaps in Java code and std::unordered_map in the C++ layer, we can't produce deterministic ordering of keys. This fails "intl402/Collator/prototype/order.js"
