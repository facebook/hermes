#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
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
NODE_FIELDS = [
    "type",
    "name",
    "id",
    "self_size",
    "edge_count",
    "trace_node_id",
    "detachedness",
]
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
TRACE_FUNCTION_INFO_FIELDS = [
    "function_id",
    "name",
    "script_name",
    "script_id",
    "line",
    "column",
]
TRACE_NODE_FIELDS = ["id", "function_info_index", "count", "size", "children"]


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

    strings = root["strings"]
    # First parse the trace fields, if present.
    trace_functions = [
        {
            "function_id": function_id,
            "name": strings[name],
            "script_name": strings[script_name],
            "script_id": script_id,
            "line": line,
            "column": column,
        }
        for function_id, name, script_name, script_id, line, column in chunk(
            root.get("trace_function_infos", []), len(TRACE_FUNCTION_INFO_FIELDS)
        )
    ]
    root["trace_function_infos"] = trace_functions

    # The result of this will be the node, and that node's parent, to ascend
    # the stack.
    trace_id_to_node = {}
    # The root node has trace_node_id 0, to make it easy to break out.
    trace_tree = root.get("trace_tree", [])
    if trace_tree:
        trace_node_stack = [(trace_tree, 0)]
        while trace_node_stack:
            trace_node, parent_id = trace_node_stack.pop()
            if not parent_id:
                # Root node should be the only one at the top level.
                assert len(trace_node) == len(
                    TRACE_NODE_FIELDS
                ), "More than one node at the top of the tree"
            for child in chunk(trace_node, len(TRACE_NODE_FIELDS)):
                # Convert function_info_index into the actual function_info.
                child[1] = trace_functions[child[1]]
                trace_id_to_node[child[0]] = (child, parent_id)
                # Children array is the last element.
                if child[-1]:
                    # Add to the stack, to do a depth first traversal of the tree.
                    trace_node_stack.append((child[-1], child[0]))

    nodes = []
    curr_edge = iter(chunk(root["edges"], len(EDGE_FIELDS)))
    for raw_type, name, id, self_size, edge_count, trace_node_id, detachedness in chunk(
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
                    "name_or_index": strings[name_or_index]
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
        node = {
            "type": NODE_TYPES[raw_type],
            "name": strings[name],
            "id": id,
            "self_size": self_size,
            "edges": edges,
            "detachedness": detachedness,
        }
        # Make an allocation stack for this object.
        allocation_stack = []
        # The root ID is zero, and so is the default ID if tracing isn't on.
        while trace_node_id:
            trace_node, trace_node_id = trace_id_to_node[trace_node_id]
            function_info = trace_node[1]
            allocation_stack.append(
                "{} at {}:{}:{}".format(
                    function_info["name"],
                    function_info["script_name"],
                    function_info["line"],
                    function_info["column"],
                )
            )
        if allocation_stack:
            # Reverse the stack, so that the first element is the topmost entry
            # point.
            node["alloc_stack"] = list(reversed(allocation_stack))
        nodes.append(node)
    try:
        next(curr_edge)
        raise AssertionError("Next edge should be the end of the edges")
    except StopIteration:
        pass
    del root["edges"]
    del root["trace_tree"]

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
