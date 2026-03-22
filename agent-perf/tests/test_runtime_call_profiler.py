#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for runtime_call_profiler.py."""

import os
import sys
import unittest

# Add tools directory to path for imports
TOOLS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tools")
sys.path.insert(0, TOOLS_DIR)

from runtime_call_profiler import (
    build_report,
    categorize,
    format_human,
    parse_perf_report_sh,
)


class TestCategorize(unittest.TestCase):
    """Tests for categorize function."""

    def test_property_access_get_by_id(self):
        """Test property access categorization for get_by_id."""
        self.assertEqual(categorize("_sh_ljs_get_by_id_rjs"), "property_access")

    def test_property_access_put_by_val(self):
        """Test property access categorization for put_by_val."""
        self.assertEqual(categorize("_sh_ljs_put_by_val_rjs"), "property_access")

    def test_property_access_del_by_index(self):
        """Test property access categorization for del_by_index."""
        self.assertEqual(categorize("_sh_ljs_del_by_index"), "property_access")

    def test_property_access_has_by_id(self):
        """Test property access categorization for has_by_id."""
        self.assertEqual(categorize("_sh_ljs_has_by_id"), "property_access")

    def test_property_access_get_computed(self):
        """Test property access categorization for get_computed."""
        self.assertEqual(categorize("_sh_ljs_get_computed"), "property_access")

    def test_property_access_prload(self):
        """Test property access categorization for prload."""
        self.assertEqual(categorize("_sh_prload"), "property_access")

    def test_property_access_prstore(self):
        """Test property access categorization for prstore."""
        self.assertEqual(categorize("_sh_prstore"), "property_access")

    def test_type_check_typeof(self):
        """Test type check categorization for typeof."""
        self.assertEqual(categorize("_sh_ljs_typeof"), "type_check")

    def test_type_check_is_object(self):
        """Test type check categorization for is_object."""
        self.assertEqual(categorize("_sh_ljs_is_object"), "type_check")

    def test_type_check_is_number(self):
        """Test type check categorization for is_number."""
        self.assertEqual(categorize("_sh_ljs_is_number"), "type_check")

    def test_type_check_to_number(self):
        """Test type check categorization for to_number."""
        self.assertEqual(categorize("_sh_to_number"), "type_check")

    def test_type_check_double(self):
        """Test type check categorization for double."""
        self.assertEqual(categorize("_sh_ljs_double"), "type_check")

    def test_type_check_bool(self):
        """Test type check categorization for bool."""
        self.assertEqual(categorize("_sh_ljs_bool"), "type_check")

    def test_gc_push_locals(self):
        """Test GC categorization for push_locals."""
        self.assertEqual(categorize("_sh_push_locals"), "gc")

    def test_gc_pop_locals(self):
        """Test GC categorization for pop_locals."""
        self.assertEqual(categorize("_sh_pop_locals"), "gc")

    def test_gc_check_native_stack(self):
        """Test GC categorization for check_native_stack."""
        self.assertEqual(categorize("_sh_check_native_stack"), "gc")

    def test_gc_new_gcscope(self):
        """Test GC categorization for new_gcscope."""
        self.assertEqual(categorize("_sh_new_gcscope"), "gc")

    def test_arithmetic_add(self):
        """Test arithmetic categorization for add."""
        self.assertEqual(categorize("_sh_ljs_add_rjs"), "arithmetic")

    def test_arithmetic_sub(self):
        """Test arithmetic categorization for sub."""
        self.assertEqual(categorize("_sh_ljs_sub"), "arithmetic")

    def test_arithmetic_mul(self):
        """Test arithmetic categorization for mul."""
        self.assertEqual(categorize("_sh_ljs_mul"), "arithmetic")

    def test_arithmetic_div(self):
        """Test arithmetic categorization for div."""
        self.assertEqual(categorize("_sh_ljs_div"), "arithmetic")

    def test_arithmetic_mod(self):
        """Test arithmetic categorization for mod."""
        self.assertEqual(categorize("_sh_ljs_mod"), "arithmetic")

    def test_arithmetic_inc(self):
        """Test arithmetic categorization for inc."""
        self.assertEqual(categorize("_sh_ljs_inc"), "arithmetic")

    def test_arithmetic_negate(self):
        """Test arithmetic categorization for negate."""
        self.assertEqual(categorize("_sh_ljs_negate"), "arithmetic")

    def test_arithmetic_bitwise(self):
        """Test arithmetic categorization for bitwise."""
        self.assertEqual(categorize("_sh_ljs_bitand"), "arithmetic")

    def test_comparison_equal(self):
        """Test comparison categorization for equal."""
        self.assertEqual(categorize("_sh_ljs_equal"), "comparison")

    def test_comparison_strict_equal(self):
        """Test comparison categorization for strict_equal."""
        self.assertEqual(categorize("_sh_ljs_strict_equal"), "comparison")

    def test_comparison_less(self):
        """Test comparison categorization for less."""
        self.assertEqual(categorize("_sh_ljs_less"), "comparison")

    def test_comparison_greater(self):
        """Test comparison categorization for greater."""
        self.assertEqual(categorize("_sh_ljs_greater"), "comparison")

    def test_call_function(self):
        """Test call categorization for call."""
        self.assertEqual(categorize("_sh_ljs_call_rjs"), "call")

    def test_call_construct(self):
        """Test call categorization for construct."""
        self.assertEqual(categorize("_sh_ljs_construct"), "call")

    def test_call_generic(self):
        """Test call categorization for generic call."""
        self.assertEqual(categorize("_sh_call"), "call")

    def test_object_create(self):
        """Test object categorization for create."""
        self.assertEqual(categorize("_sh_ljs_create_object"), "object")

    def test_object_new_object(self):
        """Test object categorization for new_object."""
        self.assertEqual(categorize("_sh_ljs_new_object"), "object")

    def test_object_array(self):
        """Test object categorization for array."""
        self.assertEqual(categorize("_sh_ljs_array"), "object")

    def test_string_concat(self):
        """Test string categorization for concat."""
        self.assertEqual(categorize("_sh_ljs_string_concat"), "string")

    def test_string_operations(self):
        """Test string categorization for string operations."""
        self.assertEqual(categorize("_sh_ljs_string_length"), "string")
        self.assertEqual(categorize("_sh_string_compare"), "string")

    def test_other_category(self):
        """Test other category for unknown functions."""
        self.assertEqual(categorize("_sh_unknown_func"), "other")
        self.assertEqual(categorize("_sh_something_else"), "other")


