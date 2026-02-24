/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

// RUN: %hermes %s | %FileCheck %s
// REQUIRES: intl

var props = HermesInternal.getRuntimeProperties();

// CHECK: ICU Provider: {{bundled|windows|custom}}
print("ICU Provider: " + props["ICU Provider"]);

// CHECK: ICU Version: {{[0-9]+}}
print("ICU Version: " + props["ICU Version"]);
