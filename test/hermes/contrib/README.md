# Contrib Extension Tests

This directory contains tests for community-contributed extensions.

## Automatic Exclusion

Tests in this directory are automatically skipped when building with:

```bash
cmake -B build -DHERMES_ENABLE_CONTRIB_EXTENSIONS=OFF
```

This is handled by `lit.local.cfg` which checks for the `contrib_extensions` feature.

## Adding Tests

When adding a new contrib extension, place its tests in this directory following the standard Hermes test format:

```javascript
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

print("Hello from my extension");
// CHECK: Hello from my extension
```

Tests here use the same Lit infrastructure as other Hermes tests. See `test/hermes/` for more examples.
