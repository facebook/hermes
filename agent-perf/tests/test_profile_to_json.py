#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for profile_to_json.py parsing functions."""

import sys
import unittest
from pathlib import Path

# Add tools directory to path for imports.
TOOLS_DIR = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(TOOLS_DIR))

from profile_to_json import filter_results, format_human, parse_perf_report


class TestParsePerfReport(unittest.TestCase):
    """Tests for parse_perf_report function."""

    def test_parse_with_sample_counts(self):
        """Test parsing lines with sample counts."""
        lines = [
            "# Overhead  Samples  Command  Shared Object  Symbol",
            "  12.34%     1234  hermes  libhermes.so  [.] someFunction",
            "   5.67%      567  hermes  [kernel]      [k] page_fault",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 2)
        self.assertEqual(results[0]["function"], "someFunction")
        self.assertEqual(results[0]["module"], "libhermes.so")
        self.assertEqual(results[0]["samples"], 1234)
        self.assertEqual(results[0]["percentage"], 12.34)

        self.assertEqual(results[1]["function"], "page_fault")
        self.assertEqual(results[1]["module"], "[kernel]")
        self.assertEqual(results[1]["samples"], 567)
        self.assertEqual(results[1]["percentage"], 5.67)

    def test_parse_without_sample_counts(self):
        """Test parsing lines without sample counts."""
        lines = [
            "  12.34%  hermes  libhermes.so  [.] someFunction",
            "   5.67%  hermes  [kernel]      [k] page_fault",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 2)
        self.assertEqual(results[0]["function"], "someFunction")
        self.assertEqual(results[0]["module"], "libhermes.so")
        self.assertEqual(results[0]["samples"], 0)
        self.assertEqual(results[0]["percentage"], 12.34)

        self.assertEqual(results[1]["function"], "page_fault")
        self.assertEqual(results[1]["samples"], 0)
        self.assertEqual(results[1]["percentage"], 5.67)

    def test_skip_comment_and_empty_lines(self):
        """Test that comment and empty lines are skipped."""
        lines = [
            "# This is a comment",
            "",
            "  12.34%     1234  hermes  libhermes.so  [.] someFunction",
            "# Another comment",
            "",
            "   5.67%      567  hermes  libc.so       [.] malloc",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 2)
        self.assertEqual(results[0]["function"], "someFunction")
        self.assertEqual(results[1]["function"], "malloc")

    def test_multiple_entries_parsed(self):
        """Test that multiple entries are parsed correctly."""
        lines = [
            "  30.00%     3000  hermes  libhermes.so  [.] functionA",
            "  20.00%     2000  hermes  libhermes.so  [.] functionB",
            "  15.00%     1500  hermes  libc.so       [.] memcpy",
            "  10.00%     1000  hermes  [kernel]      [k] syscall",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 4)
        self.assertEqual(results[0]["percentage"], 30.00)
        self.assertEqual(results[1]["percentage"], 20.00)
        self.assertEqual(results[2]["percentage"], 15.00)
        self.assertEqual(results[3]["percentage"], 10.00)

    def test_percentage_with_leading_spaces(self):
        """Test parsing percentages with varying leading spaces."""
        lines = [
            "12.34%     1234  hermes  libhermes.so  [.] noLeadingSpace",
            " 5.67%      567  hermes  libhermes.so  [.] oneSpace",
            "  1.23%      123  hermes  libhermes.so  [.] twoSpaces",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 3)
        self.assertEqual(results[0]["percentage"], 12.34)
        self.assertEqual(results[1]["percentage"], 5.67)
        self.assertEqual(results[2]["percentage"], 1.23)

    def test_function_names_with_special_chars(self):
        """Test parsing function names with special characters."""
        lines = [
            "  12.34%     1234  hermes  libhermes.so  [.] operator+",
            "   5.67%      567  hermes  libhermes.so  [.] std::vector<int>::push_back",
            "   3.45%      345  hermes  libhermes.so  [.] foo::bar(int, char*)",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 3)
        self.assertEqual(results[0]["function"], "operator+")
        self.assertEqual(results[1]["function"], "std::vector<int>::push_back")
        self.assertEqual(results[2]["function"], "foo::bar(int, char*)")

    def test_empty_input(self):
        """Test parsing empty input."""
        results = parse_perf_report([])
        self.assertEqual(results, [])

    def test_only_comments_and_empty_lines(self):
        """Test input with only comments and empty lines."""
        lines = [
            "# Comment 1",
            "",
            "# Comment 2",
            "",
        ]
        results = parse_perf_report(lines)
        self.assertEqual(results, [])

    def test_mixed_format_lines(self):
        """Test mix of lines with and without sample counts."""
        lines = [
            "  12.34%     1234  hermes  libhermes.so  [.] withSamples",
            "   5.67%  hermes  libc.so  [.] noSamples",
        ]
        results = parse_perf_report(lines)

        self.assertEqual(len(results), 2)
        self.assertEqual(results[0]["samples"], 1234)
        self.assertEqual(results[1]["samples"], 0)


