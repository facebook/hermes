#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""trace_normalize takes a trace file and removes sources of non-determinism.

Some examples of non-determinism are times and object IDs.
The script can also apply transforms to make it easier to read by a human, such
as translating doubles from the encoded format to a human-readable number.

The purpose of this is to be able to compare two traces for similarity, and
focus on the different events that occur rather than trivial differences like
times. You can use diff freely on a trace after it has been normalized.

You can also re-run a trace after it has been normalized through the
TraceInterpreter; you could submit this if you like as a benchmark.
However, it is typically better to submit the original, to keep the times in
case they're ever needed.
"""
import argparse
import json
import struct
import sys
from collections import defaultdict
from typing import Union


def isValueObject(v: str) -> bool:
    return v.startswith("object:")


def isValuePropNameID(v: str) -> bool:
    return v.startswith("propNameID:")


def isValueString(v: str) -> bool:
    return v.startswith("string:")


def isValueNumber(v: str) -> bool:
    return v.startswith("number:")


def parseObjectFromValue(v: str) -> int:
    assert isValueObject(v) or isValuePropNameID(v) or isValueString(v)
    parts = v.split(":")
    assert len(parts) == 2
    return int(parts[1])


def parseNumberFromValue(v: str) -> Union[int, float]:
    assert isValueNumber(v)
    parts = v.split(":")
    assert len(parts) == 2
    num = int(parts[1], base=16)
    flt = struct.unpack("d", struct.pack("Q", num))[0]
    return int(flt) if flt.is_integer() else flt


def createObject(objID: int) -> str:
    return "object:" + str(objID)


def createPropNameID(propID: int) -> str:
    return "propNameID:" + str(propID)


def createString(stringID: int) -> str:
    return "string:" + str(stringID)


def createNumber(num: float) -> str:
    # This might be an imprecise notation, but it is more human-readable than
    # the precise version.
    return "number:" + str(num)


class Normalizer:
    def __init__(self, globalObjID: int, convert_number: bool = False):
        def get_normalize_map(globalObjID: int):
            id = 0

            def id_factory():
                nonlocal id
                id += 1
                return id

            return defaultdict(id_factory, {globalObjID: 0})

        self.normal = get_normalize_map(globalObjID)
        self.string_normal = {}
        self.convert_number = convert_number

    def normalize_value(self, v: str) -> str:
        if isValueObject(v):
            return createObject(self.normal[parseObjectFromValue(v)])
        elif isValuePropNameID(v):
            return createPropNameID(self.normal[parseObjectFromValue(v)])
        elif isValueString(v):
            return createString(self.normal[parseObjectFromValue(v)])
        elif self.convert_number and isValueNumber(v):
            return createNumber(parseNumberFromValue(v))
        else:
            return v

    def normalize_rec(self, rec):
        # These should be kept in sync with changes to SynthTrace.h.
        # Hopefully there is never a key that is used for both objects and
        # non-objects (in different records).
        # If there is a conflict between two keys with the same name and
        # different value types:
        # * if it is another number it will cause a failure at runtime
        # * if it is a string it will cause a failure at parse time
        OBJECT_HOLDING_KEYS = [
            "objID",
            "functionID",
            "hostObjectID",
            "propNamesID",
            "propID",
            "propNameID",
        ]
        VALUE_HOLDING_KEYS = ["value", "retval"]
        if rec.get("type", "") == "CreatePropNameIDRecord":
            # PropNameIDs might get different IDs for the same string depending
            # on the GC schedule, which makes it impossible to compare two
            # traces. Normalize them more aggressively by using the string
            # contents to map to an ID.
            objID = rec["objID"]
            chars = rec["chars"]
            if objID not in self.normal:
                if chars in self.string_normal:
                    self.normal[objID] = self.string_normal[chars]
                else:
                    self.string_normal[chars] = self.normal[objID]
            rec["objID"] = self.normal[objID]
        else:
            for objkey in OBJECT_HOLDING_KEYS:
                if objkey in rec:
                    rec[objkey] = self.normal[rec[objkey]]

        for valuekey in VALUE_HOLDING_KEYS:
            if valuekey in rec:
                rec[valuekey] = self.normalize_value(rec[valuekey])

        if "args" in rec:
            # Args is an array of values, normalize each one if it's an object
            rec["args"] = [self.normalize_value(v) for v in rec["args"]]
        return rec


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "infile", nargs="?", type=argparse.FileType("r"), default=sys.stdin
    )
    parser.add_argument(
        "outfile", nargs="?", type=argparse.FileType("w"), default=sys.stdout
    )
    parser.add_argument(
        "--add-record-num", dest="add_record_num", action="store_true", default=False
    )
    parser.add_argument(
        "--convert-number", dest="convert_number", action="store_true", default=False
    )
    args = parser.parse_args()
    trace_contents = json.load(args.infile)
    normal = Normalizer(trace_contents["globalObjID"], args.convert_number)

    def stripTime(rec):
        del rec["time"]
        return rec

    trace_contents["trace"] = [
        normal.normalize_rec(stripTime(rec)) for rec in trace_contents["trace"]
    ]

    if args.add_record_num:
        # Add a globalRecordNum key for easier searching for a particular record.
        for globalRecordNum, rec in enumerate(trace_contents["trace"]):
            rec["globalRecordNum"] = globalRecordNum

    trace_contents["globalObjID"] = 0
    json.dump(trace_contents, args.outfile, indent=4)
    return 0


if __name__ == "__main__":
    sys.exit(main())