class TestParsePerfReportSh(unittest.TestCase):
    """Tests for parse_perf_report_sh function."""

    def test_filters_to_sh_functions_only(self):
        """Test that only _sh_* functions are extracted."""
        lines = [
            "# Overhead  Samples  Command  Shared Object  Symbol",
            "    10.50%     1000  app      libhermes.so   [.] _sh_ljs_add_rjs",
            "     5.25%      500  app      libhermes.so   [.] some_other_func",
            "     3.00%      300  app      libhermes.so   [.] _sh_ljs_typeof",
            "     1.50%      150  app      libhermes.so   [.] not_sh_function",
        ]

        result = parse_perf_report_sh(lines)

        self.assertEqual(len(result), 2)
        self.assertEqual(result[0]["function"], "_sh_ljs_add_rjs")
        self.assertEqual(result[1]["function"], "_sh_ljs_typeof")

    def test_handles_format_with_sample_counts(self):
        """Test parsing format that includes sample counts."""
        lines = [
            "    10.50%     1000  app      libhermes.so   [.] _sh_ljs_add_rjs",
            "     5.25%      500  app      libhermes.so   [.] _sh_push_locals",
        ]

        result = parse_perf_report_sh(lines)

        self.assertEqual(len(result), 2)
        self.assertEqual(result[0]["function"], "_sh_ljs_add_rjs")
        self.assertEqual(result[0]["percentage"], 10.50)
        self.assertEqual(result[0]["samples"], 1000)
        self.assertEqual(result[0]["module"], "libhermes.so")
        self.assertEqual(result[0]["category"], "arithmetic")

        self.assertEqual(result[1]["function"], "_sh_push_locals")
        self.assertEqual(result[1]["percentage"], 5.25)
        self.assertEqual(result[1]["samples"], 500)
        self.assertEqual(result[1]["category"], "gc")

    def test_handles_format_without_sample_counts(self):
        """Test parsing format without sample counts."""
        lines = [
            "    10.50%  app      libhermes.so   [.] _sh_ljs_add_rjs",
            "     5.25%  app      libhermes.so   [.] _sh_push_locals",
        ]

        result = parse_perf_report_sh(lines)

        self.assertEqual(len(result), 2)
        self.assertEqual(result[0]["function"], "_sh_ljs_add_rjs")
        self.assertEqual(result[0]["percentage"], 10.50)
        self.assertEqual(result[0]["samples"], 0)  # Default when not present
        self.assertEqual(result[0]["module"], "libhermes.so")

    def test_skips_comments_and_empty_lines(self):
        """Test that comments and empty lines are skipped."""
        lines = [
            "# Comment line",
            "",
            "    10.50%     1000  app      libhermes.so   [.] _sh_ljs_add_rjs",
            "# Another comment",
            "",
            "     5.25%      500  app      libhermes.so   [.] _sh_push_locals",
            "",
        ]

        result = parse_perf_report_sh(lines)

        self.assertEqual(len(result), 2)

    def test_results_sorted_by_percentage_descending(self):
        """Test that results are sorted by percentage in descending order."""
        lines = [
            "     3.00%      300  app      libhermes.so   [.] _sh_ljs_typeof",
            "    10.50%     1000  app      libhermes.so   [.] _sh_ljs_add_rjs",
            "     5.25%      500  app      libhermes.so   [.] _sh_push_locals",
            "     8.00%      800  app      libhermes.so   [.] _sh_ljs_call_rjs",
        ]

        result = parse_perf_report_sh(lines)

        self.assertEqual(len(result), 4)
        self.assertEqual(result[0]["percentage"], 10.50)
        self.assertEqual(result[1]["percentage"], 8.00)
        self.assertEqual(result[2]["percentage"], 5.25)
        self.assertEqual(result[3]["percentage"], 3.00)

    def test_empty_input(self):
        """Test handling of empty input."""
        lines = []
        result = parse_perf_report_sh(lines)
        self.assertEqual(len(result), 0)

    def test_module_field_extraction(self):
        """Test that module field is correctly extracted."""
        lines = [
            "    10.50%     1000  app      libhermes.so   [.] _sh_ljs_add_rjs",
            "     5.25%      500  app      myapp.exe      [.] _sh_push_locals",
        ]

        result = parse_perf_report_sh(lines)

        self.assertEqual(result[0]["module"], "libhermes.so")
        self.assertEqual(result[1]["module"], "myapp.exe")


