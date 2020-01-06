# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Stack.py contains the code used for decoding Hermes JS stack in the
lldb debugger.
Add "command script import <path_to_Stack.py>" to your ~/.lldbinit to
enable this command by default in lldb.
"""

from __future__ import absolute_import, division, print_function, unicode_literals


def __lldb_init_module(debugger, _):
    """Installs a new debugger command for dumping hermes js stack"""
    debugger.HandleCommand("command script add -f Stack.dump_js_stack jsbt")


def _evaluate_expression(frame, expression):
    """Helper function to evaluate expression in the context of input frame
    and throw error if evaluation failed. The evaluated SBValue is returned.
    """
    result_value = frame.EvaluateExpression(expression)
    if result_value is None or (
        result_value.GetError() and result_value.GetError().Fail()
    ):
        raise Exception(
            "Fail to evaluate {}: {}".format(
                expression, result_value.GetError().GetCString()
            )
        )
    return result_value


def dump_js_stack(debugger, command, result, internal_dict):
    """Dump hermes js stack in string format"""
    thread = debugger.GetSelectedTarget().GetProcess().GetSelectedThread()
    leaf_frame = thread.GetFrameAtIndex(0)

    # SamplingProfiler stores current thread's Runtime in thread local variable.
    expression = (
        "::hermes::vm::SamplingProfiler::getInstance()"
        "->threadLocalRuntime_.get()->getCallStackNoAlloc()"
    )
    result_value = _evaluate_expression(leaf_frame, expression)

    # TODO: Verify the result is string type.
    value_str = result_value.GetSummary().strip('"')
    for frame in value_str.split("\\n"):
        print(frame)
