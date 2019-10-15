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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("heapsnapshot")
    parser.add_argument("out")
    args = parser.parse_args()
    with open(args.heapsnapshot, "r") as f:
        root = json.load(f)
    curr_node = 0
    curr_edge = 0
    nodes = []
    while curr_node < len(root["nodes"]):
        raw_type, name, id, self_size, edge_count, trace_node_id = root["nodes"][
            curr_node : curr_node + len(NODE_FIELDS)
        ]
        edges = []
        end_edge = curr_edge + edge_count * len(EDGE_FIELDS)
        while curr_edge < end_edge:
            raw_edge_type, name_or_index, to_node = root["edges"][
                curr_edge : curr_edge + len(EDGE_FIELDS)
            ]
            real_type = EDGE_TYPES[raw_edge_type]
            edges.append(
                {
                    "type": real_type,
                    "name_or_index": root["strings"][name_or_index]
                    if real_type
                    in ("context", "property", "internal", "shortcut", "weak")
                    else name_or_index,
                    "to_node": to_node // len(NODE_FIELDS),
                }
            )
            curr_edge += len(EDGE_FIELDS)
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
        curr_node += len(NODE_FIELDS)

    with open(args.out, "w") as f:
        json.dump(nodes, f, indent=2)


if __name__ == "__main__":
    main()
