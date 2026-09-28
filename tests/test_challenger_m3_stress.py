"""Remaining M3 checks that parse real YAML/CMake or run the packaging step."""

import os
import sys
import subprocess
import tempfile
import unittest
import zipfile

import yaml

_TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
if _TESTS_DIR not in sys.path:
    sys.path.insert(0, _TESTS_DIR)

from extract import WORKFLOW_FILE, extract_cmake, extract_workflow_step

SMOOTHTRACK_DIR = os.path.join(
    os.path.abspath(os.path.join(_TESTS_DIR, "..")), "tracker-smoothtrack"
)


class TestR3CMakeDecoupling(unittest.TestCase):
    def test_r3_cmakelists_has_usbmuxd_option(self):
        content = extract_cmake()
        self.assertIn("option(OPENTRACK_HAS_USBMUXD", content)
        self.assertIn("target_compile_definitions(${self} PRIVATE", content)
        self.assertIn("OPENTRACK_SMOOTHTRACK_HAVE_USBMUXD", content)
        self.assertIn("libusbmuxd not found", content)
        self.assertIn("Android USB only", content)


class TestR4CIWorkflowAndPackagingStress(unittest.TestCase):
    def test_r4_workflow_yaml_structure_and_ndk_flags(self):
        with open(WORKFLOW_FILE, "r", encoding="utf-8") as f:
            workflow = yaml.safe_load(f)

        steps = workflow["jobs"]["windows-11-x64"]["steps"]
        step_names = [s.get("name") for s in steps]

        self.assertIn("Export Android NDK", step_names)
        self.assertIn("Build st-relay", step_names)
        self.assertIn("Package install tree", step_names)
        self.assertIn("Upload Windows 11 build artifact", step_names)

        setup_ndk = next(s for s in steps if s.get("uses") == "nttld/setup-ndk@v1")
        self.assertEqual(setup_ndk["with"]["ndk-version"], "r27c")
        self.assertEqual(setup_ndk["with"]["add-to-path"], False)

        compile_step = next(s for s in steps if s.get("name") == "Export Android NDK")
        self.assertEqual(
            compile_step["env"]["ANDROID_NDK_ROOT"],
            "${{ steps.setup-ndk.outputs.ndk-path }}",
        )
        compile_script = compile_step["run"]
        self.assertIn("ANDROID_NDK_ROOT", compile_script)
        self.assertIn("GITHUB_ENV", compile_script)
        self.assertIn("Android NDK setup failed", compile_script)
        self.assertNotIn(
            'Join-Path $env:GITHUB_WORKSPACE "tracker-smoothtrack\\android"',
            compile_script,
        )

        build_relay = next(s for s in steps if s.get("name") == "Build st-relay")
        self.assertIn("st-relay-android", build_relay["run"])

        cmake_text = extract_cmake()
        self.assertNotIn("-static", cmake_text)
        self.assertIn("-fPIE", cmake_text)
        self.assertIn("-Wl,-z,max-page-size=16384", cmake_text)
        self.assertIn("-O2", cmake_text)

    def test_r4_relay_c_source_code_integrity(self):
        src = ""
        for name in ("relay.c", "relay_io.h"):
            path = os.path.join(SMOOTHTRACK_DIR, "android", name)
            with open(path, "r", encoding="utf-8") as f:
                src += f.read()

        self.assertIn("PACKET_SIZE 48", src)
        self.assertIn("MAX_BUFFER", src)
        self.assertIn("TCP_NODELAY", src)
        self.assertIn("SO_REUSEADDR", src)
        self.assertIn("signal(SIGPIPE, SIG_IGN)", src)
        self.assertIn("st-relay: ready, bridging UDP", src)

    def test_r4_release_packaging_single_layout(self):
        package_script = extract_workflow_step("Package install tree")
        with tempfile.TemporaryDirectory() as td:
            workspace = os.path.join(td, "workspace")
            install_dir = os.path.join(workspace, "build", "install")
            relay_dir = os.path.join(install_dir, "modules", "android")
            pt_dir = os.path.join(td, "platform-tools")
            os.makedirs(install_dir, exist_ok=True)
            os.makedirs(relay_dir, exist_ok=True)
            os.makedirs(pt_dir, exist_ok=True)

            with open(os.path.join(install_dir, "opentrack.exe"), "wb") as f:
                f.write(b"MOCK_OPENTRACK_EXE" * 100)
            with open(os.path.join(relay_dir, "st-relay-arm64"), "wb") as f:
                f.write(b"MOCK_ARM64_RELAY" * 50)
            with open(os.path.join(relay_dir, "st-relay-armv7"), "wb") as f:
                f.write(b"MOCK_ARMV7_RELAY" * 50)
            with open(os.path.join(pt_dir, "adb.exe"), "wb") as f:
                f.write(b"MOCK_ADB_EXE" * 100)
            with open(os.path.join(pt_dir, "AdbWinApi.dll"), "wb") as f:
                f.write(b"MOCK_ADBWINAPI_DLL" * 100)
            with open(os.path.join(pt_dir, "AdbWinUsbApi.dll"), "wb") as f:
                f.write(b"MOCK_ADBWINUSBAPI_DLL" * 100)

            env = os.environ.copy()
            env["GITHUB_WORKSPACE"] = workspace
            env["ANDROID_SDK_ROOT"] = td
            env["RUNNER_TEMP"] = td

            ps_script_path = os.path.join(td, "test_package.ps1")
            with open(ps_script_path, "w", encoding="utf-8") as f:
                f.write(package_script)

            proc = subprocess.run(
                [
                    "powershell.exe",
                    "-NoProfile",
                    "-NonInteractive",
                    "-ExecutionPolicy",
                    "Bypass",
                    "-File",
                    ps_script_path,
                ],
                capture_output=True,
                text=True,
                env=env,
                cwd=workspace,
            )
            self.assertEqual(
                proc.returncode,
                0,
                f"Packaging script failed:\nSTDOUT:\n{proc.stdout}\nSTDERR:\n{proc.stderr}",
            )

            zip_files = [
                f for f in os.listdir(workspace) if f.startswith("opentrack-") and f.endswith(".zip")
            ]
            self.assertTrue(zip_files, f"No release zip in {workspace}: {os.listdir(workspace)}")
            with zipfile.ZipFile(os.path.join(workspace, zip_files[0]), "r") as zf:
                names = [n.replace("\\", "/") for n in zf.namelist()]
            self.assertIn("opentrack.exe", names)
            self.assertIn("adb.exe", names)
            self.assertIn("modules/android/st-relay-arm64", names)
            self.assertIn("modules/android/st-relay-armv7", names)
            self.assertNotIn("st-relay-arm64", names)
            self.assertNotIn("android/st-relay-arm64", names)
            self.assertNotIn("platform-tools/adb.exe", names)


if __name__ == "__main__":
    unittest.main(verbosity=2)
