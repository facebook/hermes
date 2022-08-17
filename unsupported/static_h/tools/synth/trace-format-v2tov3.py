#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""
This script translates synth trace files in format version 2 to format version 3.
"""

import argparse
import json
import sys


def parse_value_id(value):
    colonInd = value.find(":")
    if colonInd == -1:
        return None
    tag = value[0:colonInd]
    if tag == "object":
        return int(value[colonInd + 1 :])
    else:
        return None


def parse_value_string(value):
    colonInd = value.find(":")
    if colonInd == -1:
        return None
    tag = value[0:colonInd]
    if tag == "string":
        return value[colonInd + 1 :]
    else:
        return None


def max_obj_id(trace):
    maxId = 0
    for rec in trace:
        for key in rec:
            if key in [
                "objID",
                "propNameID",
                "propID",
                "hostObjectID",
                "propNamesID",
                "functionID",
            ]:
                maxId = max(maxId, rec[key])
            elif key in ["value", "retval"]:
                valueId = parse_value_id(rec[key])
                if valueId is not None:
                    maxId = max(maxId, valueId)
            elif key in ["args", "properties"]:
                for val in rec[key]:
                    valueId = parse_value_id(val)
                    if valueId is not None:
                        maxId = max(maxId, valueId)
    return maxId


def new_create_string_or_prop_name_id_record(type, str, id):
    return {"type": type, "objID": id, "encoding": "UTF-8", "chars": str}


def new_create_string_record(str, id):
    return new_create_string_or_prop_name_id_record("CreateStringRecord", str, id)


def new_create_prop_name_id_record(str, id):
    return new_create_string_or_prop_name_id_record("CreatePropNameIDRecord", str, id)


def transform_trace_work(trace, maxObjID):
    curObjID = maxObjID + 1
    out = []
    for rec in trace:
        if rec["type"] == "CreateHostFunctionRecord":
            funcName = ""
            if "functionName" in rec:
                funcName = rec["functionName"]
            out.append(new_create_prop_name_id_record(funcName, curObjID))
            rec["propNameID"] = curObjID
            curObjID += 1
        elif rec["type"] in [
            "GetPropertyRecord",
            "SetPropertyRecord",
            "HasPropertyRecord",
        ]:
            out.append(new_create_string_record(rec["propName"], curObjID))
            rec["propID"] = curObjID
            curObjID += 1
        elif rec["type"] in ["GetPropertyNativeRecord", "SetPropertyNativeRecord"]:
            out.append(new_create_prop_name_id_record(rec["propName"], curObjID))
            rec["propNameID"] = curObjID
            curObjID += 1
        for key in rec:
            if key in ["value", "retval"]:
                valueString = parse_value_string(rec[key])
                if valueString is not None:
                    out.append(new_create_string_record(valueString, curObjID))
                    rec[key] = "string:" + str(curObjID)
                    curObjID += 1
            elif key in ["args", "properties"]:
                newArr = []
                for val in rec[key]:
                    valueString = parse_value_string(val)
                    if valueString is not None:
                        out.append(new_create_string_record(valueString, curObjID))
                        newArr.append("string:" + str(curObjID))
                        curObjID += 1
                    else:
                        newArr.append(val)
                rec[key] = newArr
        out.append(rec)
    return out


def transform_trace(trace):
    return transform_trace_work(trace, max_obj_id(trace))


def transform2to3(jsonInput):
    out = {}
    if jsonInput["version"] != 2:
        print("Expected version 2 trace; got " + str(jsonInput["version"]))
        return None
    for key in jsonInput:
        if key == "version":
            out[key] = 3
        elif key == "trace":
            out[key] = transform_trace(jsonInput[key])
        else:
            out[key] = jsonInput[key]
    return out


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "json_file", nargs="?", type=argparse.FileType("r"), default=sys.stdin
    )
    args = parser.parse_args()
    jsonInput = json.load(args.json_file)
    newJson = transform2to3(jsonInput)
    json.dump(newJson, sys.stdout, indent=4)


if __name__ == "__main__":
    main()
