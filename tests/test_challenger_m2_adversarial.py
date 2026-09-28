"""
Empirical Challenger 2 Adversarial Stress Suite for Milestone 2 Iteration 2.
Directly tests:
1. Pre-compilation clean (stale $out64 and $out32 purged prior to compilation, including read-only files and failed compiler scenarios).
2. Pre-packaging assertions (missing opentrack.exe, missing relay binary, 0-byte binaries, missing ADB libraries).
"""

import os
import sys
import shutil
import tempfile
import stat
import subprocess
import unittest

_TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
if _TESTS_DIR not in sys.path:
    sys.path.insert(0, _TESTS_DIR)

from extract import REPO_ROOT, extract_workflow_step


class TestM2ChallengerAdversarial(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.package_script = extract_workflow_step("Package install tree")
        cls.compile_script = extract_workflow_step("Export Android NDK")
        cls.powershell_exe = "powershell.exe"

    def run_ps_script(self, script_text: str, env_vars: dict) -> subprocess.CompletedProcess:
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

    # =========================================================================
    # 1. PRE-COMPILATION CLEANUP TESTS
    # =========================================================================

    def test_ndk_locate_does_not_write_source_tree_relays(self):
        """NDK locate must export ANDROID_NDK_ROOT and leave source-tree st-relay files untouched."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            relay_dir = os.path.join(ws, "tracker-smoothtrack", "android")
            os.makedirs(relay_dir, exist_ok=True)
            with open(os.path.join(relay_dir, "relay.c"), "w", encoding="utf-8") as f:
                f.write("int main() { return 0; }\n")

            out64 = os.path.join(relay_dir, "st-relay-arm64")
            out32 = os.path.join(relay_dir, "st-relay-armv7")
            with open(out64, "wb") as f:
                f.write(b"STALE_OLD_ARM64_BINARY")
            with open(out32, "wb") as f:
                f.write(b"STALE_OLD_ARMV7_BINARY")

            mock_ndk = os.path.join(test_dir, "mock_ndk")
            llvm_bin = os.path.join(mock_ndk, "toolchains", "llvm", "prebuilt", "windows-x86_64", "bin")
            os.makedirs(llvm_bin, exist_ok=True)

            failing_bat = "@echo off\r\necho should not be invoked 1>&2\r\nexit /b 1\r\n"
            with open(os.path.join(llvm_bin, "aarch64-linux-android24-clang.cmd"), "w") as f:
                f.write(failing_bat)
            with open(os.path.join(llvm_bin, "armv7a-linux-androideabi24-clang.cmd"), "w") as f:
                f.write(failing_bat)

            github_env = os.path.join(test_dir, "github_env.txt")
            with open(github_env, "w", encoding="utf-8") as f:
                pass

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_NDK_ROOT": mock_ndk,
                "GITHUB_ENV": github_env,
            }
            result = self.run_ps_script(self.compile_script, env_vars)
            self.assertEqual(result.returncode, 0, f"NDK locate failed:\n{result.stdout}\n{result.stderr}")
            with open(out64, "rb") as f:
                self.assertEqual(f.read(), b"STALE_OLD_ARM64_BINARY")
            with open(out32, "rb") as f:
                self.assertEqual(f.read(), b"STALE_OLD_ARMV7_BINARY")
            with open(github_env, encoding="utf-8") as f:
                exported = f.read()
            self.assertIn("ANDROID_NDK_ROOT=", exported)

    def test_ndk_locate_does_not_overwrite_readonly_source_tree_relays(self):
        """Read-only source-tree st-relay files must not be rewritten by the NDK locate step."""
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            relay_dir = os.path.join(ws, "tracker-smoothtrack", "android")
            os.makedirs(relay_dir, exist_ok=True)
            with open(os.path.join(relay_dir, "relay.c"), "w", encoding="utf-8") as f:
                f.write("int main() { return 0; }\n")

            out64 = os.path.join(relay_dir, "st-relay-arm64")
            out32 = os.path.join(relay_dir, "st-relay-armv7")
            with open(out64, "wb") as f:
                f.write(b"STALE_READONLY_ARM64")
            with open(out32, "wb") as f:
                f.write(b"STALE_READONLY_ARMV7")

            os.chmod(out64, stat.S_IREAD)
            os.chmod(out32, stat.S_IREAD)

            mock_ndk = os.path.join(test_dir, "mock_ndk")
            llvm_bin = os.path.join(mock_ndk, "toolchains", "llvm", "prebuilt", "windows-x86_64", "bin")
            os.makedirs(llvm_bin, exist_ok=True)

            mock_clang_bat = "@echo off\r\necho FRESH_NEW_BINARY\r\nexit /b 0\r\n"
            with open(os.path.join(llvm_bin, "aarch64-linux-android24-clang.cmd"), "w") as f:
                f.write(mock_clang_bat)
            with open(os.path.join(llvm_bin, "armv7a-linux-androideabi24-clang.cmd"), "w") as f:
                f.write(mock_clang_bat)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_NDK_ROOT": mock_ndk
            }
            result = self.run_ps_script(self.compile_script, env_vars)
            self.assertEqual(result.returncode, 0, f"NDK locate failed:\n{result.stdout}\n{result.stderr}")

            os.chmod(out64, stat.S_IWRITE | stat.S_IREAD)
            os.chmod(out32, stat.S_IWRITE | stat.S_IREAD)
            with open(out64, "rb") as f:
                self.assertEqual(f.read(), b"STALE_READONLY_ARM64")
            with open(out32, "rb") as f:
                self.assertEqual(f.read(), b"STALE_READONLY_ARMV7")

    # =========================================================================
    # 2. PRE-PACKAGING ASSERTION TESTS
    # =========================================================================

    def _setup_valid_staging_tree(self, ws: str, test_dir: str):
        install_dir = os.path.join(ws, "build", "install")
        os.makedirs(os.path.join(install_dir, "modules"), exist_ok=True)
        relay_dest = os.path.join(install_dir, "modules", "android")
        os.makedirs(relay_dest, exist_ok=True)
        mock_sdk = os.path.join(test_dir, "mock_sdk", "platform-tools")
        os.makedirs(mock_sdk, exist_ok=True)

        with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
            f.write(b"OPENTRACK_EXE_VALID" * 10)
        with open(os.path.join(install_dir, "modules", "opentrack-tracker-smoothtrack.dll"), "wb") as f:
            f.write(b"SMOOTHTRACK_DLL" * 10)
        with open(os.path.join(relay_dest, "st-relay-arm64"), "wb") as f:
            f.write(b"ARM64_VALID" * 10)
        with open(os.path.join(relay_dest, "st-relay-armv7"), "wb") as f:
            f.write(b"ARMV7_VALID" * 10)
        with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
            f.write(b"ADB_VALID" * 10)
        with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
            f.write(b"ADB_WIN_API_VALID" * 10)
        with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
            f.write(b"ADB_WIN_USB_API_VALID" * 10)

        return install_dir, relay_dest, mock_sdk

    def test_prepackaging_assertion_fails_when_opentrack_exe_missing(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, _, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            os.unlink(os.path.join(install_dir, "opentrack.exe"))

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Missing required root artifact" in combined or "Pre-packaging verification failed" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_opentrack_exe_is_zero_byte(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, _, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                pass  # truncate to 0 bytes

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined or "Pre-packaging verification failed" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_st_relay_arm64_missing(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, android_src_dir, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            os.unlink(os.path.join(android_src_dir, "st-relay-arm64"))

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required relay binary missing from build output" in combined or "Missing required root artifact" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_st_relay_armv7_missing(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, android_src_dir, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            os.unlink(os.path.join(android_src_dir, "st-relay-armv7"))

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required relay binary missing from build output" in combined or "Missing required root artifact" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_st_relay_arm64_is_zero_byte(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, android_src_dir, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            with open(os.path.join(android_src_dir, "st-relay-arm64"), "wb") as f:
                pass  # 0 bytes

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined or "Pre-packaging verification failed" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_st_relay_armv7_is_zero_byte(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, android_src_dir, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            with open(os.path.join(android_src_dir, "st-relay-armv7"), "wb") as f:
                pass  # 0 bytes

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined or "Pre-packaging verification failed" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_adb_exe_is_zero_byte(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, android_src_dir, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                pass  # 0 bytes

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined or "Pre-packaging verification failed" in combined,
                f"Expected error message not found: {combined}"
            )

    def test_prepackaging_assertion_fails_when_adb_dll_is_zero_byte(self):
        with tempfile.TemporaryDirectory() as test_dir:
            ws = os.path.join(test_dir, "workspace")
            os.makedirs(ws)
            install_dir, android_src_dir, mock_sdk = self._setup_valid_staging_tree(ws, test_dir)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                pass  # 0 bytes

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(test_dir, "mock_sdk"),
                "RUNNER_TEMP": test_dir
            }
            result = self.run_ps_script(self.package_script, env_vars)
            self.assertNotEqual(result.returncode, 0)
            combined = result.stdout + result.stderr
            self.assertTrue(
                "Required artifact is 0-byte" in combined or "Pre-packaging verification failed" in combined,
                f"Expected error message not found: {combined}"
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)
