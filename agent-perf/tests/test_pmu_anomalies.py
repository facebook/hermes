#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for pmu_anomalies.py."""

import os
import sys
import unittest

# Add tools directory to path for imports
TOOLS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tools")
sys.path.insert(0, TOOLS_DIR)

from pmu_anomalies import compute_metrics, detect_anomalies, format_human, safe_div


class TestSafeDiv(unittest.TestCase):
    """Tests for safe_div function."""

    def test_normal_division(self):
        """Test normal division returns correct result."""
        self.assertEqual(safe_div(10.0, 2.0), 5.0)
        self.assertEqual(safe_div(7.0, 2.0), 3.5)
        self.assertEqual(safe_div(1.0, 3.0), 1.0 / 3.0)

    def test_division_by_zero(self):
        """Test division by zero returns None."""
        self.assertIsNone(safe_div(10.0, 0.0))
        self.assertIsNone(safe_div(0.0, 0.0))
        self.assertIsNone(safe_div(-5.0, 0.0))

    def test_zero_numerator(self):
        """Test zero numerator with non-zero denominator."""
        self.assertEqual(safe_div(0.0, 5.0), 0.0)

    def test_negative_values(self):
        """Test division with negative values."""
        self.assertEqual(safe_div(-10.0, 2.0), -5.0)
        self.assertEqual(safe_div(10.0, -2.0), -5.0)
        self.assertEqual(safe_div(-10.0, -2.0), 5.0)


class TestComputeMetrics(unittest.TestCase):
    """Tests for compute_metrics function."""

    def test_all_counters_present(self):
        """Test metrics computation with all counters present."""
        data = {
            "instructions": 1000000,
            "cycles": 500000,
            "branch_instructions": 200000,
            "branch_misses": 20000,
            "L1_icache_loads": 500000,
            "L1_icache_misses": 25000,
            "L1_dcache_loads": 800000,
            "L1_dcache_misses": 80000,
            "dTLB_loads": 800000,
            "dTLB_misses": 24000,
        }

        metrics = compute_metrics(data)

        self.assertAlmostEqual(metrics["ipc"], 2.0, places=4)
        self.assertAlmostEqual(metrics["branch_miss_rate"], 10.0, places=4)
        self.assertAlmostEqual(metrics["l1_icache_miss_rate"], 5.0, places=4)
        self.assertAlmostEqual(metrics["l1_dcache_miss_rate"], 10.0, places=4)
        self.assertAlmostEqual(metrics["dtlb_miss_rate"], 3.0, places=4)

    def test_missing_counters(self):
        """Test metrics computation with missing counters returns None values."""
        data = {
            "instructions": 1000000,
            "cycles": 500000,
            # Missing branch counters
            # Missing cache counters
        }

        metrics = compute_metrics(data)

        self.assertAlmostEqual(metrics["ipc"], 2.0, places=4)
        self.assertIsNone(metrics["branch_miss_rate"])
        self.assertIsNone(metrics["l1_icache_miss_rate"])
        self.assertIsNone(metrics["l1_dcache_miss_rate"])
        self.assertIsNone(metrics["dtlb_miss_rate"])

    def test_zero_denominators(self):
        """Test metrics computation with zero denominators returns None."""
        data = {
            "instructions": 1000000,
            "cycles": 0,  # Zero cycles
            "branch_instructions": 0,  # Zero branch instructions
            "branch_misses": 10,
            "L1_icache_loads": 0,  # Zero icache loads
            "L1_icache_misses": 5,
            "L1_dcache_loads": 0,  # Zero dcache loads
            "L1_dcache_misses": 10,
            "dTLB_loads": 0,  # Zero TLB loads
            "dTLB_misses": 2,
        }

        metrics = compute_metrics(data)

        self.assertIsNone(metrics["ipc"])
        self.assertIsNone(metrics["branch_miss_rate"])
        self.assertIsNone(metrics["l1_icache_miss_rate"])
        self.assertIsNone(metrics["l1_dcache_miss_rate"])
        self.assertIsNone(metrics["dtlb_miss_rate"])

    def test_all_zero_counters(self):
        """Test metrics computation with all zero counters."""
        data = {
            "instructions": 0,
            "cycles": 0,
            "branch_instructions": 0,
            "branch_misses": 0,
            "L1_icache_loads": 0,
            "L1_icache_misses": 0,
            "L1_dcache_loads": 0,
            "L1_dcache_misses": 0,
            "dTLB_loads": 0,
            "dTLB_misses": 0,
        }

        metrics = compute_metrics(data)

        # All should be None due to division by zero
        self.assertIsNone(metrics["ipc"])
        self.assertIsNone(metrics["branch_miss_rate"])
        self.assertIsNone(metrics["l1_icache_miss_rate"])
        self.assertIsNone(metrics["l1_dcache_miss_rate"])
        self.assertIsNone(metrics["dtlb_miss_rate"])

    def test_empty_data(self):
        """Test metrics computation with empty data dict."""
        data = {}

        metrics = compute_metrics(data)

        # All should be None due to missing/zero counters
        self.assertIsNone(metrics["ipc"])
        self.assertIsNone(metrics["branch_miss_rate"])
        self.assertIsNone(metrics["l1_icache_miss_rate"])
        self.assertIsNone(metrics["l1_dcache_miss_rate"])
        self.assertIsNone(metrics["dtlb_miss_rate"])