class TestBuildReport(unittest.TestCase):
    """Tests for build_report function."""

    def test_correct_category_aggregation(self):
        """Test that categories are correctly aggregated."""
        entries = [
            {
                "function": "_sh_ljs_add_rjs",
                "module": "lib.so",
                "samples": 1000,
                "percentage": 10.0,
                "category": "arithmetic",
            },
            {
                "function": "_sh_ljs_sub",
                "module": "lib.so",
                "samples": 500,
                "percentage": 5.0,
                "category": "arithmetic",
            },
            {
                "function": "_sh_push_locals",
                "module": "lib.so",
                "samples": 800,
                "percentage": 8.0,
                "category": "gc",
            },
            {
                "function": "_sh_ljs_get_by_id_rjs",
                "module": "lib.so",
                "samples": 600,
                "percentage": 6.0,
                "category": "property_access",
            },
        ]

        report = build_report(entries)

        # Check summary totals
        self.assertEqual(report["total_sh_functions"], 4)
        self.assertEqual(report["total_sh_percentage"], 29.0)
        self.assertEqual(report["total_sh_samples"], 2900)

        # Check category aggregation
        categories = {cat["category"]: cat for cat in report["categories"]}
        self.assertEqual(len(categories), 3)

        self.assertEqual(categories["arithmetic"]["total_percentage"], 15.0)
        self.assertEqual(categories["arithmetic"]["total_samples"], 1500)
        self.assertEqual(categories["arithmetic"]["function_count"], 2)

        self.assertEqual(categories["gc"]["total_percentage"], 8.0)
        self.assertEqual(categories["gc"]["total_samples"], 800)
        self.assertEqual(categories["gc"]["function_count"], 1)

        self.assertEqual(categories["property_access"]["total_percentage"], 6.0)
        self.assertEqual(categories["property_access"]["total_samples"], 600)
        self.assertEqual(categories["property_access"]["function_count"], 1)

    def test_categories_sorted_by_percentage(self):
        """Test that categories are sorted by total percentage descending."""
        entries = [
            {
                "function": "_sh_push_locals",
                "module": "lib.so",
                "samples": 800,
                "percentage": 8.0,
                "category": "gc",
            },
            {
                "function": "_sh_ljs_add_rjs",
                "module": "lib.so",
                "samples": 1000,
                "percentage": 10.0,
                "category": "arithmetic",
            },
            {
                "function": "_sh_ljs_get_by_id_rjs",
                "module": "lib.so",
                "samples": 600,
                "percentage": 6.0,
                "category": "property_access",
            },
        ]

        report = build_report(entries)

        categories = report["categories"]
        self.assertEqual(categories[0]["category"], "arithmetic")
        self.assertEqual(categories[1]["category"], "gc")
        self.assertEqual(categories[2]["category"], "property_access")

    def test_top_n_limits_optimization_candidates(self):
        """Test that top_n parameter limits optimization candidates."""
        entries = [
            {
                "function": f"_sh_func_{i}",
                "module": "lib.so",
                "samples": 100 - i,
                "percentage": 10.0 - i * 0.1,
                "category": "other",
            }
            for i in range(20)
        ]

        report = build_report(entries, top_n=5)

        self.assertEqual(len(report["optimization_candidates"]), 5)
        self.assertEqual(report["optimization_candidates"][0]["function"], "_sh_func_0")
        self.assertEqual(report["optimization_candidates"][4]["function"], "_sh_func_4")

    def test_top_n_none_returns_all(self):
        """Test that top_n=None returns all entries as candidates."""
        entries = [
            {
                "function": f"_sh_func_{i}",
                "module": "lib.so",
                "samples": 100,
                "percentage": 10.0,
                "category": "other",
            }
            for i in range(10)
        ]

        report = build_report(entries, top_n=None)

        self.assertEqual(len(report["optimization_candidates"]), 10)

    def test_empty_entries(self):
        """Test handling of empty entries list."""
        entries = []

        report = build_report(entries)

        self.assertEqual(report["total_sh_functions"], 0)
        self.assertEqual(report["total_sh_percentage"], 0)
        self.assertEqual(report["total_sh_samples"], 0)
        self.assertEqual(len(report["categories"]), 0)
        self.assertEqual(len(report["optimization_candidates"]), 0)


