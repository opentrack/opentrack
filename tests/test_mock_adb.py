"""Subprocess tests for tests/mock_adb — these fail until the CLI contract works.

These cover the mock adb CLI only. Production adb_client (C++ QProcess) is not
launched from Python; C++ QTest is not in scope for this PR.
"""

from __future__ import annotations

import os
import subprocess
import sys
import tempfile
import time
import unittest

_TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
if _TESTS_DIR not in sys.path:
    sys.path.insert(0, _TESTS_DIR)

from mock_adb_cli import ADB_PY, WRAPPER, mock_adb_env, run_mock_adb as _run


def _tasklist_has_pid(pid):
    result = subprocess.run(
        ["tasklist", "/FI", f"PID eq {pid}", "/NH"],
        capture_output=True,
        text=True,
    )
    out = result.stdout
    if "No tasks" in out:
        return False
    return str(pid) in out


def _pid_alive(pid):
    if pid <= 0:
        return False
    if sys.platform == "win32":
        return _tasklist_has_pid(pid)
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    return True


def _kill_pid_tree(pid):
    if pid <= 0:
        return
    if sys.platform == "win32":
        subprocess.run(
            ["taskkill", "/F", "/T", "/PID", str(pid)],
            capture_output=True,
            text=True,
        )
        return
    try:
        os.kill(pid, 9)
    except OSError:
        pass


