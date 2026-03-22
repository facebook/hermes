#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for sh_source_mapper.py"""

import sys
import unittest
from pathlib import Path

# Add tools directory to path to import the module
tools_dir = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(tools_dir))

from sh_source_mapper import (
    build_c_to_js_map,
    build_js_to_c_map,
    parse_line_directives,
    query_c_line,
    query_js_line,
)


class TestParseLineDirectives(unittest.TestCase):
    """Test parsing of #line directives."""

    def test_standard_line_directive(self):
        source = """
#line 10 "input.js"
var x = 42;
"""
        mappings = parse_line_directives(source)
        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["c_line"], 3)
        self.assertEqual(mappings[0]["js_line"], 10)
        self.assertEqual(mappings[0]["js_file"], "input.js")
        self.assertEqual(mappings[0]["c_source"], "var x = 42;")

    def test_alternate_line_directive_format(self):
        source = """
# 10 "input.js"
var x = 42;
"""
        mappings = parse_line_directives(source)
        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["c_line"], 3)
        self.assertEqual(mappings[0]["js_line"], 10)
        self.assertEqual(mappings[0]["js_file"], "input.js")

    def test_consecutive_code_lines_increment_js_line(self):
        source = """
#line 10 "input.js"
var x = 42;
var y = 43;
var z = 44;
"""
        mappings = parse_line_directives(source)
        self.assertEqual(len(mappings), 3)
        self.assertEqual(mappings[0]["js_line"], 10)
        self.assertEqual(mappings[1]["js_line"], 11)
        self.assertEqual(mappings[2]["js_line"], 12)
        self.assertEqual(mappings[0]["c_line"], 3)
        self.assertEqual(mappings[1]["c_line"], 4)
        self.assertEqual(mappings[2]["c_line"], 5)

    def test_skips_empty_lines(self):
        source = """
#line 10 "input.js"
var x = 42;

var y = 43;
"""
        mappings = parse_line_directives(source)
        # Empty line should not create a mapping
        self.assertEqual(len(mappings), 2)
        self.assertEqual(mappings[0]["c_line"], 3)
        self.assertEqual(mappings[1]["c_line"], 5)

    def test_skips_bare_braces(self):
        source = """
#line 10 "input.js"
{
var x = 42;
}
"""
        mappings = parse_line_directives(source)
        # Bare { and } should not create mappings
        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["c_source"], "var x = 42;")

    def test_multiple_files_in_same_c_output(self):
        source = """
#line 10 "file1.js"
var a = 1;
#line 20 "file2.js"
var b = 2;
#line 30 "file1.js"
var c = 3;
"""
        mappings = parse_line_directives(source)
        self.assertEqual(len(mappings), 3)
        self.assertEqual(mappings[0]["js_file"], "file1.js")
        self.assertEqual(mappings[0]["js_line"], 10)
        self.assertEqual(mappings[1]["js_file"], "file2.js")
        self.assertEqual(mappings[1]["js_line"], 20)
        self.assertEqual(mappings[2]["js_file"], "file1.js")
        self.assertEqual(mappings[2]["js_line"], 30)

    def test_no_line_directives(self):
        source = """
var x = 42;
var y = 43;
"""
        mappings = parse_line_directives(source)
        self.assertEqual(len(mappings), 0)

    def test_line_directive_with_whitespace(self):
        source = """
  #line 10 "input.js"
var x = 42;
"""
        mappings = parse_line_directives(source)
        self.assertEqual(len(mappings), 1)
        self.assertEqual(mappings[0]["js_line"], 10)


class TestBuildMaps(unittest.TestCase):
    """Test building lookup maps."""

    def test_build_c_to_js_map(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 11, "js_line": 6, "js_file": "input.js", "c_source": "line2"},
        ]
        c_to_js = build_c_to_js_map(mappings)
        self.assertEqual(len(c_to_js), 2)
        self.assertEqual(c_to_js[10]["js_line"], 5)
        self.assertEqual(c_to_js[11]["js_line"], 6)

    def test_build_js_to_c_map(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 11, "js_line": 6, "js_file": "input.js", "c_source": "line2"},
        ]
        js_to_c = build_js_to_c_map(mappings)
        self.assertIn("input.js", js_to_c)
        self.assertIn(5, js_to_c["input.js"])
        self.assertIn(6, js_to_c["input.js"])
        self.assertEqual(js_to_c["input.js"][5], [10])
        self.assertEqual(js_to_c["input.js"][6], [11])

    def test_js_to_c_map_multiple_c_lines_per_js_line(self):
        # Same JS line can map to multiple C lines (e.g., inlining)
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 20, "js_line": 5, "js_file": "input.js", "c_source": "line2"},
            {"c_line": 30, "js_line": 5, "js_file": "input.js", "c_source": "line3"},
        ]
        js_to_c = build_js_to_c_map(mappings)
        self.assertEqual(len(js_to_c["input.js"][5]), 3)
        self.assertIn(10, js_to_c["input.js"][5])
        self.assertIn(20, js_to_c["input.js"][5])
        self.assertIn(30, js_to_c["input.js"][5])