class TestFormatHuman(unittest.TestCase):
    """Tests for format_human function."""

    def test_contains_header(self):
        """Test that output contains the header."""
        entries = [
            {
                "function": "_sh_ljs_add_rjs",
                "module": "lib.so",
                "samples": 1000,
                "percentage": 10.0,
                "category": "arithmetic",
            },
        ]
        report = build_report(entries)

        output = format_human(report)

        self.assertIn("Static Hermes Runtime Call Profile", output)
        self.assertIn("_sh_* functions", output)

    def test_contains_summary_statistics(self):
        """Test that output contains summary statistics."""
        entries = [
            {
                "function": "_sh_ljs_add_rjs",
                "module": "lib.so",
                "samples": 1000,
                "percentage": 10.0,
                "category": "arithmetic",
            },
            {
                "function": "_sh_push_locals",
                "module": "lib.so",
                "samples": 500,
                "percentage": 5.0,
                "category": "gc",
            },
        ]
        report = build_report(entries)

        output = format_human(report)

        self.assertIn("Total _sh_* functions profiled: 2", output)
        self.assertIn("Total _sh_* percentage:         15.00%", output)
        self.assertIn("Total _sh_* samples:            1500", output)

    def test_contains_category_breakdown(self):
        """Test that output contains category breakdown."""
        entries = [
            {
                "function": "_sh_ljs_add_rjs",
                "module": "lib.so",
                "samples": 1000,
                "percentage": 10.0,
                "category": "arithmetic",
            },
            {
                "function": "_sh_push_locals",
                "module": "lib.so",
                "samples": 500,
                "percentage": 5.0,
                "category": "gc",
            },
        ]
        report = build_report(entries)

        output = format_human(report)

        self.assertIn("Category Breakdown", output)
        self.assertIn("arithmetic", output)
        self.assertIn("gc", output)
        self.assertIn("10.00%", output)
        self.assertIn("5.00%", output)

    def test_contains_optimization_candidates(self):
        """Test that output contains optimization candidates."""
        entries = [
            {
                "function": "_sh_ljs_add_rjs",
                "module": "lib.so",
                "samples": 1000,
                "percentage": 10.0,
                "category": "arithmetic",
            },
            {
                "function": "_sh_push_locals",
                "module": "lib.so",
                "samples": 500,
                "percentage": 5.0,
                "category": "gc",
            },
        ]
        report = build_report(entries, top_n=2)

        output = format_human(report, top_n=2)

        self.assertIn("Optimization Candidates", output)
        self.assertIn("_sh_ljs_add_rjs", output)
        self.assertIn("_sh_push_locals", output)

    def test_empty_report(self):
        """Test formatting of empty report."""
        entries = []
        report = build_report(entries)

        output = format_human(report)

        self.assertIn("Total _sh_* functions profiled: 0", output)
        self.assertIn("Total _sh_* percentage:         0.00%", output)
        self.assertIn("Total _sh_* samples:            0", output)


if __name__ == "__main__":
    unittest.main()