class TestFilterResults(unittest.TestCase):
    """Tests for filter_results function."""

    def setUp(self):
        """Create sample results for filtering."""
        self.results = [
            {
                "function": "funcA",
                "module": "libhermes.so",
                "samples": 3000,
                "percentage": 30.0,
            },
            {
                "function": "funcB",
                "module": "libhermes.so",
                "samples": 2000,
                "percentage": 20.0,
            },
            {
                "function": "funcC",
                "module": "libc.so",
                "samples": 1500,
                "percentage": 15.0,
            },
            {
                "function": "funcD",
                "module": "[kernel]",
                "samples": 1000,
                "percentage": 10.0,
            },
        ]

    def test_no_filter_no_limit(self):
        """Test with no filter or limit, should sort by percentage."""
        filtered = filter_results(self.results)

        self.assertEqual(len(filtered), 4)
        self.assertEqual(filtered[0]["percentage"], 30.0)
        self.assertEqual(filtered[1]["percentage"], 20.0)
        self.assertEqual(filtered[2]["percentage"], 15.0)
        self.assertEqual(filtered[3]["percentage"], 10.0)

    def test_module_filter_case_insensitive(self):
        """Test module filter is case-insensitive substring match."""
        # Filter for "hermes" should match "libhermes.so".
        filtered = filter_results(self.results, module_filter="hermes")
        self.assertEqual(len(filtered), 2)
        self.assertEqual(filtered[0]["module"], "libhermes.so")
        self.assertEqual(filtered[1]["module"], "libhermes.so")

        # Case-insensitive test.
        filtered = filter_results(self.results, module_filter="HERMES")
        self.assertEqual(len(filtered), 2)

        # Filter for "libc".
        filtered = filter_results(self.results, module_filter="libc")
        self.assertEqual(len(filtered), 1)
        self.assertEqual(filtered[0]["module"], "libc.so")

        # Filter for "kernel".
        filtered = filter_results(self.results, module_filter="kernel")
        self.assertEqual(len(filtered), 1)
        self.assertEqual(filtered[0]["module"], "[kernel]")

    def test_top_n_limit(self):
        """Test top_n limit."""
        filtered = filter_results(self.results, top_n=2)
        self.assertEqual(len(filtered), 2)
        self.assertEqual(filtered[0]["percentage"], 30.0)
        self.assertEqual(filtered[1]["percentage"], 20.0)

        filtered = filter_results(self.results, top_n=1)
        self.assertEqual(len(filtered), 1)
        self.assertEqual(filtered[0]["percentage"], 30.0)

    def test_module_filter_and_top_n(self):
        """Test combining module filter and top_n."""
        # Filter for hermes and take top 1.
        filtered = filter_results(self.results, module_filter="hermes", top_n=1)
        self.assertEqual(len(filtered), 1)
        self.assertEqual(filtered[0]["module"], "libhermes.so")
        self.assertEqual(filtered[0]["percentage"], 30.0)

    def test_top_n_larger_than_results(self):
        """Test top_n larger than number of results."""
        filtered = filter_results(self.results, top_n=100)
        self.assertEqual(len(filtered), 4)

    def test_top_n_zero(self):
        """Test top_n=0 returns all results."""
        filtered = filter_results(self.results, top_n=0)
        self.assertEqual(len(filtered), 4)

    def test_filter_no_matches(self):
        """Test filter with no matches."""
        filtered = filter_results(self.results, module_filter="nonexistent")
        self.assertEqual(len(filtered), 0)

    def test_sorting_by_percentage(self):
        """Test that results are sorted by percentage descending."""
        unsorted = [
            {"function": "f1", "module": "m1", "samples": 1, "percentage": 5.0},
            {"function": "f2", "module": "m2", "samples": 2, "percentage": 15.0},
            {"function": "f3", "module": "m3", "samples": 3, "percentage": 10.0},
        ]
        filtered = filter_results(unsorted)

        self.assertEqual(filtered[0]["percentage"], 15.0)
        self.assertEqual(filtered[1]["percentage"], 10.0)
        self.assertEqual(filtered[2]["percentage"], 5.0)


class TestFormatHuman(unittest.TestCase):
    """Tests for format_human function."""

    def test_format_empty_results(self):
        """Test formatting empty results."""
        output = format_human([])
        self.assertEqual(output, "No matching entries found.")

    def test_format_with_results(self):
        """Test formatting with results."""
        results = [
            {
                "function": "funcA",
                "module": "libhermes.so",
                "samples": 1234,
                "percentage": 12.34,
            },
            {
                "function": "funcB",
                "module": "libc.so",
                "samples": 567,
                "percentage": 5.67,
            },
        ]
        output = format_human(results)

        # Should contain header.
        self.assertIn("%", output)
        self.assertIn("Samples", output)
        self.assertIn("Module", output)
        self.assertIn("Function", output)

        # Should contain separator line.
        self.assertIn("-" * 80, output)

        # Should contain formatted entries.
        self.assertIn("12.34%", output)
        self.assertIn("1234", output)
        self.assertIn("libhermes.so", output)
        self.assertIn("funcA", output)

        self.assertIn("5.67%", output)
        self.assertIn("567", output)
        self.assertIn("libc.so", output)
        self.assertIn("funcB", output)

    def test_format_alignment(self):
        """Test that columns are properly aligned."""
        results = [
            {
                "function": "f",
                "module": "m",
                "samples": 1,
                "percentage": 1.23,
            },
        ]
        output = format_human(results)
        lines = output.split("\n")

        # Should have header, separator, and one data line.
        self.assertEqual(len(lines), 3)

        # Data line should be formatted with proper spacing.
        data_line = lines[2]
        self.assertTrue(data_line.strip().startswith("1.23%"))


if __name__ == "__main__":
    unittest.main()
