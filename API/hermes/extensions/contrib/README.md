# Contributing Extensions to Hermes

This directory contains community-contributed extensions. These extensions are maintained by their contributors and the community, not by the Hermes team.

## What This Means

- **Community Support**: If you encounter issues with a contrib extension, reach out to its author or the community rather than the Hermes team.
- **Opt-Out Available**: Contrib extensions can be disabled for critical deployments with `-DHERMES_ENABLE_CONTRIB_EXTENSIONS=OFF`.
- **Promotion Path**: Popular, well-maintained extensions may be promoted to core if they become widely adopted.

## Requirements for Acceptance

1. **JSI Only**: Extensions must use only JSI APIs, not internal Hermes APIs.
2. **Tests Required**: Include `.js` test files using Lit (same format as Hermes tests).
3. **Follow Patterns**: Match the existing extension structure (see `ContribDummy` example).
4. **Documentation**: Provide clear documentation of what your extension does.

## How to Add a Contrib Extension

### Step 1: Create the JavaScript File

Create `NN-YourExtension.js` where `NN` is a two-digit number controlling load order:

```javascript
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// YourExtension description
  extensions.YourExtension = function(nativeHelpers) {
    // Setup code here
    // nativeHelpers contains any native functions passed from C++
  };
```

### Step 2: Create the C++ Header

Create `YourExtension.h`:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_EXTENSIONS_CONTRIB_YOUREXTENSION_H
#define HERMES_EXTENSIONS_CONTRIB_YOUREXTENSION_H

#include <jsi/jsi.h>

namespace facebook {
namespace hermes {

void installYourExtension(jsi::Runtime &runtime, jsi::Object &extensions);

} // namespace hermes
} // namespace facebook

#endif
```

### Step 3: Create the C++ Implementation

Create `YourExtension.cpp`:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "YourExtension.h"
#include "../Intrinsics.h"  // If you need intrinsics

namespace facebook {
namespace hermes {

void installYourExtension(jsi::Runtime &rt, jsi::Object &extensions) {
  // Create native helpers object if needed
  jsi::Object nativeHelpers = jsi::Object(rt);

  // Add native functions (optional)
  nativeHelpers.setProperty(rt, "myNativeFunc",
    jsi::Function::createFromHostFunction(rt,
      jsi::PropNameID::forAscii(rt, "myNativeFunc"),
      1,  // argument count
      [](jsi::Runtime &rt, const jsi::Value &,
         const jsi::Value *args, size_t count) -> jsi::Value {
        // Implementation
        return jsi::Value::undefined();
      }));

  // Get and call the setup function
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "YourExtension");
  setup.call(rt, nativeHelpers);
}

} // namespace hermes
} // namespace facebook
```

### Step 4: Update Build and Registration

All updates are within the `contrib/` directory:

**a) Add to `CMakeLists.txt`:**

```cmake
set(CONTRIB_EXTENSIONS_CPP_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/ContribExtensions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ContribDummy.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/YourExtension.cpp  # Add this
  PARENT_SCOPE
)
```

**b) Add to `ContribExtensions.cpp`:**

```cpp
#include "ContribExtensions.h"

#include "ContribDummy.h"
#include "YourExtension.h"  // Add this

void installContribExtensions(jsi::Runtime &rt, jsi::Object &extensions) {
  installContribDummy(rt, extensions);
  installYourExtension(rt, extensions);  // Add this
}
```

### Step 5: Add Tests

Create test files in `test/hermes/contrib/` following the Lit test format:

```javascript
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

// Test your extension here
print("Expected output");
// CHECK: Expected output
```

Tests in `test/hermes/contrib/` are automatically skipped when contrib extensions are disabled.

## Disabling Contrib Extensions

To build Hermes without contrib extensions:

```bash
cmake -B build -DHERMES_ENABLE_CONTRIB_EXTENSIONS=OFF
```

This is useful for:
- Critical production deployments requiring minimal attack surface
- Debugging issues to isolate core vs contrib behavior
- Builds with strict binary size requirements
