#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for regalloc_report.py"""

import os
import sys
import unittest

# Add tools directory to path to import the module
TOOLS_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "tools")
sys.path.insert(0, TOOLS_DIR)

from regalloc_report import build_report, format_human, parse_dump_ra


class TestRegAllocReport(unittest.TestCase):
    def test_parse_dump_ra_function_boundaries(self):
        """Test that function boundaries are correctly detected."""
        lines = [
            "function foo(a, b) {\n",
            "  %r0 = LoadParamInst a\n",
            "  %r1 = LoadParamInst b\n",
            "  %r2 = AddInst %r0, %r1\n",
            "  ReturnInst %r2\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(len(functions), 1)
        self.assertEqual(functions[0]["name"], "foo")

    def test_parse_dump_ra_collect_registers(self):
        """Test that register numbers are collected."""
        lines = [
            "function foo(a) {\n",
            "  %r0 = LoadParamInst a\n",
            "  %r1 = ConstantInst 1\n",
            "  %r5 = AddInst %r0, %r1\n",
            "  ReturnInst %r5\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(functions[0]["registers_used"], 3)  # r0, r1, r5
        self.assertEqual(functions[0]["max_register"], 5)
        self.assertIn(0, functions[0]["register_set"])
        self.assertIn(1, functions[0]["register_set"])
        self.assertIn(5, functions[0]["register_set"])

    def test_parse_dump_ra_count_instructions(self):
        """Test that instructions are counted."""
        lines = [
            "function foo(a) {\n",
            "  %r0 = LoadParamInst a\n",
            "  %r1 = ConstantInst 1\n",
            "  %r2 = AddInst %r0, %r1\n",
            "  ReturnInst %r2\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        # Should count lines with = and (Inst or %r\d+=)
        self.assertGreater(functions[0]["instruction_count"], 0)

    def test_parse_dump_ra_detect_spills_store_load(self):
        """Test detection of spills via StoreStackInst/LoadStackInst."""
        lines = [
            "function foo(a) {\n",
            "  %r0 = LoadParamInst a\n",
            "  StoreStackInst %r0, %stack1\n",
            "  %r1 = LoadStackInst %stack1\n",
            "  ReturnInst %r1\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(functions[0]["spill_count"], 2)
        self.assertIn(1, functions[0]["spill_slots"])

    def test_parse_dump_ra_detect_spills_markers(self):
        """Test detection of spills via Spill/Reload markers."""
        lines = [
            "function foo(a) {\n",
            "  %r0 = LoadParamInst a\n",
            "  ; Spill %r0\n",
            "  ; Reload %r0\n",
            "  ReturnInst %r0\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(functions[0]["spill_count"], 2)

    def test_parse_dump_ra_extract_spill_slots(self):
        """Test extraction of spill slot numbers."""
        lines = [
            "function foo(a) {\n",
            "  StoreStackInst %r0, %stack5\n",
            "  LoadStackInst %stack5\n",
            "  Mov %r1, %stack10\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertIn(5, functions[0]["spill_slots"])
        self.assertIn(10, functions[0]["spill_slots"])

    def test_parse_dump_ra_function_with_id(self):
        """Test parsing function#id format."""
        lines = [
            "function bar#123(x) {\n",
            "  %r0 = LoadParamInst x\n",
            "  ReturnInst %r0\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(len(functions), 1)
        self.assertEqual(functions[0]["name"], "bar")

    def test_parse_dump_ra_multiple_functions(self):
        """Test parsing multiple functions."""
        lines = [
            "function foo(a) {\n",
            "  %r0 = LoadParamInst a\n",
            "  ReturnInst %r0\n",
            "}\n",
            "function bar(b) {\n",
            "  %r0 = LoadParamInst b\n",
            "  %r1 = ConstantInst 1\n",
            "  ReturnInst %r1\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(len(functions), 2)
        self.assertEqual(functions[0]["name"], "foo")
        self.assertEqual(functions[1]["name"], "bar")

    def test_build_report_summary_totals(self):
        """Test that build_report computes correct summary totals."""
        functions = [
            {
                "name": "foo",
                "registers_used": 3,
                "max_register": 5,
                "spill_count": 2,
                "instruction_count": 10,
            },
            {
                "name": "bar",
                "registers_used": 5,
                "max_register": 8,
                "spill_count": 4,
                "instruction_count": 20,
            },
        ]
        report = build_report(functions)
        self.assertEqual(report["total_functions"], 2)
        self.assertEqual(report["summary"]["total_registers_across_functions"], 8)
        self.assertEqual(report["summary"]["total_spills_across_functions"], 6)
        self.assertEqual(report["summary"]["max_register_pressure"], 8)
        self.assertEqual(report["summary"]["avg_registers_per_function"], 4.0)
        self.assertEqual(report["summary"]["avg_spills_per_function"], 3.0)

    def test_build_report_high_spill_detection_count(self):
        """Test detection of high-spill functions based on count."""
        functions = [
            {
                "name": "foo",
                "registers_used": 3,
                "max_register": 5,
                "spill_count": 15,
                "instruction_count": 100,
            },
            {
                "name": "bar",
                "registers_used": 5,
                "max_register": 8,
                "spill_count": 2,
                "instruction_count": 20,
            },
        ]
        report = build_report(functions)
        self.assertEqual(len(report["high_spill_functions"]), 1)
        self.assertEqual(report["high_spill_functions"][0]["name"], "foo")

    def test_build_report_high_spill_detection_ratio(self):
        """Test detection of high-spill functions based on ratio."""
        functions = [
            {
                "name": "foo",
                "registers_used": 3,
                "max_register": 5,
                "spill_count": 5,
                "instruction_count": 20,  # 5/20 = 25% > 20%
            },
            {
                "name": "bar",
                "registers_used": 5,
                "max_register": 8,
                "spill_count": 2,
                "instruction_count": 50,  # 2/50 = 4% < 20%
            },
        ]
        report = build_report(functions)
        self.assertEqual(len(report["high_spill_functions"]), 1)
        self.assertEqual(report["high_spill_functions"][0]["name"], "foo")

    def test_build_report_functions_sorted_by_max_register(self):
        """Test that functions are sorted by max_register descending."""
        functions = [
            {
                "name": "foo",
                "registers_used": 3,
                "max_register": 5,
                "spill_count": 0,
                "instruction_count": 10,
            },
            {
                "name": "bar",
                "registers_used": 5,
                "max_register": 10,
                "spill_count": 0,
                "instruction_count": 20,
            },
            {
                "name": "baz",
                "registers_used": 2,
                "max_register": 3,
                "spill_count": 0,
                "instruction_count": 5,
            },
        ]
        report = build_report(functions)
        self.assertEqual(report["functions"][0]["name"], "bar")
        self.assertEqual(report["functions"][1]["name"], "foo")
        self.assertEqual(report["functions"][2]["name"], "baz")

    def test_build_report_empty_input(self):
        """Test build_report with empty input."""
        report = build_report([])
        self.assertEqual(report["total_functions"], 0)
        self.assertEqual(report["functions"], [])
        self.assertEqual(report["high_spill_functions"], [])
        self.assertEqual(report["summary"], {})

    def test_format_human_with_high_spill_functions(self):
        """Test format_human with high spill functions."""
        report = {
            "total_functions": 2,
            "summary": {
                "total_registers_across_functions": 10,
                "total_spills_across_functions": 15,
                "max_register_pressure": 8,
                "avg_registers_per_function": 5.0,
                "avg_spills_per_function": 7.5,
            },
            "functions": [
                {
                    "name": "foo",
                    "registers_used": 5,
                    "max_register": 8,
                    "spill_count": 12,
                    "instruction_count": 50,
                }
            ],
            "high_spill_functions": [
                {
                    "name": "foo",
                    "registers_used": 5,
                    "max_register": 8,
                    "spill_count": 12,
                    "instruction_count": 50,
                }
            ],
        }
        result = format_human(report)
        self.assertIn("Register Allocation Quality Report", result)
        self.assertIn("High Spill Functions", result)
        self.assertIn("foo", result)

    def test_format_human_no_functions(self):
        """Test format_human with no functions."""
        report = {
            "total_functions": 0,
            "functions": [],
            "high_spill_functions": [],
            "summary": {},
        }
        result = format_human(report)
        self.assertIn("No functions found", result)
        self.assertIn("shermes -dump-ra", result)

    def test_parse_dump_ra_scope_function(self):
        """Test parsing functions with 'scope' prefix."""
        lines = [
            "scope function foo(a) {\n",
            "  %r0 = LoadParamInst a\n",
            "  ReturnInst %r0\n",
            "}\n",
        ]
        functions = parse_dump_ra(lines)
        self.assertEqual(len(functions), 1)
        self.assertEqual(functions[0]["name"], "foo")


if __name__ == "__main__":
    unittest.main()
