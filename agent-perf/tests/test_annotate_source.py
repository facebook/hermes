#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for annotate_source.py parsing functions."""

import sys
import unittest
from pathlib import Path

# Add tools directory to path for imports.
TOOLS_DIR = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(TOOLS_DIR))

from annotate_source import format_human, parse_perf_annotate


class TestParsePerfAnnotate(unittest.TestCase):
    """Tests for parse_perf_annotate function."""

    def test_parse_annotated_lines_with_percentage_and_line_number(self):
        """Test parsing annotated lines with percentage and line number."""
        lines = [
            "someFunction():",
            "       :    10: int foo(int x) {",
            "  0.50 :    11:     return x + 1;",
            "       :    12: }",
        ]
        results = parse_perf_annotate(lines)

        self.assertEqual(len(results), 3)

        self.assertEqual(results[0]["line_number"], 10)
        self.assertEqual(results[0]["source"], " int foo(int x) {")
        self.assertEqual(results[0]["percentage"], 0.0)
        self.assertEqual(results[0]["hot"], False)

        self.assertEqual(results[1]["line_number"], 11)
        self.assertEqual(results[1]["source"], "     return x + 1;")
        self.assertEqual(results[1]["percentage"], 0.50)
        self.assertEqual(results[1]["hot"], False)

        self.assertEqual(results[2]["line_number"], 12)
        self.assertEqual(results[2]["source"], " }")
        self.assertEqual(results[2]["percentage"], 0.0)
        self.assertEqual(results[2]["hot"], False)

    def test_parse_lines_without_percentage_cold_lines(self):
        """Test that lines without percentage get 0.0%."""
        lines = [
            "myFunction():",
            "       :    1: void bar() {",
            "       :    2:     // comment",
            "  1.50 :    3:     doWork();",
            "       :    4: }",
        ]
        results = parse_perf_annotate(lines)

        self.assertEqual(len(results), 4)
        self.assertEqual(results[0]["percentage"], 0.0)
        self.assertEqual(results[1]["percentage"], 0.0)
        self.assertEqual(results[2]["percentage"], 1.50)
        self.assertEqual(results[3]["percentage"], 0.0)

    def test_function_filter_case_insensitive(self):
        """Test function filter is case-insensitive substring match."""
        lines = [
            "someFunction():",
            "  0.50 :    1: code1",
            "anotherFunction():",
            "  1.00 :    1: code2",
        ]

        # Filter for "some" (case variations).
        results = parse_perf_annotate(lines, function_filter="some")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["source"], " code1")

        results = parse_perf_annotate(lines, function_filter="SOME")
        self.assertEqual(len(results), 1)

        # Filter for "another".
        results = parse_perf_annotate(lines, function_filter="another")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["source"], " code2")

        # Filter for "Function" should match both.
        results = parse_perf_annotate(lines, function_filter="Function")
        self.assertEqual(len(results), 2)

    def test_file_marker_detection(self):
        """Test that file markers are detected and set."""
        lines = [
            "myFunc():",
            "       : /path/to/file.cpp:",
            "  0.50 :    1: code",
            "       : /another/file.cpp:",
            "  1.00 :    1: more code",
        ]
        results = parse_perf_annotate(lines)

        self.assertEqual(len(results), 2)
        self.assertEqual(results[0]["file"], "/path/to/file.cpp")
        self.assertEqual(results[1]["file"], "/another/file.cpp")

    def test_hot_detection(self):
        """Test that hot threshold (>= 1.0%) is correctly applied."""
        lines = [
            "func():",
            "  0.99 :    1: not hot",
            "  1.00 :    2: hot threshold",
            "  1.50 :    3: hot above",
            "  5.00 :    4: very hot",
        ]
        results = parse_perf_annotate(lines)

        self.assertEqual(len(results), 4)
        self.assertEqual(results[0]["hot"], False)
        self.assertEqual(results[1]["hot"], True)
        self.assertEqual(results[2]["hot"], True)
        self.assertEqual(results[3]["hot"], True)

    def test_format_human_with_hot_markers(self):
        """Test format_human marks hot lines with [HOT]."""
        results = [
            {
                "file": "test.cpp",
                "line_number": 1,
                "source": "cold line",
                "percentage": 0.50,
                "hot": False,
            },
            {
                "file": "test.cpp",
                "line_number": 2,
                "source": "hot line",
                "percentage": 1.50,
                "hot": True,
            },
        ]
        output = format_human(results)

        self.assertIn("[HOT   1.5%]", output)
        self.assertIn("[     0.50%]", output)
        self.assertIn("cold line", output)
        self.assertIn("hot line", output)

    def test_format_human_with_file_separators(self):
        """Test format_human adds file separators."""
        results = [
            {
                "file": "file1.cpp",
                "line_number": 1,
                "source": "code1",
                "percentage": 0.0,
                "hot": False,
            },
            {
                "file": "file2.cpp",
                "line_number": 1,
                "source": "code2",
                "percentage": 0.0,
                "hot": False,
            },
        ]
        output = format_human(results)

        self.assertIn("--- file1.cpp ---", output)
        self.assertIn("--- file2.cpp ---", output)

    def test_format_human_empty_results(self):
        """Test format_human with empty results."""
        output = format_human([])
        self.assertEqual(output, "No annotated lines found.")

    def test_empty_input(self):
        """Test parsing empty input."""
        results = parse_perf_annotate([])
        self.assertEqual(results, [])

    def test_multiple_functions_filter_selects_one(self):
        """Test filtering selects only matching function."""
        lines = [
            "firstFunction():",
            "  0.50 :    1: first code",
            "secondFunction():",
            "  1.00 :    1: second code",
            "thirdFunction():",
            "  1.50 :    1: third code",
        ]

        results = parse_perf_annotate(lines, function_filter="second")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["source"], " second code")

    def test_no_function_filter_includes_all(self):
        """Test that without filter, all functions are included."""
        lines = [
            "func1():",
            "  0.50 :    1: code1",
            "func2():",
            "  1.00 :    1: code2",
        ]

        results = parse_perf_annotate(lines)
        self.assertEqual(len(results), 2)

    def test_simple_annotated_format_without_line_numbers(self):
        """Test parsing simple annotated format without explicit line numbers."""
        lines = [
            "func():",
            "  0.50 : some code",
            "       : more code",
            "  1.00 : hot code",
        ]
        results = parse_perf_annotate(lines)

        self.assertEqual(len(results), 3)
        # Line counter should increment.
        self.assertEqual(results[0]["line_number"], 1)
        self.assertEqual(results[1]["line_number"], 2)
        self.assertEqual(results[2]["line_number"], 3)

        self.assertEqual(results[0]["percentage"], 0.50)
        self.assertEqual(results[1]["percentage"], 0.0)
        self.assertEqual(results[2]["percentage"], 1.00)

    def test_unknown_file_when_no_marker(self):
        """Test that file is <unknown> when no file marker present."""
        lines = [
            "func():",
            "  0.50 :    1: code",
        ]
        results = parse_perf_annotate(lines)

        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["file"], "<unknown>")

    def test_format_human_zero_percentage_no_prefix(self):
        """Test format_human uses spaces for zero percentage."""
        results = [
            {
                "file": "test.cpp",
                "line_number": 1,
                "source": "code",
                "percentage": 0.0,
                "hot": False,
            },
        ]
        output = format_human(results)

        # Should have spaces instead of percentage for zero.
        lines = output.split("\n")
        data_line = [line for line in lines if "code" in line][0]
        # Should start with spaces (no percentage shown for 0.0).
        self.assertTrue(data_line.strip().endswith("1: code"))


class TestFormatHumanEdgeCases(unittest.TestCase):
    """Additional edge case tests for format_human."""

    def test_format_multiple_files_with_blank_separator(self):
        """Test that blank line separates different files."""
        results = [
            {
                "file": "a.cpp",
                "line_number": 1,
                "source": "x",
                "percentage": 0.0,
                "hot": False,
            },
            {
                "file": "b.cpp",
                "line_number": 1,
                "source": "y",
                "percentage": 0.0,
                "hot": False,
            },
        ]
        output = format_human(results)
        lines = output.split("\n")

        # Find the blank line between files.
        blank_indices = [i for i, line in enumerate(lines) if line == ""]
        self.assertGreater(len(blank_indices), 0)


if __name__ == "__main__":
    unittest.main()
