#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for annotate_disasm.py parsing functions."""

import sys
import unittest
from pathlib import Path

# Add tools directory to path for imports.
TOOLS_DIR = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(TOOLS_DIR))

from annotate_disasm import format_human, parse_disasm_annotations


class TestParseDisasmAnnotations(unittest.TestCase):
    """Tests for parse_disasm_annotations function."""

    def test_parse_assembly_line_with_percentage(self):
        """Test parsing assembly lines with percentages."""
        lines = [
            "  someFunction():",
            "  12.30  :  4005a3: callq  0x400480 <malloc@plt>",
            "   5.50  :  4005a8: test   %rax,%rax",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 2)

        self.assertEqual(results[0]["address"], "0x4005a3")
        self.assertEqual(results[0]["instruction"], "callq  0x400480 <malloc@plt>")
        self.assertEqual(results[0]["mnemonic"], "callq")
        self.assertEqual(results[0]["percentage"], 12.30)
        self.assertEqual(results[0]["hot"], True)
        self.assertEqual(results[0]["function"], "someFunction()")

        self.assertEqual(results[1]["address"], "0x4005a8")
        self.assertEqual(results[1]["instruction"], "test   %rax,%rax")
        self.assertEqual(results[1]["mnemonic"], "test")
        self.assertEqual(results[1]["percentage"], 5.50)
        self.assertEqual(results[1]["hot"], True)

    def test_parse_cold_assembly_lines(self):
        """Test parsing cold assembly lines (no percentage)."""
        lines = [
            "  func():",
            "        :  4005a0: mov    %rdi,%rbx",
            "        :  4005a3: push   %rbp",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 2)

        self.assertEqual(results[0]["percentage"], 0.0)
        self.assertEqual(results[0]["hot"], False)
        self.assertEqual(results[0]["address"], "0x4005a0")
        self.assertEqual(results[0]["mnemonic"], "mov")

        self.assertEqual(results[1]["percentage"], 0.0)
        self.assertEqual(results[1]["hot"], False)
        self.assertEqual(results[1]["address"], "0x4005a3")
        self.assertEqual(results[1]["mnemonic"], "push")

    def test_function_filter_case_insensitive(self):
        """Test function filter is case-insensitive substring match."""
        lines = [
            "  firstFunc():",
            "  1.00  :  400000: nop",
            "  secondFunc():",
            "  2.00  :  500000: ret",
        ]

        # Filter for "first".
        results = parse_disasm_annotations(lines, function_filter="first")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["function"], "firstFunc()")

        # Case-insensitive.
        results = parse_disasm_annotations(lines, function_filter="FIRST")
        self.assertEqual(len(results), 1)

        # Filter for "second".
        results = parse_disasm_annotations(lines, function_filter="second")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["function"], "secondFunc()")

        # Filter for "Func" should match both.
        results = parse_disasm_annotations(lines, function_filter="Func")
        self.assertEqual(len(results), 2)

    def test_mnemonic_extraction(self):
        """Test that mnemonic is correctly extracted as first word."""
        lines = [
            "  func():",
            "  1.00  :  400000: mov    %rax,%rbx",
            "  1.00  :  400001: callq  0x400100",
            "  1.00  :  400002: jmp    *%rax",
            "  1.00  :  400003: retq",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 4)
        self.assertEqual(results[0]["mnemonic"], "mov")
        self.assertEqual(results[1]["mnemonic"], "callq")
        self.assertEqual(results[2]["mnemonic"], "jmp")
        self.assertEqual(results[3]["mnemonic"], "retq")

    def test_skip_percent_header_lines(self):
        """Test that 'Percent' header lines are skipped."""
        lines = [
            "Percent | Source code & Disassembly:",
            "--------+----------------------------",
            "  func():",
            "  1.00  :  400000: nop",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["instruction"], "nop")

    def test_format_human_with_hot_markers(self):
        """Test format_human uses >>> markers for hot instructions."""
        results = [
            {
                "address": "0x400000",
                "instruction": "mov %rax,%rbx",
                "mnemonic": "mov",
                "percentage": 0.50,
                "hot": False,
                "function": "func()",
            },
            {
                "address": "0x400003",
                "instruction": "callq 0x500000",
                "mnemonic": "callq",
                "percentage": 5.00,
                "hot": True,
                "function": "func()",
            },
        ]
        output = format_human(results)

        self.assertIn(">>>   5.0%", output)
        self.assertIn("0.50%", output)
        self.assertIn("mov %rax,%rbx", output)
        self.assertIn("callq 0x500000", output)

    def test_format_human_with_function_headers(self):
        """Test format_human adds function headers."""
        results = [
            {
                "address": "0x400000",
                "instruction": "nop",
                "mnemonic": "nop",
                "percentage": 1.0,
                "hot": True,
                "function": "func1()",
            },
            {
                "address": "0x500000",
                "instruction": "ret",
                "mnemonic": "ret",
                "percentage": 1.0,
                "hot": True,
                "function": "func2()",
            },
        ]
        output = format_human(results)

        self.assertIn("=== func1() ===", output)
        self.assertIn("=== func2() ===", output)

    def test_format_human_empty_results(self):
        """Test format_human with empty results."""
        output = format_human([])
        self.assertEqual(output, "No disassembly entries found.")

    def test_multiple_functions(self):
        """Test parsing multiple functions."""
        lines = [
            "  func1():",
            "  1.00  :  100000: nop",
            "  func2():",
            "  2.00  :  200000: ret",
            "  func3():",
            "  3.00  :  300000: jmp 0x0",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 3)
        self.assertEqual(results[0]["function"], "func1()")
        self.assertEqual(results[1]["function"], "func2()")
        self.assertEqual(results[2]["function"], "func3()")

    def test_hot_threshold_exactly_one_percent(self):
        """Test that exactly 1.0% is considered hot."""
        lines = [
            "  func():",
            "  0.99  :  400000: below",
            "  1.00  :  400001: exactly",
            "  1.01  :  400002: above",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 3)
        self.assertEqual(results[0]["hot"], False)
        self.assertEqual(results[1]["hot"], True)
        self.assertEqual(results[2]["hot"], True)

    def test_empty_input(self):
        """Test parsing empty input."""
        results = parse_disasm_annotations([])
        self.assertEqual(results, [])

    def test_address_0x_prefix(self):
        """Test that addresses get 0x prefix."""
        lines = [
            "  func():",
            "  1.00  :  4005a3: nop",
            "  1.00  :  abc123: ret",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 2)
        self.assertEqual(results[0]["address"], "0x4005a3")
        self.assertEqual(results[1]["address"], "0xabc123")

    def test_unknown_function_when_no_header(self):
        """Test that function is <unknown> when no header seen."""
        lines = [
            "  1.00  :  400000: nop",
        ]
        results = parse_disasm_annotations(lines)

        # Without function header, should still parse but function is None.
        # Based on code, current_function starts as None.
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["function"], "<unknown>")

    def test_format_human_cold_instruction_no_marker(self):
        """Test format_human uses spaces for cold (0%) instructions."""
        results = [
            {
                "address": "0x400000",
                "instruction": "nop",
                "mnemonic": "nop",
                "percentage": 0.0,
                "hot": False,
                "function": "func()",
            },
        ]
        output = format_human(results)

        lines = output.split("\n")
        data_line = [line for line in lines if "nop" in line][0]
        # Should have spaces, not percentage marker.
        self.assertTrue("           " in data_line)

    def test_complex_instruction_formats(self):
        """Test various complex instruction formats."""
        lines = [
            "  func():",
            "  1.00  :  400000: lea    0x8(%rsp),%rax",
            "  1.00  :  400005: movq   $0x0,0x10(%rbp)",
            "  1.00  :  40000d: callq  *%rax",
            "  1.00  :  40000f: jmpq   *0x2002b2(%rip)",
        ]
        results = parse_disasm_annotations(lines)

        self.assertEqual(len(results), 4)
        self.assertEqual(results[0]["mnemonic"], "lea")
        self.assertEqual(results[1]["mnemonic"], "movq")
        self.assertEqual(results[2]["mnemonic"], "callq")
        self.assertEqual(results[3]["mnemonic"], "jmpq")

    def test_format_human_function_separator_blank_line(self):
        """Test that blank line separates different functions."""
        results = [
            {
                "address": "0x400000",
                "instruction": "nop",
                "mnemonic": "nop",
                "percentage": 1.0,
                "hot": True,
                "function": "func1()",
            },
            {
                "address": "0x500000",
                "instruction": "ret",
                "mnemonic": "ret",
                "percentage": 1.0,
                "hot": True,
                "function": "func2()",
            },
        ]
        output = format_human(results)
        lines = output.split("\n")

        # Find blank lines between function sections.
        blank_indices = [i for i, line in enumerate(lines) if line == ""]
        self.assertGreater(len(blank_indices), 0)


if __name__ == "__main__":
    unittest.main()
