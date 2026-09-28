"""Single YAML/CMake extract helper for packaging and layout tests."""

import os

import yaml

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
WORKFLOW_FILE = os.path.join(REPO_ROOT, ".github", "workflows", "windows-11.yml")
CMAKE_FILE = os.path.join(REPO_ROOT, "tracker-smoothtrack", "CMakeLists.txt")


def extract_workflow_step(step_name: str) -> str:
    """Return the `run` script text of a named windows-11-x64 workflow step."""
    with open(WORKFLOW_FILE, "r", encoding="utf-8") as fh:
        data = yaml.safe_load(fh)
    steps = data["jobs"]["windows-11-x64"]["steps"]
    for step in steps:
        if step.get("name") == step_name:
            return step.get("run", "")
    raise ValueError(f"Step '{step_name}' not found in {WORKFLOW_FILE}")


def extract_cmake() -> str:
    """Return tracker-smoothtrack/CMakeLists.txt contents."""
    with open(CMAKE_FILE, "r", encoding="utf-8") as fh:
        return fh.read()
