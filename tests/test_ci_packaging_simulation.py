"""
Tests for CI packaging workflow from .github/workflows/windows-11.yml
Empirical challenger validation for Milestone 2.
"""

import os
import re
import sys
import tempfile
import zipfile
import subprocess
import unittest

import yaml

_TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
if _TESTS_DIR not in sys.path:
    sys.path.insert(0, _TESTS_DIR)

from extract import REPO_ROOT, WORKFLOW_FILE, extract_workflow_step


def write_installed_relays(install_dir, arm64=b"ARM64" * 10, armv7=b"ARMV7" * 10):
    dest = os.path.join(install_dir, "modules", "android")
    os.makedirs(dest, exist_ok=True)
    if arm64 is not None:
        with open(os.path.join(dest, "st-relay-arm64"), "wb") as f:
            f.write(arm64)
    if armv7 is not None:
        with open(os.path.join(dest, "st-relay-armv7"), "wb") as f:
            f.write(armv7)
    return dest


class TestCIPackagingWorkflow(unittest.TestCase):
    """Empirically validates the Windows 11 packaging workflow logic."""

    @classmethod
    def setUpClass(cls):
        cls.package_script = extract_workflow_step("Package install tree")
        cls.compile_script = extract_workflow_step("Export Android NDK")
        cls.powershell_exe = "powershell.exe"

    def run_ps_script(self, script_text: str, env_vars: dict) -> subprocess.CompletedProcess:
        """Runs a powershell script with custom environment variables in a clean process."""
        temp_script_path = None
        try:
            with tempfile.NamedTemporaryFile("w", suffix=".ps1", delete=False, encoding="utf-8") as tf:
                tf.write(script_text)
                temp_script_path = tf.name

            env = os.environ.copy()
            env.update(env_vars)

            cmd = [
                self.powershell_exe,
                "-NoProfile",
                "-NonInteractive",
                "-ExecutionPolicy", "Bypass",
                "-File", temp_script_path
            ]
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                env=env,
                cwd=env_vars.get("GITHUB_WORKSPACE", REPO_ROOT)
            )
            return result
        finally:
            if temp_script_path and os.path.exists(temp_script_path):
                try:
                    os.unlink(temp_script_path)
                except Exception:
                    pass

    def test_01_successful_packaging_simulation(self):
        """Simulate complete build install tree and verify all root and subdir artifacts and zip layout."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(os.path.join(install_dir, "modules"), exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            # Create mock opentrack.exe and module DLL
            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"MOCK_OPENTRACK_EXE_BINARY_CONTENT" * 10)
            with open(os.path.join(install_dir, "modules", "opentrack-tracker-smoothtrack.dll"), "wb") as f:
                f.write(b"MOCK_SMOOTHTRACK_MODULE_DLL" * 10)

            write_installed_relays(
                install_dir,
                arm64=b"MOCK_ARM64_RELAY_BINARY_ELF" * 20,
                armv7=b"MOCK_ARMV7_RELAY_BINARY_ELF" * 20,
            )

            # Create mock adb binaries in platform-tools
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"MOCK_ADB_EXE" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"MOCK_ADB_WIN_API_DLL" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"MOCK_ADB_WIN_USB_API_DLL" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(
                result.returncode, 0,
                f"Packaging script failed unexpectedly:\nSTDOUT:\n{result.stdout}\nSTDERR:\n{result.stderr}"
            )

            # 1. Verify files exist in root ($install / opentrack-bin)
            root_expected = [
                "opentrack.exe",
                "adb.exe",
                "AdbWinApi.dll",
                "AdbWinUsbApi.dll",
            ]
            for fname in root_expected:
                p = os.path.join(install_dir, fname)
                self.assertTrue(os.path.exists(p), f"Missing root artifact: {fname}")
                self.assertGreater(os.path.getsize(p), 0, f"0-byte root artifact: {fname}")

            # Relays and extra adb copies must not land at install root / modules / platform-tools.
            for fname in ["st-relay-arm64", "st-relay-armv7"]:
                self.assertFalse(
                    os.path.exists(os.path.join(install_dir, fname)),
                    f"Relay {fname} must not be copied to install root",
                )
            for fname in ["adb.exe", "AdbWinApi.dll", "AdbWinUsbApi.dll"]:
                self.assertFalse(
                    os.path.exists(os.path.join(install_dir, "modules", fname)),
                    f"ADB {fname} must not be copied to modules/",
                )
                self.assertFalse(
                    os.path.exists(os.path.join(install_dir, "platform-tools", fname)),
                    f"ADB {fname} must not be copied to platform-tools/",
                )

            self.assertTrue(
                os.path.exists(os.path.join(install_dir, "modules", "opentrack-tracker-smoothtrack.dll")),
                "Missing modules/opentrack-tracker-smoothtrack.dll",
            )

            # Canonical relay location: modules/android/
            for fname in ["st-relay-arm64", "st-relay-armv7"]:
                p = os.path.join(install_dir, "modules", "android", fname)
                self.assertTrue(os.path.exists(p), f"Missing modules/android/ artifact: {fname}")
                self.assertFalse(
                    os.path.exists(os.path.join(install_dir, "android", fname)),
                    f"Relay {fname} must not be copied to install/android/",
                )

            # 3. Verify release zip file exists and is non-empty
            zip_path = os.path.join(ws, "opentrack-windows11-x64.zip")
            self.assertTrue(os.path.exists(zip_path), "Release zip file does not exist")
            self.assertGreater(os.path.getsize(zip_path), 0, "Release zip file is 0-byte")

            # 4. Verify ZIP internal layout
            with zipfile.ZipFile(zip_path, "r") as zf:
                namelist = [n.replace("\\", "/") for n in zf.namelist()]

                # Root level files must NOT have directory prefixes
                for fname in root_expected:
                    self.assertIn(fname, namelist, f"ZIP root missing '{fname}' (actual: {namelist[:10]})")

                self.assertNotIn("st-relay-arm64", namelist)
                self.assertNotIn("modules/adb.exe", namelist)
                self.assertNotIn("platform-tools/adb.exe", namelist)
                self.assertNotIn("android/st-relay-arm64", namelist)
                self.assertIn("modules/android/st-relay-arm64", namelist)
                self.assertIn("modules/android/st-relay-armv7", namelist)
                self.assertIn("modules/opentrack-tracker-smoothtrack.dll", namelist)

    def test_02_missing_opentrack_exe_fails(self):
        """Verify that missing opentrack.exe in install tree triggers a terminating failure."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            # Do NOT create opentrack.exe
            write_installed_relays(install_dir)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when opentrack.exe is missing")
            combined_out = result.stdout + result.stderr
            self.assertTrue(
                "Missing required root artifact" in combined_out or "Pre-packaging verification failed" in combined_out,
                f"Expected error message not found in output:\n{combined_out}"
            )

    def test_03_zero_byte_opentrack_exe_fails(self):
        """Verify that a 0-byte opentrack.exe triggers a terminating failure."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            # Create 0-byte opentrack.exe
            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                pass  # 0 bytes
            write_installed_relays(install_dir)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when opentrack.exe is 0 bytes")
            combined_out = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined_out or "Pre-packaging verification failed" in combined_out,
                f"Expected 0-byte error message not found in output:\n{combined_out}"
            )

    def test_04_missing_relay_binary_arm64_fails(self):
        """Verify that missing st-relay-arm64 triggers a terminating failure."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            write_installed_relays(install_dir, arm64=None, armv7=b"ARMV7" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when st-relay-arm64 is missing")
            combined_out = result.stdout + result.stderr
            self.assertTrue(
                "Required relay binary missing" in combined_out or "st-relay-arm64" in combined_out,
                f"Expected missing relay binary error not found in output:\n{combined_out}"
            )

    def test_05_missing_relay_binary_armv7_fails(self):
        """Verify that missing st-relay-armv7 triggers a terminating failure."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            write_installed_relays(install_dir, arm64=b"ARM64" * 10, armv7=None)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when st-relay-armv7 is missing")
            combined_out = result.stdout + result.stderr
            self.assertTrue(
                "Required relay binary missing" in combined_out or "st-relay-armv7" in combined_out,
                f"Expected missing relay binary error not found in output:\n{combined_out}"
            )

    def test_06_zero_byte_relay_binary_fails(self):
        """Verify that 0-byte relay binary fails pre-packaging verification."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            write_installed_relays(install_dir, arm64=b"", armv7=b"ARMV7" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when relay binary is 0-byte")
            combined_out = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined_out or "Pre-packaging verification failed" in combined_out,
                f"Expected 0-byte error not found in output:\n{combined_out}"
            )

    def test_07_missing_install_tree_fails(self):
        """Verify that a completely missing install tree throws terminating error."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            # No build/install created
            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when install tree does not exist")
            combined_out = result.stdout + result.stderr
            self.assertTrue(
                "Install tree not found" in combined_out,
                f"Expected 'Install tree not found' in output:\n{combined_out}"
            )

    def test_08_relay_fallback_from_cmake_build_tree(self):
        """If CMake install missed modules/android, stage relays from the build tree only."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)
            build_relay_dir = os.path.join(ws, "build", "tracker-smoothtrack", "android")
            os.makedirs(build_relay_dir, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            with open(os.path.join(build_relay_dir, "st-relay-arm64"), "wb") as f:
                f.write(b"ARM64_IN_BUILD" * 10)
            with open(os.path.join(build_relay_dir, "st-relay-armv7"), "wb") as f:
                f.write(b"ARMV7_IN_BUILD" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }

            result = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(
                result.returncode, 0,
                f"Packaging should stage relays from the CMake build tree:\nSTDOUT:\n{result.stdout}\nSTDERR:\n{result.stderr}",
            )
            self.assertTrue(os.path.exists(os.path.join(install_dir, "modules", "android", "st-relay-arm64")))
            self.assertFalse(os.path.exists(os.path.join(ws, "tracker-smoothtrack", "android", "st-relay-arm64")))

    def test_09_compile_relay_script_error_handling_when_ndk_missing(self):
        """Compile step fails clearly if setup-ndk did not provide ANDROID_NDK_ROOT."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_NDK_ROOT": "",
            }
            result = self.run_ps_script(self.compile_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Compile step should fail if NDK is missing")
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Android NDK setup failed" in combined,
                f"Expected NDK setup failure error:\n{combined}"
            )

    def test_10_compile_relay_script_happy_path_simulation(self):
        """Pinned NDK path is exported as ANDROID_NDK_ROOT and source-tree relays are not written."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            relay_dir = os.path.join(ws, "tracker-smoothtrack", "android")
            os.makedirs(relay_dir, exist_ok=True)
            with open(os.path.join(relay_dir, "relay.c"), "w", encoding="utf-8") as f:
                f.write("int main() { return 0; }\n")

            mock_ndk = os.path.join(test_dir, "mock_ndk")
            os.makedirs(mock_ndk, exist_ok=True)

            github_env = os.path.join(test_dir, "github_env.txt")
            with open(github_env, "w", encoding="utf-8") as f:
                pass

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_NDK_ROOT": mock_ndk,
                "GITHUB_ENV": github_env,
            }
            result = self.run_ps_script(self.compile_script, env_vars)
            self.assertEqual(result.returncode, 0, f"NDK export script failed:\n{result.stdout}\n{result.stderr}")
            self.assertFalse(os.path.exists(os.path.join(relay_dir, "st-relay-arm64")))
            self.assertFalse(os.path.exists(os.path.join(relay_dir, "st-relay-armv7")))
            with open(github_env, encoding="utf-8") as f:
                exported = f.read()
            self.assertIn("ANDROID_NDK_ROOT=", exported)
            self.assertIn(os.path.normcase(mock_ndk), os.path.normcase(exported))

    def test_11_compile_relay_script_fails_when_ndk_path_missing(self):
        """Compile step fails clearly if ANDROID_NDK_ROOT does not exist (setup-ndk failed)."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            missing = os.path.join(test_dir, "does-not-exist-ndk")

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_NDK_ROOT": missing,
            }
            result = self.run_ps_script(self.compile_script, env_vars)
            self.assertNotEqual(result.returncode, 0, "Script should fail when NDK path is missing")
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Android NDK setup failed" in combined,
                f"Expected NDK setup failure in output:\n{combined}"
            )

    def test_13_workflow_pins_ndk_and_platform_tools(self):
        """Windows CI must pin NDK r27c and a versioned platform-tools zip with SHA256."""
        with open(WORKFLOW_FILE, encoding="utf-8") as f:
            text = f.read()
        self.assertNotIn("platform-tools-latest-windows.zip", text)
        self.assertNotIn("platform-tools_r36.0.0-windows.zip", text)
        self.assertIn("ndk-version: r27c", text)
        pinned_url = "https://dl.google.com/android/repository/platform-tools_r36.0.0-win.zip"
        pinned_sha = "12C2841F354E92A0EB2FD7BF6F0F9BF8538ABCE7BD6B060AC8349D6F6A61107C"
        self.assertIn(pinned_url, text)
        self.assertIn(pinned_sha, text)
        self.assertLess(abs(text.index(pinned_sha) - text.index(pinned_url)), 500)
        workflow = yaml.safe_load(text)
        steps = workflow["jobs"]["windows-11-x64"]["steps"]
        setup_ndk = next(s for s in steps if s.get("uses") == "nttld/setup-ndk@v1")
        self.assertEqual(setup_ndk["with"]["add-to-path"], False)
        export_step = next(s for s in steps if s.get("name") == "Export Android NDK")
        self.assertEqual(
            export_step["env"]["ANDROID_NDK_ROOT"],
            "${{ steps.setup-ndk.outputs.ndk-path }}",
        )
        dests = re.search(r"\$adbDests\s*=\s*@\((.*?)\)", text, re.S)
        self.assertIsNotNone(dests, "$adbDests array is required")
        adb_dests = [re.sub(r"#.*", "", raw).strip() for raw in dests.group(1).split(",") if re.sub(r"#.*", "", raw).strip()]
        self.assertEqual(adb_dests, ["$install"], f"adb Copy-Item destination count must be 1, got {adb_dests}")


    def test_12_adb_self_overwrite_when_platform_tools_in_install_dest(self):
        """Regression: same-path Copy-Item of adb from install/platform-tools must succeed."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            install_dir = os.path.join(ws, "build", "install")
            pt_dir = os.path.join(install_dir, "platform-tools")
            os.makedirs(pt_dir, exist_ok=True)
            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            write_installed_relays(install_dir)
            with open(os.path.join(pt_dir, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(pt_dir, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(pt_dir, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            # Point ANDROID_HOME such that platform-tools resolves to pt_dir
            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": install_dir,
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            if result.returncode != 0 and "Cannot overwrite the item" in result.stderr:
                self.fail(f"BUG CONFIRMED in windows-11.yml line 252: {result.stderr.strip()}")
            self.assertEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
