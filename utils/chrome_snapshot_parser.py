#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import json


"""
Takes in a Chrome heap snapshot and makes the output into more explorable JSON
"""


# The order of all these lists must match the order of fields that v8 outputs.
EDGE_FIELDS = ["type", "name_or_index", "to_node"]
EDGE_TYPES = [
    "context",
    "element",
    "property",
    "internal",
    "hidden",
    "shortcut",
    "weak",
]
NODE_FIELDS = ["type", "name", "id", "self_size", "edge_count", "trace_node_id"]
NODE_TYPES = [
    "hidden",
    "array",
    "string",
    "object",
    "code",
    "closure",
    "regexp",
    "number",
    "native",
    "synthetic",
    "concatenated string",
    "sliced string",
    "symbol",
    "bigint",
]
LOCATION_FIELDS = ["object_index", "script_id", "line", "column"]
SAMPLE_FIELDS = ["timestamp_us", "last_assigned_id"]


def chunk(arr, chunk_size):
    assert len(arr) % chunk_size == 0, "arr must be evenly divisible by the chunk size"
    assert chunk_size >= 1, "chunk_size must be at least 1"
    for i in range(0, len(arr), chunk_size):
        yield arr[i : i + chunk_size]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("heapsnapshot")
    parser.add_argument("out")
    args = parser.parse_args()
    with open(args.heapsnapshot, "r") as f:
        root = json.load(f)
    nodes = []
    curr_edge = iter(chunk(root["edges"], len(EDGE_FIELDS)))
    for raw_type, name, id, self_size, edge_count, trace_node_id in chunk(
        root["nodes"], len(NODE_FIELDS)
    ):
        edges = []
        for _ in range(edge_count):
            raw_edge_type, name_or_index, to_node = next(curr_edge)
            real_type = EDGE_TYPES[raw_edge_type]
            assert (
                to_node % len(NODE_FIELDS) == 0
            ), "to_node in an edge isn't divisible by {}".format(len(NODE_FIELDS))
            edges.append(
                {
                    "type": real_type,
                    "name_or_index": root["strings"][name_or_index]
                    if real_type
                    in ("context", "property", "internal", "shortcut", "weak")
                    else name_or_index,
                    # Instead of printing the index, print the ID of the node
                    # pointed to. to_node points to the start of a node chunk,
                    # and the ID is the 3rd element in that chunk (zero-based
                    # indexing so +2 to get to the third element).
                    "to_node": root["nodes"][to_node + 2],
                }
            )
        nodes.append(
            {
                "type": NODE_TYPES[raw_type],
                "name": root["strings"][name],
                "id": id,
                "self_size": self_size,
                "edges": edges,
                "trace_node_id": trace_node_id,
            }
        )
    del root["edges"]

    # Iterate through locations and add the location resolution to nodes
    for object_index, script_id, line, column in chunk(
        root.get("locations", []), len(LOCATION_FIELDS)
    ):
        nodes[object_index // len(NODE_FIELDS)]["location"] = {
            "script_id": script_id,
            # Line numbers and column numbers are 0-based internally,
            # but 1-based when viewed.
            "line": line + 1,
            "column": column + 1,
        }
    del root["locations"]

    root["nodes"] = nodes
    root["samples"] = [
        {"timestamp": timestamp_us, "last_assigned_id": last_assigned_id}
        for timestamp_us, last_assigned_id in chunk(
            root.get("samples", []), len(SAMPLE_FIELDS)
        )
    ]

    del root["strings"]
    with open(args.out, "w") as f:
        json.dump(root, f, indent=2)


if __name__ == "__main__":
    main()
