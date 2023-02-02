/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s

var a = [0,1]
a.sort(function(x,y){
  a.__defineGetter__(1, function(){
    delete a[0];
    return 1;
  });
  a.__defineGetter__(0, function(){
    return 1;
  });
  return -1;
})
