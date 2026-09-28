#!/usr/bin/env python3
"""Mock Android Debug Bridge CLI for SmoothTrack tests."""

from __future__ import annotations

import os
import shutil
import sys
import time


def _log_argv(args):
    log_path = os.environ.get("MOCK_ADB_LOG")
    if not log_path:
        return
    parent = os.path.dirname(log_path)
    if parent:
        os.makedirs(parent, exist_ok=True)
    with open(log_path, "a", encoding="utf-8") as fh:
        fh.write(" ".join(args) + "\n")


def _strip_serial(args):
    if len(args) >= 2 and args[0] == "-s":
        return args[2:]
    return args


def _shell_is_relay_launch(shell_args):
    return bool(shell_args) and shell_args[0] == "/data/local/tmp/st-relay"


def _shell_tokens(shell_args):
    text = " ".join(shell_args)
    for sep in ("$(", ")", "||", "|", ";", "&"):
        text = text.replace(sep, " ")
    return text.split()


def _shell_is_kill(shell_args):
    parts = _shell_tokens(shell_args)
    return any(tok in ("pkill", "killall", "pidof", "kill") for tok in parts)


def _write_relay_pid():
    pid_path = os.environ.get("MOCK_ADB_RELAY_PID")
    if not pid_path:
        return
    parent = os.path.dirname(pid_path)
    if parent:
        os.makedirs(parent, exist_ok=True)
    with open(pid_path, "w", encoding="utf-8") as fh:
        fh.write(str(os.getpid()))
        fh.flush()
        os.fsync(fh.fileno())


def _sleep_until_killed():
    _write_relay_pid()
    try:
        while True:
            time.sleep(0.05)
    except KeyboardInterrupt:
        return 0
    return 0


def main(argv):
    args = argv[1:]
    _log_argv(args)
    args = _strip_serial(args)
    if not args:
        return 0

    cmd = args[0]

    if cmd == "start-server":
        ms = int(os.environ.get("MOCK_ADB_START_SERVER_MS", "0") or "0")
        if ms > 0:
            time.sleep(ms / 1000.0)
        return 0

    if cmd == "devices":
        devices = os.environ.get(
            "MOCK_ADB_DEVICES",
            "emulator-5554 device product:sdk model:sdk",
        )
        sys.stdout.write(devices)
        if devices and not devices.endswith("\n"):
            sys.stdout.write("\n")
        return 0

    if cmd == "reverse":
        return 0

    if cmd == "push":
        if len(args) >= 3:
            src, dest = args[1], args[2]
            fs_root = os.environ.get("MOCK_ADB_FS")
            if fs_root and dest.rstrip("/").endswith("st-relay"):
                os.makedirs(fs_root, exist_ok=True)
                shutil.copy(src, os.path.join(fs_root, "st-relay"))
        return 0

    if cmd == "shell":
        shell_args = args[1:]
        if shell_args[:2] == ["getprop", "ro.product.cpu.abi"]:
            sys.stdout.write("arm64-v8a\n")
            return 0
        if shell_args and shell_args[0] == "chmod":
            return 0
        if _shell_is_relay_launch(shell_args):
            if os.environ.get("MOCK_ADB_RELAY_EXIT") == "1":
                sys.stderr.write("Failed to connect to TCP reverse tunnel\n")
                return 5
            return _sleep_until_killed()
        if _shell_is_kill(shell_args):
            sys.stdout.write("kill\n")
            return 0
        sys.stderr.write("unknown shell command\n")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
