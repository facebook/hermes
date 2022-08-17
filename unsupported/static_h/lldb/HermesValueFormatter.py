# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""HermesValueFormatter contains the code used for decoding a HermesValue in the
lldb debugger"""

from __future__ import absolute_import, division, print_function, unicode_literals

import shlex
import struct

import lldb


def __lldb_init_module(debugger, _):
    """Installs a new debugger handle for formatting hermes values"""
    debugger.HandleCommand(
        "type summary add "
        '"hermes::vm::HermesValue" '
        '"hermes::vm::PinnedHermesValue" '
        '"hermes::vm::GCHermesValue" '
        "-w hermes -F HermesValueFormatter.hv_type_format"
    )
    debugger.HandleCommand(
        "command script add -f HermesValueFormatter.hv_format hv_format"
    )


class Tag:
    FIRST_TAG = 0xFFF9
    LAST_TAG = 0xFFFF
    EMPTY_INVALID_TAG = FIRST_TAG
    UNDEFINED_NULL_TAG = FIRST_TAG + 1
    BOOL_TAG = FIRST_TAG + 2
    SYMBOL_TAG = FIRST_TAG + 3
    NATIVE_VALUE_TAG = FIRST_TAG + 4
    STR_TAG = FIRST_TAG + 5
    OBJECT_TAG = FIRST_TAG + 6


class ExtendedTag:
    EMPTY = Tag.EMPTY_INVALID_TAG * 2
    INVALID = Tag.EMPTY_INVALID_TAG * 2 + 1
    UNDEFINED = Tag.UNDEFINED_NULL_TAG * 2
    NULL = Tag.UNDEFINED_NULL_TAG * 2 + 1
    BOOL = Tag.BOOL_TAG * 2
    SYMBOL = Tag.SYMBOL_TAG * 2
    NATIVE1 = Tag.NATIVE_VALUE_TAG * 2
    NATIVE2 = Tag.NATIVE_VALUE_TAG * 2 + 1
    STR1 = Tag.STR_TAG * 2
    STR2 = Tag.STR_TAG * 2 + 1
    OBJECT1 = Tag.OBJECT_TAG * 2
    OBJECT2 = Tag.OBJECT_TAG * 2 + 1

    # Tag associations and rules for formatting.
    EXTENDED_TAG_TO_NAME = {
        EMPTY: "Empty",
        INVALID: "Invalid",
        UNDEFINED: "Undefined",
        NULL: "Null",
        BOOL: "Boolean",
        SYMBOL: "Symbol",
        NATIVE1: "NativeValue",
        NATIVE2: "NativeValue",
        STR1: "String",
        STR2: "String",
        OBJECT1: "Object",
        OBJECT2: "Object",
    }
    NO_VALUE_TAGS = [EMPTY, INVALID, UNDEFINED, NULL]
    POINTER_TAGS = [STR1, STR2, OBJECT1, OBJECT2]

    def __init__(self, raw):
        self.raw = raw

    def __str__(self):
        return self.EXTENDED_TAG_TO_NAME[self.raw]


class HermesValue:
    # Various constants and masks used to decode the HermesValue.
    NUM_TAG_EXP_BITS = 16
    NUM_DATA_BITS = 64 - NUM_TAG_EXP_BITS
    DATA_MASK = (1 << NUM_DATA_BITS) - 1

    def __init__(self, raw):
        self.raw = raw

    def get_tag(self):
        return self.raw >> self.NUM_DATA_BITS

    def get_extended_tag(self):
        return ExtendedTag(self.raw >> (self.NUM_DATA_BITS - 1))

    def is_number(self):
        return self.raw < (Tag.FIRST_TAG << self.NUM_DATA_BITS)

    def get_number(self):
        # Need to reinterpret the integer bytes as double bytes
        # struct.unpack returns a tuple with one element.
        (f,) = struct.unpack("d", struct.pack("Q", self.raw))
        return int(f) if f.is_integer() else f

    def __str__(self):
        ret_fmt_str = "{}, {}"
        tag_fmt_str = "Tag: {}"
        value_fmt_str = "Value: {}"
        if self.is_number():
            formatted_tagname = tag_fmt_str.format("Number")
            formatted_value = value_fmt_str.format(self.get_number())
            return ret_fmt_str.format(formatted_tagname, formatted_value)

        tag = self.get_extended_tag()
        tag_name = str(tag)
        formatted_tagname = tag_fmt_str.format(tag_name)
        if tag.raw in ExtendedTag.NO_VALUE_TAGS:
            return formatted_tagname

        if tag.raw == ExtendedTag.BOOL:
            # Get the value as a boolean and output "true" or "false".
            formatted_value = value_fmt_str.format(str(bool(self.raw & 0x1)).lower())
        elif tag.raw == ExtendedTag.SYMBOL:
            # Get the symbol id as an 32-bit int.
            low_32_bits_mask = 0x00000000FFFFFFFF
            formatted_value = value_fmt_str.format(self.raw & low_32_bits_mask)
        elif tag.raw in (ExtendedTag.NATIVE1, ExtendedTag.NATIVE2):
            # Display both as hex and as decimal, since native values could be
            # either a pointer or an int.
            formatted_value = value_fmt_str.format(
                "{} ({})".format(
                    hex(self.raw & self.DATA_MASK), self.raw & self.DATA_MASK
                )
            )
        elif tag.raw in ExtendedTag.POINTER_TAGS:
            # Just display the pointer value.
            # It would be nice to inline StringPrimitives, but that requires
            # using the C++ API to access the heap, which can't be done here.
            formatted_value = value_fmt_str.format(hex(self.raw & self.DATA_MASK))

        return "{}, {}".format(formatted_tagname, formatted_value)


def hv_type_format(valobj, _):
    """This formats a HermesValue based on the same encoding API used by
    HermesValues. It should be kept up to date with any changes to that
    encoding."""
    raw_val = valobj.GetChildMemberWithName("raw_")
    return str(HermesValue(raw_val.GetValueAsUnsigned()))


def _raise_eval_failure(expression, error):
    raise Exception(
        "Fail to evaluate {}: \n{}".format(
            expression, "<N/A>" if error is None else error.GetCString()
        )
    )


def _is_eval_failed(evaluation_result):
    return evaluation_result is None or (
        evaluation_result.GetError() and evaluation_result.GetError().Fail()
    )


def _evaluate_expression(frame, expression):
    """Helper function to evaluate expression in the context of input frame
    and throw error if evaluation failed. The evaluated SBValue is returned.
    """
    result_value = frame.EvaluateExpression(expression)
    if _is_eval_failed(result_value):
        _raise_eval_failure(expression, result_value.GetError())

    return result_value


def hv_format(debugger, command, result, internal_dict):
    args = shlex.split(command)
    thread = debugger.GetSelectedTarget().GetProcess().GetSelectedThread()
    frame = thread.GetSelectedFrame()
    try:
        args = [_evaluate_expression(frame, expr) for expr in args]
    except Exception as e:
        print(str(e), file=result)
        return

    for arg in args:
        print(str(HermesValue(arg.GetValueAsUnsigned())), file=result)
