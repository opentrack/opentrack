"""Invoke tests/mock_adb as a subprocess.

Windows tests drive `sys.executable adb.py` so Popen.kill() reaps the sleeper.
The POSIX wrapper is used when it is executable; otherwise we fall back to
python adb.py.
"""

import os
import subprocess
import sys

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
MOCK_DIR = os.path.join(REPO_ROOT, "tests", "mock_adb")
ADB_PY = os.path.join(MOCK_DIR, "adb.py")
WRAPPER = os.path.join(MOCK_DIR, "adb.bat" if sys.platform == "win32" else "adb")


def mock_adb_argv():
    if sys.platform == "win32":
        return [sys.executable, ADB_PY]
    if os.path.isfile(WRAPPER) and os.access(WRAPPER, os.X_OK):
        return [WRAPPER]
    return [sys.executable, ADB_PY]


def mock_adb_env(extra=None):
    merged = os.environ.copy()
    py_dir = os.path.dirname(sys.executable)
    merged["PATH"] = py_dir + os.pathsep + merged.get("PATH", "")
    merged["PYTHON"] = sys.executable
    if extra:
        merged.update(extra)
    return merged


def run_mock_adb(args, env, timeout=5):
    return subprocess.run(
        mock_adb_argv() + list(args),
        capture_output=True,
        text=True,
        env=mock_adb_env(env),
        timeout=timeout,
    )