class TestMockAdbCli(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.TemporaryDirectory()
        self.log = os.path.join(self.tmpdir.name, "adb.log")
        self.fs = os.path.join(self.tmpdir.name, "fs")
        os.makedirs(self.fs, exist_ok=True)
        self.env = {
            "MOCK_ADB_LOG": self.log,
            "MOCK_ADB_FS": self.fs,
        }

    def tearDown(self):
        self.tmpdir.cleanup()

    def _log_lines(self):
        with open(self.log, encoding="utf-8") as fh:
            return [line.rstrip("\n") for line in fh]

    def test_argv_recorded_to_mock_adb_log(self):
        result = _run(["devices", "-l"], self.env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(self._log_lines(), ["devices -l"])

    def test_devices_l_default_output(self):
        result = _run(["devices", "-l"], self.env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(
            result.stdout.strip(),
            "emulator-5554 device product:sdk model:sdk",
        )

    def test_devices_l_honors_mock_adb_devices(self):
        env = dict(self.env)
        env["MOCK_ADB_DEVICES"] = "pixel-usb device product:oriole model:Pixel_6"
        result = _run(["devices", "-l"], env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout.strip(), env["MOCK_ADB_DEVICES"])

    def test_start_server_default_is_immediate(self):
        t0 = time.perf_counter()
        result = _run(["start-server"], self.env)
        elapsed_ms = (time.perf_counter() - t0) * 1000.0
        self.assertEqual(result.returncode, 0)
        self.assertLess(elapsed_ms, 1000.0)

    def test_start_server_sleeps_mock_adb_start_server_ms(self):
        env = dict(self.env)
        env["MOCK_ADB_START_SERVER_MS"] = "250"
        t0 = time.perf_counter()
        result = _run(["start-server"], env, timeout=5)
        elapsed_ms = (time.perf_counter() - t0) * 1000.0
        self.assertEqual(result.returncode, 0)
        self.assertGreaterEqual(elapsed_ms, 200.0)
        self.assertIn("start-server", self._log_lines())

    def test_reverse_tcp_logged_and_exits_zero(self):
        result = _run(["reverse", "tcp:4242", "tcp:4242"], self.env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(self._log_lines(), ["reverse tcp:4242 tcp:4242"])

    def test_reverse_remove_exits_zero(self):
        result = _run(["reverse", "--remove", "tcp:4242"], self.env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(self._log_lines(), ["reverse --remove tcp:4242"])

    def test_push_copies_src_to_mock_adb_fs(self):
        src = os.path.join(self.tmpdir.name, "st-relay-arm64")
        with open(src, "wb") as fh:
            fh.write(b"ELF-RELAY")
        result = _run(["push", src, "/data/local/tmp/st-relay"], self.env)
        self.assertEqual(result.returncode, 0)
        dest = os.path.join(self.fs, "st-relay")
        self.assertTrue(os.path.isfile(dest), "push must copy into $MOCK_ADB_FS/st-relay")
        with open(dest, "rb") as fh:
            self.assertEqual(fh.read(), b"ELF-RELAY")

    def test_shell_getprop_abi(self):
        result = _run(["shell", "getprop", "ro.product.cpu.abi"], self.env)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout.strip(), "arm64-v8a")

    def test_shell_chmod_exits_zero(self):
        result = _run(["shell", "chmod", "755", "/data/local/tmp/st-relay"], self.env)
        self.assertEqual(result.returncode, 0)

    def test_shell_kill_helpers_exit_zero(self):
        for args in (
            ["shell", "pkill", "-f", "st-relay"],
            ["shell", "killall", "st-relay"],
            ["shell", "pidof", "st-relay"],
            ["shell", "kill", "1"],
            [
                "shell",
                "sh",
                "-c",
                "pkill -f st-relay || killall st-relay || kill $(pidof st-relay) || true",
            ],
        ):
            with self.subTest(args=args):
                result = _run(args, self.env)
                self.assertEqual(result.returncode, 0)
                self.assertEqual(result.stdout.strip(), "kill")

    def test_kill_tokens_do_not_match_substrings(self):
        result = _run(["shell", "echo", "not_pkill_here"], self.env)
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("kill", result.stdout.split())
        self.assertIn("unknown shell command", result.stderr)

    def test_relay_exit_one_prints_stderr_and_exits_five(self):
        env = dict(self.env)
        env["MOCK_ADB_RELAY_EXIT"] = "1"
        result = _run(["shell", "/data/local/tmp/st-relay", "4242", "4242"], env)
        self.assertEqual(result.returncode, 5)
        self.assertIn("Failed to connect to TCP reverse tunnel", result.stderr)

    def test_relay_sleeps_until_killed(self):
        pid_path = os.path.join(self.tmpdir.name, "relay.pid")
        env = dict(self.env)
        env["MOCK_ADB_RELAY_PID"] = pid_path
        proc = subprocess.Popen(
            [sys.executable, ADB_PY, "shell", "/data/local/tmp/st-relay", "4242", "4242"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            env=mock_adb_env(env),
        )
        child_pid = None
        popped_pid = proc.pid
        try:
            deadline = time.time() + 2.0
            while time.time() < deadline and not os.path.isfile(pid_path):
                time.sleep(0.02)
            self.assertTrue(os.path.isfile(pid_path), "mock must write MOCK_ADB_RELAY_PID")
            with open(pid_path, encoding="utf-8") as fh:
                child_pid = int(fh.read().strip())
            self.assertEqual(child_pid, popped_pid)
            self.assertIsNone(proc.poll(), "relay must sleep until killed")
        finally:
            proc.kill()
            _kill_pid_tree(popped_pid)
            if child_pid and child_pid != popped_pid:
                _kill_pid_tree(child_pid)
            proc.wait(timeout=5)

        self.assertIsNotNone(proc.returncode, "python adb.py must exit on kill")
        proc = None
        deadline = time.time() + 2.0
        while time.time() < deadline and _pid_alive(popped_pid):
            time.sleep(0.05)
        self.assertFalse(_pid_alive(popped_pid), "python adb.py sleeper must be gone")
        if child_pid:
            self.assertFalse(_pid_alive(child_pid), "pid-file process must be gone")

    def test_wrapper_runs_short_devices_command(self):
        result = subprocess.run(
            [WRAPPER, "devices", "-l"],
            capture_output=True,
            text=True,
            env=mock_adb_env(self.env),
            timeout=5,
        )
        self.assertEqual(result.returncode, 0)
        self.assertEqual(
            result.stdout.strip(),
            "emulator-5554 device product:sdk model:sdk",
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
