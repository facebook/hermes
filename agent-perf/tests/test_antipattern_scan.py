#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for antipattern_scan.py"""

import os
import sys
import tempfile
import unittest

# Add tools directory to path to import the module
TOOLS_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "tools")
sys.path.insert(0, TOOLS_DIR)

from antipattern_scan import collect_files, format_human, scan_file


class TestAntipatternScan(unittest.TestCase):
    def test_pass_by_value_detected(self):
        """Test that pass-by-value of large types is detected."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("void foo(std::string s) { return; }\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            self.assertEqual(len(findings), 1)
            self.assertEqual(findings[0]["pattern"], "pass_by_value")
            self.assertIn("std::string", findings[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_pass_by_reference_not_detected(self):
        """Test that pass-by-reference is not flagged."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("void foo(const std::string& s) { return; }\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            # Should not detect pass_by_value for const reference
            pass_by_value = [f for f in findings if f["pattern"] == "pass_by_value"]
            self.assertEqual(len(pass_by_value), 0)
        finally:
            os.unlink(filepath)

    def test_repeated_lookup_detected(self):
        """Test that repeated container lookups are detected."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("int x = map[key] + map[key];\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            repeated = [f for f in findings if f["pattern"] == "repeated_lookup"]
            self.assertEqual(len(repeated), 1)
            self.assertIn("map[key]", repeated[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_string_concat_in_loop_detected(self):
        """Test that string concatenation in loops is detected."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("for (int i = 0; i < 10; i++) {\n")
            f.write('  str += "x";\n')
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            concat = [f for f in findings if f["pattern"] == "string_concat_loop"]
            self.assertGreater(len(concat), 0)
        finally:
            os.unlink(filepath)

    def test_string_concat_outside_loop_not_detected(self):
        """Test that string concatenation outside loops is not flagged."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write('str += "x";\n')
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            concat = [f for f in findings if f["pattern"] == "string_concat_loop"]
            self.assertEqual(len(concat), 0)
        finally:
            os.unlink(filepath)

    def test_virtual_dispatch_detected(self):
        """Test that virtual function declarations are detected."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("virtual void foo();\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            virtual = [f for f in findings if f["pattern"] == "virtual_dispatch_hot"]
            self.assertEqual(len(virtual), 1)
        finally:
            os.unlink(filepath)

    def test_virtual_in_comment_not_detected(self):
        """Test that 'virtual' in comments is not flagged."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("// This is a virtual function comment\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            virtual = [f for f in findings if f["pattern"] == "virtual_dispatch_hot"]
            self.assertEqual(len(virtual), 0)
        finally:
            os.unlink(filepath)

    def test_unnecessary_copy_detected(self):
        """Test that unnecessary auto copies are detected."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("auto x = map.find(k);\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            copy = [f for f in findings if f["pattern"] == "unnecessary_copy"]
            self.assertEqual(len(copy), 1)
            self.assertIn("auto", copy[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_redundant_find_access_detected(self):
        """Test that find() followed by operator[] is detected."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            f.write("auto it = map.find(key);\n")
            f.write("int val = map[key];\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_file(filepath)
            redundant = [f for f in findings if f["pattern"] == "redundant_find_access"]
            self.assertEqual(len(redundant), 1)
            self.assertIn("map.find", redundant[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_format_human_empty(self):
        """Test format_human with no findings."""
        result = format_human([])
        self.assertEqual(result, "No anti-patterns detected.")

    def test_format_human_with_findings(self):
        """Test format_human with findings."""
        findings = [
            {
                "file": "/path/to/test.cpp",
                "line": 10,
                "pattern": "pass_by_value",
                "detail": "std::string passed by value",
                "estimated_impact": "medium",
            }
        ]
        result = format_human(findings)
        self.assertIn("Found 1 potential anti-pattern", result)
        self.assertIn("pass_by_value", result)
        self.assertIn("medium", result)

    def test_collect_files_single_file(self):
        """Test collect_files with a single file."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".cpp", delete=False) as f:
            filepath = f.name

        try:
            files = collect_files(filepath)
            self.assertEqual(files, [filepath])
        finally:
            os.unlink(filepath)

    def test_collect_files_directory(self):
        """Test collect_files with a directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            # Create some C++ files
            cpp1 = os.path.join(tmpdir, "file1.cpp")
            cpp2 = os.path.join(tmpdir, "file2.h")
            txt = os.path.join(tmpdir, "readme.txt")

            open(cpp1, "w").close()
            open(cpp2, "w").close()
            open(txt, "w").close()

            files = collect_files(tmpdir)
            self.assertEqual(len(files), 2)
            self.assertIn(cpp1, files)
            self.assertIn(cpp2, files)
            self.assertNotIn(txt, files)


if __name__ == "__main__":
    unittest.main()
