"""
Stress test harness for path equality logic and defensive guards
in .github/workflows/windows-11.yml.
Empirical challenger validation for Milestone 2 Iteration 2.
"""

import os
import sys
import shutil
import tempfile
import zipfile
import subprocess
import unittest

_TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
if _TESTS_DIR not in sys.path:
    sys.path.insert(0, _TESTS_DIR)

from extract import REPO_ROOT, extract_workflow_step


class TestPathEqualityStress(unittest.TestCase):
    """Stress tests path equality, edge cases, and defensive guards."""

    @classmethod
    def setUpClass(cls):
        cls.package_script = extract_workflow_step("Package install tree")
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

    def run_ps_command(self, cmd_text: str) -> subprocess.CompletedProcess:
        cmd = [
            self.powershell_exe,
            "-NoProfile",
            "-NonInteractive",
            "-ExecutionPolicy", "Bypass",
            "-Command", cmd_text
        ]
        return subprocess.run(cmd, capture_output=True, text=True)

    # -------------------------------------------------------------------------
    # Category 1: Direct .NET Path & PowerShell operator edge-case tests
    # -------------------------------------------------------------------------
    def test_dotnet_getfullpath_bracket_characters(self):
        """Verify [System.IO.Path]::GetFullPath handles bracket characters cleanly."""
        script = r"""
        $p1 = "C:\workspace[test]\build\opentrack.exe"
        $p2 = "c:/workspace[test]/build/opentrack.exe"
        $f1 = [System.IO.Path]::GetFullPath($p1)
        $f2 = [System.IO.Path]::GetFullPath($p2)
        if ($f1 -ne $f2) {
            Write-Error "Mismatch: $f1 vs $f2"
            exit 1
        }
        Write-Output "MATCH: $f1"
        """
        res = self.run_ps_command(script)
        self.assertEqual(res.returncode, 0, f"Failed: {res.stderr}")
        self.assertIn("MATCH", res.stdout)

    def test_dotnet_getfullpath_mixed_slashes_and_relative_dots(self):
        """Verify [System.IO.Path]::GetFullPath normalizes forward/backward slashes and dot-segments."""
        script = r"""
        $p1 = "C:\opentrack\build\install\..\install/adb.exe"
        $p2 = "c:/opentrack/build/./install\adb.exe"
        $f1 = [System.IO.Path]::GetFullPath($p1)
        $f2 = [System.IO.Path]::GetFullPath($p2)
        if ($f1 -ne $f2) {
            Write-Error "Mismatch: $f1 vs $f2"
            exit 1
        }
        Write-Output "MATCH: $f1"
        """
        res = self.run_ps_command(script)
        self.assertEqual(res.returncode, 0, f"Failed: {res.stderr}")
        self.assertIn("MATCH", res.stdout)

    def test_dotnet_getfullpath_case_insensitivity_comparison(self):
        """Verify PowerShell -ne and -eq operators perform case-insensitive comparison on GetFullPath."""
        script = r"""
        $p1 = "C:\OPENTRACK\BUILD\INSTALL\ADB.EXE"
        $p2 = "c:\opentrack\build\install\adb.exe"
        $f1 = [System.IO.Path]::GetFullPath($p1)
        $f2 = [System.IO.Path]::GetFullPath($p2)
        if ($f1 -ne $f2) {
            Write-Error "Should be equal under PowerShell case-insensitive comparison"
            exit 1
        }
        Write-Output "EQUAL"
        """
        res = self.run_ps_command(script)
        self.assertEqual(res.returncode, 0, f"Failed: {res.stderr}")
        self.assertIn("EQUAL", res.stdout)

    def test_dotnet_getfullpath_redundant_separators(self):
        """Verify redundant slashes are collapsed to canonical form."""
        script = r"""
        $p1 = "C:\\\opentrack\\build\\\\install\\st-relay-arm64"
        $p2 = "C:/opentrack//build/install/st-relay-arm64"
        $f1 = [System.IO.Path]::GetFullPath($p1)
        $f2 = [System.IO.Path]::GetFullPath($p2)
        if ($f1 -ne $f2) {
            Write-Error "Mismatch: $f1 vs $f2"
            exit 1
        }
        Write-Output "COLLAPSED_MATCH: $f1"
        """
        res = self.run_ps_command(script)
        self.assertEqual(res.returncode, 0, f"Failed: {res.stderr}")
        self.assertIn("COLLAPSED_MATCH", res.stdout)

    # -------------------------------------------------------------------------
    # Category 2: Full packaging workflow simulation under standard and stress conditions
    # -------------------------------------------------------------------------
    def test_workflow_packaging_with_spaces_in_path(self):
        """Execute packaging workflow in a workspace path with spaces."""
        with tempfile.TemporaryDirectory() as base_tmp:
            ws = os.path.join(base_tmp, "workspace with spaces build 2026")
            os.makedirs(ws, exist_ok=True)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(os.path.join(install_dir, "modules"), exist_ok=True)
            relay_dest = os.path.join(install_dir, "modules", "android")
            os.makedirs(relay_dest, exist_ok=True)
            mock_sdk = os.path.join(base_tmp, "mock sdk with spaces", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK_EXE" * 10)
            with open(os.path.join(install_dir, "modules", "opentrack-tracker-smoothtrack.dll"), "wb") as f:
                f.write(b"SMOOTHTRACK_DLL" * 10)
            with open(os.path.join(relay_dest, "st-relay-arm64"), "wb") as f:
                f.write(b"ARM64" * 10)
            with open(os.path.join(relay_dest, "st-relay-armv7"), "wb") as f:
                f.write(b"ARMV7" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(base_tmp, "mock sdk with spaces"),
                "RUNNER_TEMP": base_tmp
            }

            res = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(res.returncode, 0, f"Failed on spaces in path:\nSTDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}")

            zip_path = os.path.join(ws, "opentrack-windows11-x64.zip")
            self.assertTrue(os.path.exists(zip_path))
            self.assertGreater(os.path.getsize(zip_path), 0)

    def test_workflow_packaging_with_trailing_slashes_in_env(self):
        """Execute packaging workflow when environment variables contain trailing slashes."""
        with tempfile.TemporaryDirectory() as base_tmp:
            ws = os.path.join(base_tmp, "workspace")
            os.makedirs(ws, exist_ok=True)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(os.path.join(install_dir, "modules"), exist_ok=True)
            relay_dest = os.path.join(install_dir, "modules", "android")
            os.makedirs(relay_dest, exist_ok=True)
            mock_sdk = os.path.join(base_tmp, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            with open(os.path.join(relay_dest, "st-relay-arm64"), "wb") as f:
                f.write(b"ARM64" * 10)
            with open(os.path.join(relay_dest, "st-relay-armv7"), "wb") as f:
                f.write(b"ARMV7" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws + "\\",
                "ANDROID_HOME": os.path.join(base_tmp, "mock_sdk") + "/",
                "RUNNER_TEMP": base_tmp + "\\"
            }

            res = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(res.returncode, 0, f"Failed on trailing slashes:\nSTDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}")

    def test_workflow_packaging_with_mixed_slash_env(self):
        """Execute packaging workflow when GITHUB_WORKSPACE uses forward slashes."""
        with tempfile.TemporaryDirectory() as base_tmp:
            ws = os.path.join(base_tmp, "workspace")
            os.makedirs(ws, exist_ok=True)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(os.path.join(install_dir, "modules"), exist_ok=True)
            relay_dest = os.path.join(install_dir, "modules", "android")
            os.makedirs(relay_dest, exist_ok=True)
            mock_sdk = os.path.join(base_tmp, "mock_sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            with open(os.path.join(relay_dest, "st-relay-arm64"), "wb") as f:
                f.write(b"ARM64" * 10)
            with open(os.path.join(relay_dest, "st-relay-armv7"), "wb") as f:
                f.write(b"ARMV7" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            ws_forward = ws.replace("\\", "/")
            sdk_forward = os.path.join(base_tmp, "mock_sdk").replace("\\", "/")

            env_vars = {
                "GITHUB_WORKSPACE": ws_forward,
                "ANDROID_HOME": sdk_forward,
                "RUNNER_TEMP": base_tmp
            }

            res = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(res.returncode, 0, f"Failed on forward slashes:\nSTDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}")

    def test_relay_fallback_staging_with_spaces_in_path(self):
        """Verify fallback staging handles install path containing spaces without Copy-Item self-overwrite."""
        with tempfile.TemporaryDirectory() as base_tmp:
            ws = os.path.join(base_tmp, "work space build 2026")
            os.makedirs(ws, exist_ok=True)
            install_dir = os.path.join(ws, "build", "install")
            os.makedirs(install_dir, exist_ok=True)
            mock_sdk = os.path.join(base_tmp, "mock sdk", "platform-tools")
            os.makedirs(mock_sdk, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            relay_dest = os.path.join(install_dir, "modules", "android")
            os.makedirs(relay_dest, exist_ok=True)
            with open(os.path.join(relay_dest, "st-relay-arm64"), "wb") as f:
                f.write(b"ARM64" * 10)
            with open(os.path.join(relay_dest, "st-relay-armv7"), "wb") as f:
                f.write(b"ARMV7" * 10)
            with open(os.path.join(mock_sdk, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(mock_sdk, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(mock_sdk, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": os.path.join(base_tmp, "mock sdk"),
                "RUNNER_TEMP": base_tmp
            }

            res = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(res.returncode, 0, f"Failed on fallback in path with spaces:\nSTDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}")
            self.assertNotIn("Cannot overwrite the item", res.stderr)
            self.assertTrue(os.path.exists(os.path.join(install_dir, "modules", "android", "st-relay-arm64")))
            self.assertFalse(os.path.exists(os.path.join(install_dir, "android", "st-relay-arm64")))

    def test_adb_self_overwrite_with_spaces_in_path(self):
        """Verify ADB self-overwrite guard works in directories containing spaces."""
        with tempfile.TemporaryDirectory() as base_tmp:
            ws = os.path.join(base_tmp, "work space sdk test")
            install_dir = os.path.join(ws, "build", "install")
            pt_dir = os.path.join(install_dir, "platform-tools")
            os.makedirs(pt_dir, exist_ok=True)
            relay_dest = os.path.join(install_dir, "modules", "android")
            os.makedirs(relay_dest, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"OPENTRACK" * 10)
            with open(os.path.join(relay_dest, "st-relay-arm64"), "wb") as f:
                f.write(b"ARM64" * 10)
            with open(os.path.join(relay_dest, "st-relay-armv7"), "wb") as f:
                f.write(b"ARMV7" * 10)
            with open(os.path.join(pt_dir, "adb.exe"), "wb") as f:
                f.write(b"ADB" * 10)
            with open(os.path.join(pt_dir, "AdbWinApi.dll"), "wb") as f:
                f.write(b"DLL1" * 10)
            with open(os.path.join(pt_dir, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"DLL2" * 10)

            env_vars = {
                "GITHUB_WORKSPACE": ws,
                "ANDROID_HOME": install_dir,
                "RUNNER_TEMP": base_tmp
            }

            res = self.run_ps_script(self.package_script, env_vars)
            self.assertEqual(res.returncode, 0, f"Failed on ADB self-overwrite with spaces:\nSTDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}")
            self.assertNotIn("Cannot overwrite the item", res.stderr)
            self.assertTrue(os.path.exists(os.path.join(install_dir, "adb.exe")))
            self.assertFalse(os.path.exists(os.path.join(install_dir, "modules", "adb.exe")))

    # -------------------------------------------------------------------------
    # Category 3: Bracket character wildcard vulnerability analysis
    # -------------------------------------------------------------------------
    def test_powershell_test_path_wildcard_behavior_with_brackets(self):
        """Demonstrate that PowerShell Test-Path with positional -Path interprets brackets as wildcards,
        whereas -LiteralPath treats them literally."""
        with tempfile.TemporaryDirectory() as base_tmp:
            bracket_dir = os.path.join(base_tmp, "dir[test]")
            os.makedirs(bracket_dir, exist_ok=True)
            bracket_file = os.path.join(bracket_dir, "test.txt")
            with open(bracket_file, "w") as f:
                f.write("data")

            # In PowerShell: Test-Path -Path fails on [test] because it looks for dirl, dire, dirs, dirt
            # Test-Path -LiteralPath succeeds
            ps = f"""
            $f = "{bracket_file}"
            $withoutLit = Test-Path -Path $f
            $withLit = Test-Path -LiteralPath $f
            if ($withoutLit -eq $false -and $withLit -eq $true) {{
                Write-Output "CONFIRMED_POWERSHELL_WILDCARD_BEHAVIOR"
            }} else {{
                Write-Error "Unexpected: withoutLit=$withoutLit withLit=$withLit"
                exit 1
            }}
            """
            res = self.run_ps_command(ps)
            self.assertEqual(res.returncode, 0, f"Failed:\n{res.stderr}")
            self.assertIn("CONFIRMED_POWERSHELL_WILDCARD_BEHAVIOR", res.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