class TestQueryCLine(unittest.TestCase):
    """Test querying C line to JS source."""

    def test_exact_match(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 20, "js_line": 15, "js_file": "input.js", "c_source": "line2"},
        ]
        result = query_c_line(mappings, 10)
        self.assertIsNotNone(result)
        self.assertEqual(result["c_line"], 10)
        self.assertEqual(result["js_line"], 5)
        self.assertEqual(result["js_file"], "input.js")
        self.assertNotIn("approximate", result)

    def test_nearest_preceding_mapping_with_offset(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 20, "js_line": 15, "js_file": "input.js", "c_source": "line2"},
        ]
        # Query line 15, should use mapping at line 10 with offset +5
        result = query_c_line(mappings, 15)
        self.assertIsNotNone(result)
        self.assertEqual(result["c_line"], 15)
        self.assertEqual(result["js_line"], 10)  # 5 + (15 - 10)
        self.assertEqual(result["js_file"], "input.js")
        self.assertTrue(result.get("approximate"))

    def test_no_mapping_found_returns_none(self):
        mappings = [
            {"c_line": 20, "js_line": 10, "js_file": "input.js", "c_source": "line1"},
        ]
        # Query line before first mapping
        result = query_c_line(mappings, 5)
        self.assertIsNone(result)

    def test_uses_closest_preceding_mapping(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 15, "js_line": 8, "js_file": "input.js", "c_source": "line2"},
            {"c_line": 20, "js_line": 12, "js_file": "input.js", "c_source": "line3"},
        ]
        # Query line 18, should use mapping at line 15
        result = query_c_line(mappings, 18)
        self.assertEqual(result["js_line"], 11)  # 8 + (18 - 15)


class TestQueryJsLine(unittest.TestCase):
    """Test querying JS line to C source."""

    def test_finds_all_c_lines_for_js_line(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
            {"c_line": 20, "js_line": 5, "js_file": "input.js", "c_source": "line2"},
            {"c_line": 30, "js_line": 6, "js_file": "input.js", "c_source": "line3"},
        ]
        results = query_js_line(mappings, 5)
        self.assertEqual(len(results), 2)
        c_lines = [r["c_line"] for r in results]
        self.assertIn(10, c_lines)
        self.assertIn(20, c_lines)

    def test_js_file_filter(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "file1.js", "c_source": "line1"},
            {"c_line": 20, "js_line": 5, "js_file": "file2.js", "c_source": "line2"},
        ]
        # Query with file filter
        results = query_js_line(mappings, 5, "file1.js")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["js_file"], "file1.js")

    def test_js_file_substring_match(self):
        mappings = [
            {
                "c_line": 10,
                "js_line": 5,
                "js_file": "src/input.js",
                "c_source": "line1",
            },
            {
                "c_line": 20,
                "js_line": 5,
                "js_file": "test/other.js",
                "c_source": "line2",
            },
        ]
        # Filter by substring
        results = query_js_line(mappings, 5, "input")
        self.assertEqual(len(results), 1)
        self.assertIn("input", results[0]["js_file"])

    def test_no_results_returns_empty_list(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "input.js", "c_source": "line1"},
        ]
        results = query_js_line(mappings, 99)
        self.assertEqual(len(results), 0)

    def test_no_file_filter_returns_all_matches(self):
        mappings = [
            {"c_line": 10, "js_line": 5, "js_file": "file1.js", "c_source": "line1"},
            {"c_line": 20, "js_line": 5, "js_file": "file2.js", "c_source": "line2"},
        ]
        results = query_js_line(mappings, 5, None)
        self.assertEqual(len(results), 2)


class TestComplexScenarios(unittest.TestCase):
    """Test complex real-world scenarios."""

    def test_realistic_generated_c_with_line_directives(self):
        source = """
/* Generated by shermes */
#line 1 "input.js"
function add(a, b) {
#line 2 "input.js"
  return a + b;
#line 3 "input.js"
}
#line 5 "input.js"
var result = add(1, 2);
"""
        mappings = parse_line_directives(source)
        # Should have mappings for actual code lines
        self.assertGreater(len(mappings), 0)

        # Each JS line should be represented
        js_lines = [m["js_line"] for m in mappings]
        self.assertIn(1, js_lines)
        self.assertIn(2, js_lines)
        self.assertIn(5, js_lines)

    def test_interleaved_directives_and_code(self):
        source = """
#line 10 "input.js"
var x = 1;
var y = 2;
#line 50 "other.js"
var z = 3;
#line 11 "input.js"
var w = 4;
"""
        mappings = parse_line_directives(source)

        # Should track file switches correctly
        file1_mappings = [m for m in mappings if "input.js" in m["js_file"]]
        file2_mappings = [m for m in mappings if "other.js" in m["js_file"]]

        self.assertGreater(len(file1_mappings), 0)
        self.assertGreater(len(file2_mappings), 0)

        # Check js_line increments within same file
        self.assertEqual(file1_mappings[0]["js_line"], 10)
        self.assertEqual(file1_mappings[1]["js_line"], 11)
        # After directive switch, should not increment from previous
        self.assertEqual(file2_mappings[0]["js_line"], 50)

    def test_empty_mappings_queries(self):
        mappings = []
        result = query_c_line(mappings, 10)
        self.assertIsNone(result)

        results = query_js_line(mappings, 5)
        self.assertEqual(len(results), 0)


if __name__ == "__main__":
    unittest.main()
