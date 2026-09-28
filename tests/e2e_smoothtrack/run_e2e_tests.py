#!/usr/bin/env python3
"""Standalone runner for remaining SmoothTrack mock-adb tests.

Duplicate tier files were removed; the CLI contract lives in tests/test_mock_adb.py.
"""

import os
import sys
import time
import unittest

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TESTS_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
for path in (PROJECT_ROOT, TESTS_DIR):
    if path not in sys.path:
        sys.path.insert(0, path)


def build_suite() -> unittest.TestSuite:
    return unittest.defaultTestLoader.loadTestsFromName("test_mock_adb")


def main():
    print("=" * 78)
    print("       SMOOTHTRACK DUAL-PLATFORM USB TRACKER E2E TEST SUITE")
    print("=" * 78)
    print(f"Python: {sys.version.split()[0]} ({sys.platform})")
    print(f"Working Directory: {os.getcwd()}")
    print("-" * 78)

    t0 = time.time()
    suite = build_suite()
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    duration = time.time() - t0

    status = "PASS" if result.wasSuccessful() else "FAIL"
    print("\n" + "=" * 78)
    print(f"  E2E result               : {status}")
    print(f"  Total Test Cases Executed : {result.testsRun}")
    print(f"  Failures                  : {len(result.failures)}")
    print(f"  Errors                    : {len(result.errors)}")
    print(f"  Skipped                   : {len(result.skipped)}")
    print(f"  Total Duration            : {duration:.3f} seconds")
    print("=" * 78)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
