"""Remaining M3 white-box checks that parse real CMake and CI YAML."""

import os
import sys
import unittest

_TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
if _TESTS_DIR not in sys.path:
    sys.path.insert(0, _TESTS_DIR)

from extract import extract_cmake, extract_workflow_step


class TestCMakeAndPackagingInvariants(unittest.TestCase):
    def test_cmakelists_contains_decoupling_and_install_rules(self):
        cmake_text = extract_cmake()
        self.assertIn("option(OPENTRACK_HAS_USBMUXD", cmake_text)
        self.assertIn("OPENTRACK_SMOOTHTRACK_HAVE_USBMUXD", cmake_text)
        self.assertIn("st-relay-arm64", cmake_text)
        self.assertIn("st-relay-armv7", cmake_text)
        self.assertIn("${opentrack-libexec}/android", cmake_text)

    def test_workflow_has_defensive_path_equality_guards_for_all_components(self):
        package = extract_workflow_step("Package install tree")
        guard_snippet = "[System.IO.Path]::GetFullPath($src) -ne [System.IO.Path]::GetFullPath($destFile)"
        occurrences = package.count(guard_snippet)
        self.assertGreaterEqual(
            occurrences,
            2,
            f"Expected at least 2 defensive path equality guards in packaging step, found {occurrences}",
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
