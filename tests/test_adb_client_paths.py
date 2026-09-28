"""Path-contract checks for adb_client timeouts, relay search, and tracker copy."""

import os
import unittest

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
ADB_H = os.path.join(REPO_ROOT, "tracker-smoothtrack", "adb_client.h")
ADB_CPP = os.path.join(REPO_ROOT, "tracker-smoothtrack", "adb_client.cpp")
TRACKER_CPP = os.path.join(REPO_ROOT, "tracker-smoothtrack", "ftnoir_tracker_smoothtrack.cpp")

def _function_source(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        raise AssertionError(f"{signature!r} not found")
    rest = text[start + len(signature) :]
    nxt = len(rest)
    for marker in ("\nQString ", "\nbool ", "\nvoid ", "\nint "):
        pos = rest.find(marker)
        if 0 <= pos < nxt:
            nxt = pos
    return text[start : start + len(signature) + nxt]


def _start_android_source() -> str:
    with open(TRACKER_CPP, encoding="utf-8") as f:
        text = f.read()
    start = text.find("module_status smoothtrack::start_android")
    if start < 0:
        raise AssertionError("start_android not found")
    nxt = text.find("\nmodule_status ", start + 1)
    return text[start:nxt] if nxt > start else text[start:]


class TestAdbClientPaths(unittest.TestCase):
    def test_header_named_timeouts(self):
        with open(ADB_H, encoding="utf-8") as f:
            text = f.read()
        self.assertIn("START_SERVER_TIMEOUT_MS = 20000", text)
        self.assertIn("PUSH_TIMEOUT_MS = 15000", text)
        self.assertIn("DEFAULT_TIMEOUT_MS = 5000", text)
        self.assertIn("QUICK_TIMEOUT_MS = 1500", text)
        self.assertNotIn("DEFAULT_TIMEOUT_MS = 2000", text)

    def test_run_adb_cmd_distinguishes_start_failure_from_timeout(self):
        with open(ADB_CPP, encoding="utf-8") as f:
            text = f.read()
        start = text.find("bool run_adb_cmd")
        self.assertGreaterEqual(start, 0)
        body = text[start : text.find("void kill_device_relay", start)]
        self.assertIn("waitForStarted", body)
        self.assertIn("Failed to start '%1': %2", body)
        self.assertIn("Process timed out", body)
        self.assertIn("timer.elapsed()", body)

    def test_stop_gates_reverse_teardown(self):
        with open(ADB_CPP, encoding="utf-8") as f:
            text = f.read()
        body = _function_source(text, "void adb_client::stop")
        self.assertIn("reverse_installed && active_port", body)
        self.assertIn("kill-server", body)
        self.assertIn("started_adb_server", body)

    def test_start_verifies_relay_launch(self):
        with open(ADB_CPP, encoding="utf-8") as f:
            text = f.read()
        body = _function_source(text, "bool adb_client::start")
        self.assertIn("waitForFinished(400)", body)
        self.assertIn("collect_qprocess_output", body)
        self.assertIn("start-server", body)

    def test_get_device_abi_empty_on_failure(self):
        with open(ADB_CPP, encoding="utf-8") as f:
            text = f.read()
        body = _function_source(text, "QString adb_client::get_device_abi")
        self.assertNotIn("arm64-v8a", body)

    def test_find_adb_canonical_roots(self):
        with open(ADB_CPP, encoding="utf-8") as f:
            text = f.read()
        body = _function_source(text, "QString adb_client::find_adb")
        self.assertIn("user_hint", body)
        self.assertIn("applicationDirPath()", body)
        self.assertIn('findExecutable("adb")', body)
        self.assertNotIn("platform-tools", body)
        self.assertNotIn("Android/Sdk", body)

    def test_find_relay_binary_has_no_x86_64(self):
        with open(ADB_CPP, encoding="utf-8") as f:
            text = f.read()
        body = _function_source(text, "QString adb_client::find_relay_binary")
        self.assertNotIn("x86_64", body)
        self.assertIn("/modules/android/", body)
        self.assertIn("/../libexec/opentrack/android/", body)
        self.assertIn("/../Plugins/android/", body)
        self.assertNotIn("tracker-smoothtrack/android", body)

    def test_android_accept_timeout_does_not_say_tap_play(self):
        body = _start_android_source()
        self.assertNotIn("Tap Play in SmoothTrack", body)

    def test_android_skips_accept_wait_if_relay_not_running(self):
        body = _start_android_source()
        running_at = body.find("is_running()")
        wait_at = body.find("waitForNewConnection")
        self.assertGreaterEqual(running_at, 0)
        self.assertGreaterEqual(wait_at, 0)
        self.assertLess(running_at, wait_at)

    def test_android_accept_timeout_attaches_relay_stderr(self):
        body = _start_android_source()
        wait_at = body.find("waitForNewConnection")
        self.assertGreaterEqual(wait_at, 0)
        after_wait = body[wait_at:]
        stop_at = after_wait.find("adb->stop()")
        stderr_at = after_wait.find("relay_stderr()")
        self.assertGreaterEqual(stderr_at, 0)
        self.assertGreaterEqual(stop_at, 0)
        self.assertLess(stderr_at, stop_at)
        self.assertIn(".arg(port);", after_wait)
        self.assertNotIn(".arg(port).arg(port)", after_wait)


if __name__ == "__main__":
    unittest.main(verbosity=2)