class TestDetectAnomalies(unittest.TestCase):
    """Tests for detect_anomalies function."""

    def test_low_ipc_triggers(self):
        """Test low IPC anomaly detection."""
        metrics = {
            "ipc": 0.5,  # Below threshold of 1.0
            "branch_miss_rate": 5.0,
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 1.0,
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 1)
        self.assertEqual(anomalies[0]["anomaly"], "low_ipc")
        self.assertEqual(anomalies[0]["measured_value"], 0.5)
        self.assertEqual(anomalies[0]["threshold"], 1.0)
        self.assertEqual(anomalies[0]["direction"], "below")
        self.assertIn("pipeline stalls", anomalies[0]["hint"])

    def test_high_branch_miss_triggers(self):
        """Test high branch miss rate anomaly detection."""
        metrics = {
            "ipc": 2.0,
            "branch_miss_rate": 15.0,  # Above threshold of 10%
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 1.0,
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 1)
        self.assertEqual(anomalies[0]["anomaly"], "high_branch_miss")
        self.assertEqual(anomalies[0]["measured_value"], 15.0)
        self.assertEqual(anomalies[0]["threshold"], 10.0)
        self.assertEqual(anomalies[0]["direction"], "above")
        self.assertIn("branch misprediction", anomalies[0]["hint"])

    def test_high_l1_icache_miss_triggers(self):
        """Test high L1 icache miss rate anomaly detection."""
        metrics = {
            "ipc": 2.0,
            "branch_miss_rate": 5.0,
            "l1_icache_miss_rate": 8.0,  # Above threshold of 5%
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 1.0,
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 1)
        self.assertEqual(anomalies[0]["anomaly"], "high_l1_icache_miss")
        self.assertEqual(anomalies[0]["measured_value"], 8.0)
        self.assertEqual(anomalies[0]["threshold"], 5.0)
        self.assertEqual(anomalies[0]["direction"], "above")
        self.assertIn("instruction cache", anomalies[0]["hint"])

    def test_high_l1_dcache_miss_triggers(self):
        """Test high L1 dcache miss rate anomaly detection."""
        metrics = {
            "ipc": 2.0,
            "branch_miss_rate": 5.0,
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 12.0,  # Above threshold of 10%
            "dtlb_miss_rate": 1.0,
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 1)
        self.assertEqual(anomalies[0]["anomaly"], "high_l1_dcache_miss")
        self.assertEqual(anomalies[0]["measured_value"], 12.0)
        self.assertEqual(anomalies[0]["threshold"], 10.0)
        self.assertEqual(anomalies[0]["direction"], "above")
        self.assertIn("data cache", anomalies[0]["hint"])

    def test_high_dtlb_miss_triggers(self):
        """Test high dTLB miss rate anomaly detection."""
        metrics = {
            "ipc": 2.0,
            "branch_miss_rate": 5.0,
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 5.0,  # Above threshold of 3%
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 1)
        self.assertEqual(anomalies[0]["anomaly"], "high_dtlb_miss")
        self.assertEqual(anomalies[0]["measured_value"], 5.0)
        self.assertEqual(anomalies[0]["threshold"], 3.0)
        self.assertEqual(anomalies[0]["direction"], "above")
        self.assertIn("dTLB", anomalies[0]["hint"])

    def test_no_anomalies(self):
        """Test no anomalies when all metrics within bounds."""
        metrics = {
            "ipc": 2.0,  # Above 1.0
            "branch_miss_rate": 5.0,  # Below 10%
            "l1_icache_miss_rate": 2.0,  # Below 5%
            "l1_dcache_miss_rate": 5.0,  # Below 10%
            "dtlb_miss_rate": 1.0,  # Below 3%
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 0)

    def test_multiple_anomalies(self):
        """Test multiple anomalies detected simultaneously."""
        metrics = {
            "ipc": 0.8,  # Below 1.0
            "branch_miss_rate": 12.0,  # Above 10%
            "l1_icache_miss_rate": 6.0,  # Above 5%
            "l1_dcache_miss_rate": 15.0,  # Above 10%
            "dtlb_miss_rate": 4.0,  # Above 3%
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 5)
        anomaly_names = {a["anomaly"] for a in anomalies}
        expected = {
            "low_ipc",
            "high_branch_miss",
            "high_l1_icache_miss",
            "high_l1_dcache_miss",
            "high_dtlb_miss",
        }
        self.assertEqual(anomaly_names, expected)

    def test_none_values_skipped(self):
        """Test that None metric values are skipped in anomaly detection."""
        metrics = {
            "ipc": None,  # Would trigger if not None
            "branch_miss_rate": None,
            "l1_icache_miss_rate": 8.0,  # Should trigger
            "l1_dcache_miss_rate": None,
            "dtlb_miss_rate": None,
        }

        anomalies = detect_anomalies(metrics)

        self.assertEqual(len(anomalies), 1)
        self.assertEqual(anomalies[0]["anomaly"], "high_l1_icache_miss")

    def test_boundary_values(self):
        """Test boundary values at thresholds."""
        # Exactly at threshold should not trigger
        metrics = {
            "ipc": 1.0,  # Exactly at threshold
            "branch_miss_rate": 10.0,  # Exactly at threshold
            "l1_icache_miss_rate": 5.0,  # Exactly at threshold
            "l1_dcache_miss_rate": 10.0,  # Exactly at threshold
            "dtlb_miss_rate": 3.0,  # Exactly at threshold
        }

        anomalies = detect_anomalies(metrics)
        self.assertEqual(len(anomalies), 0)

        # Just below/above threshold should trigger
        metrics = {
            "ipc": 0.9999,  # Just below 1.0
            "branch_miss_rate": 10.0001,  # Just above 10%
            "l1_icache_miss_rate": 5.0001,  # Just above 5%
            "l1_dcache_miss_rate": 10.0001,  # Just above 10%
            "dtlb_miss_rate": 3.0001,  # Just above 3%
        }

        anomalies = detect_anomalies(metrics)
        self.assertEqual(len(anomalies), 5)


