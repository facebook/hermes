/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

try{
    var fs = require('./internalBinding-global-scope3.js');
}catch(e){
    print('caught:', e.name, e.message);
}
//CHECK: caught: ReferenceError Property 'internalBinding' doesn't exist
