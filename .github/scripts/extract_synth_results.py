#!/usr/bin/env python3
"""Extract key metrics from synth tool output."""

import json
import sys


def extract_json_block(text):
    """Extract the first top-level JSON object from text."""
    start = text.index("{")
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return json.loads(text[start : i + 1])
    raise ValueError("No complete JSON object found in input")


def main():
    if len(sys.argv) != 4:
        print(
            f"Usage: {sys.argv[0]} <synth_output_file> <sha> <commit_timestamp>",
            file=sys.stderr,
        )
        sys.exit(1)

    input_file = sys.argv[1]
    sha = sys.argv[2]
    commit_timestamp = sys.argv[3]

    with open(input_file) as f:
        text = f.read()

    stats = extract_json_block(text)
    general = stats["general"]

    results = {
        "sha": sha,
        "commit_timestamp": commit_timestamp,
        "totalTime": general["totalTime"],
        "totalGCTime": general["totalGCTime"],
        "maxGCPause": general["maxGCPause"],
        "numCollections": general["numCollections"],
        "peakAllocatedBytes": general["peakAllocatedBytes"],
        "perfEvent_instructions": general.get("perfEvent_instructions"),
        "perfEvent_cpu-cycles": general.get("perfEvent_cpu-cycles"),
        "perfEvent_L1-icache-load-misses": general.get(
            "perfEvent_L1-icache-load-misses"
        ),
        "perfEvent_L1-dcache-load-misses": general.get(
            "perfEvent_L1-dcache-load-misses"
        ),
    }

    print(json.dumps(results, indent=2))


if __name__ == "__main__":
    main()