class TestFormatHuman(unittest.TestCase):
    """Tests for format_human function."""

    def test_no_anomalies(self):
        """Test formatting with no anomalies."""
        metrics = {
            "ipc": 2.0,
            "branch_miss_rate": 5.0,
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 1.0,
        }
        anomalies = []

        output = format_human(anomalies, metrics)

        self.assertIn("=== PMU Metrics ===", output)
        self.assertIn("ipc: 2.0000", output)
        self.assertIn("branch_miss_rate: 5.0000", output)
        self.assertIn("No anomalies detected", output)
        self.assertIn("All metrics within thresholds", output)

    def test_with_anomalies(self):
        """Test formatting with anomalies contains expected elements."""
        metrics = {
            "ipc": 0.5,
            "branch_miss_rate": 15.0,
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 1.0,
        }
        anomalies = detect_anomalies(metrics)

        output = format_human(anomalies, metrics)

        self.assertIn("=== PMU Metrics ===", output)
        self.assertIn("ipc: 0.5000", output)
        self.assertIn("branch_miss_rate: 15.0000", output)
        self.assertIn("=== Anomalies Detected: 2 ===", output)
        self.assertIn("[ANOMALY]", output)
        self.assertIn("IPC (Instructions Per Cycle)", output)
        self.assertIn("Branch Miss Rate", output)
        self.assertIn("Measured: 0.5000", output)
        self.assertIn("Measured: 15.0000", output)
        self.assertIn("Hint:", output)
        self.assertIn("pipeline stalls", output)
        self.assertIn("branch misprediction", output)

    def test_none_values_shown_as_na(self):
        """Test that None metric values are shown as N/A."""
        metrics = {
            "ipc": 2.0,
            "branch_miss_rate": None,
            "l1_icache_miss_rate": None,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": None,
        }
        anomalies = []

        output = format_human(anomalies, metrics)

        self.assertIn("ipc: 2.0000", output)
        self.assertIn("branch_miss_rate: N/A (insufficient data)", output)
        self.assertIn("l1_icache_miss_rate: N/A (insufficient data)", output)
        self.assertIn("l1_dcache_miss_rate: 5.0000", output)
        self.assertIn("dtlb_miss_rate: N/A (insufficient data)", output)

    def test_direction_words(self):
        """Test that direction words (below/above) are correctly displayed."""
        metrics = {
            "ipc": 0.5,  # below threshold
            "branch_miss_rate": 15.0,  # above threshold
            "l1_icache_miss_rate": 2.0,
            "l1_dcache_miss_rate": 5.0,
            "dtlb_miss_rate": 1.0,
        }
        anomalies = detect_anomalies(metrics)

        output = format_human(anomalies, metrics)

        # IPC uses "below"
        self.assertIn("threshold: below 1", output)
        # Branch miss uses "above"
        self.assertIn("threshold: above 10", output)


if __name__ == "__main__":
    unittest.main()
