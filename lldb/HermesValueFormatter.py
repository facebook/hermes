# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""HermesValueFormatter contains the code used for decoding a HermesValue in the
lldb debugger"""

from __future__ import absolute_import, division, print_function, unicode_literals

import struct

import lldb


# Various constants and masks used to decode the HermesValue.
NUM_TAG_EXP_BITS = 17
NUM_DATA_BITS = 64 - NUM_TAG_EXP_BITS
DATA_MASK = (1 << NUM_DATA_BITS) - 1
FIRST_TAG = 0xFFF88000 >> 15
LAST_TAG = 0xFFFF8000 >> 15
EMPTY_TAG = FIRST_TAG
UNDEFINED_TAG = FIRST_TAG + 1
NULL_TAG = FIRST_TAG + 2
BOOL_TAG = FIRST_TAG + 3
NATIVE_VALUE_TAG = FIRST_TAG + 4
SYMBOL_TAG = FIRST_TAG + 5
STR_TAG = FIRST_TAG + 13
OBJECT_TAG = FIRST_TAG + 14

# Tag associations and rules for formatting.
TAG_TO_NAME = {
    EMPTY_TAG: "Empty",
    UNDEFINED_TAG: "Undefined",
    NULL_TAG: "Null",
    BOOL_TAG: "Boolean",
    NATIVE_VALUE_TAG: "NativeValue",
    SYMBOL_TAG: "Symbol",
    STR_TAG: "String",
    OBJECT_TAG: "Object",
}
NO_VALUE_TAGS = [EMPTY_TAG, UNDEFINED_TAG, NULL_TAG]
POINTER_TAGS = [NATIVE_VALUE_TAG, STR_TAG, OBJECT_TAG]


def __lldb_init_module(debugger, _):
    """Installs a new debugger handle for formatting hermes values"""
    debugger.HandleCommand(
        b'type summary add "hermes::vm::HermesValue" -w hermes -F HermesValueFormatter.hv_format'
    )


def hv_format(valobj, _):
    """This formats a HermesValue based on the same encoding API used by
    HermesValues. It should be kept up to date with any changes to that
    encoding."""
    raw_val = valobj.GetChildMemberWithName("raw_")
    raw = raw_val.GetValueAsUnsigned(0)
    tag = raw >> NUM_DATA_BITS
    is_number = raw < (FIRST_TAG << NUM_DATA_BITS)
    ret_fmt_str = "{}, {}"
    tag_fmt_str = "Tag: {}"
    value_fmt_str = "Value: {}"
    if is_number:
        formatted_tagname = tag_fmt_str.format("Number")
        # Need to reinterpret the integer bytes as double bytes
        # struct.unpack returns a tuple with one element
        formatted_value = value_fmt_str.format(
            struct.unpack("d", struct.pack("Q", raw))[0]
        )
        return ret_fmt_str.format(formatted_tagname, formatted_value)

    tag_name = TAG_TO_NAME[tag]
    formatted_tagname = tag_fmt_str.format(tag_name)
    if tag in NO_VALUE_TAGS:
        return formatted_tagname

    if tag == BOOL_TAG:
        # Get the value as a boolean and output True or False
        formatted_value = value_fmt_str.format(bool(raw & 0x1))
    elif tag == SYMBOL_TAG:
        # Get the symbol id as an 32-bit int
        low_32_bits_mask = 0x00000000FFFFFFFF
        formatted_value = value_fmt_str.format(raw & low_32_bits_mask)
    elif tag in POINTER_TAGS:
        # Just display the pointer value.
        # It would be nice to inline StringPrimitives, but that requires
        # using the C++ API to access the heap, which can't be done here.
        formatted_value = value_fmt_str.format(hex(raw & DATA_MASK))

    return "{}, {}".format(formatted_tagname, formatted_value)
