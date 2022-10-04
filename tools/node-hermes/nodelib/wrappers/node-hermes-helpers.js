/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

'use strict';

/// Captures the stack trace and sets the err variable's stack property
/// correctly. Excludes the top two function layers from the stack (this
/// function or the Trace function from which this is called).
function ErrorCaptureStackTrace(err, trace){
    var o = new Error();
    o.name = err.name;
    o.message = err.message;
    // Don't want to include the trace or this function as an element in the stack
    var newStack = "";
    var lines = o.stack.split("\n");
    for (var i = 0; i < lines.length; i++) {
      if (i != 1 && i != 2){
        newStack += lines[i];
        if (i != lines.length-1){
          newStack += "\n";
        }
      }
    }
    err.stack = newStack;
}

Error['CaptureStackTrace'] =  ErrorCaptureStackTrace;
