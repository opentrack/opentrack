#!/usr/bin/env python3

from __future__ import annotations

import argparse
import pathlib
import re
from dataclasses import dataclass
from typing import Iterable


@dataclass
class Command:
    name: str
    args: str
    file: pathlib.Path
    line: int
    context: tuple[str, ...]


@dataclass
class TargetDecl:
    display_name: str
    target_name: str
    decl_kind: str
    file: pathlib.Path
    line: int
    args: str
    extra_flags: set[str]
    context: tuple[str, ...]


COMMAND_START_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(")
WHITESPACE_RE = re.compile(r"\s+")


def normalize_space(text: str) -> str:
    return WHITESPACE_RE.sub(" ", text.strip())


def strip_comment(line: str) -> str:
    if "#" not in line:
        return line
    in_quote = False
    out: list[str] = []
    for ch in line:
        if ch == '"':
            in_quote = not in_quote
            out.append(ch)
            continue
        if ch == "#" and not in_quote:
            break
        out.append(ch)
    return "".join(out)


def parse_commands(path: pathlib.Path) -> list[Command]:
    lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
    commands: list[Command] = []

    context_stack: list[str] = []
    buf: list[str] = []
    start_line = 0
    depth = 0

    for i, raw in enumerate(lines, start=1):
        line = strip_comment(raw)
        if not buf:
            if not line.strip():
                continue
            m = COMMAND_START_RE.match(line)
            if not m:
                continue
            start_line = i
            buf = [line]
            depth = line.count("(") - line.count(")")
            if depth > 0:
                continue
        else:
            buf.append(line)
            depth += line.count("(") - line.count(")")
            if depth > 0:
                continue

        text = "\n".join(buf)
        buf = []

        m = re.match(r"\s*([A-Za-z_][A-Za-z0-9_]*)\s*\((.*)\)\s*$", text, re.S)
        if not m:
            continue

        name = m.group(1).strip().lower()
        args = normalize_space(m.group(2))

        if name == "if":
            context_stack.append(args)
            continue
        if name == "elseif":
            if context_stack:
                context_stack[-1] = f"ELSEIF {args}"
            else:
                context_stack.append(f"ELSEIF {args}")
            continue
        if name == "else":
            if context_stack:
                context_stack[-1] = f"ELSE ({context_stack[-1]})"
            else:
                context_stack.append("ELSE")
            continue
        if name == "endif":
            if context_stack:
                context_stack.pop()
            continue

        commands.append(Command(name=name, args=args, file=path, line=start_line, context=tuple(context_stack)))

    return commands


def token_set(text: str) -> list[str]:
    if not text:
        return []
    return [t.strip() for t in re.split(r"\s+", text) if t.strip()]


def collect_targets(commands: list[Command]) -> list[TargetDecl]:
    out: list[TargetDecl] = []

    for c in commands:
        toks = token_set(c.args)
        if not toks:
            continue

        if c.name == "add_library":
            t = toks[0]
            out.append(
                TargetDecl(
                    display_name=t,
                    target_name=t,
                    decl_kind="add_library",
                    file=c.file,
                    line=c.line,
                    args=c.args,
                    extra_flags={x for x in toks[1:] if x.isupper()},
                    context=c.context,
                )
            )
        elif c.name == "add_executable":
            t = toks[0]
            out.append(
                TargetDecl(
                    display_name=t,
                    target_name=t,
                    decl_kind="add_executable",
                    file=c.file,
                    line=c.line,
                    args=c.args,
                    extra_flags={x for x in toks[1:] if x.isupper()},
                    context=c.context,
                )
            )
        elif c.name == "add_custom_target":
            t = toks[0]
            out.append(
                TargetDecl(
                    display_name=t,
                    target_name=t,
                    decl_kind="add_custom_target",
                    file=c.file,
                    line=c.line,
                    args=c.args,
                    extra_flags=set(),
                    context=c.context,
                )
            )
        elif c.name == "otr_module":
            module_name = toks[0]
            out.append(
                TargetDecl(
                    display_name=module_name,
                    target_name=f"opentrack-{module_name}",
                    decl_kind="otr_module",
                    file=c.file,
                    line=c.line,
                    args=c.args,
                    extra_flags={x for x in toks[1:] if re.fullmatch(r"[A-Z0-9-]+", x)},
                    context=c.context,
                )
            )

    unique: dict[tuple[str, str, int], TargetDecl] = {}
    for t in out:
        unique[(str(t.file), t.target_name, t.line)] = t
    return sorted(unique.values(), key=lambda x: (x.target_name, str(x.file), x.line))


def is_prefix_context(prefix: tuple[str, ...], full: tuple[str, ...]) -> bool:
    if len(prefix) > len(full):
        return False
    return prefix == full[: len(prefix)]


def commands_near_target(target: TargetDecl, file_commands: list[Command]) -> list[Command]:
    out: list[Command] = []
    for c in file_commands:
        if c.name in {"add_library", "add_executable", "add_custom_target", "otr_module"}:
            continue
        if is_prefix_context(c.context, target.context) or is_prefix_context(target.context, c.context):
            out.append(c)
    return out


def extract_find_packages(commands: Iterable[Command]) -> list[str]:
    out: list[str] = []
    for c in commands:
        if c.name != "find_package":
            continue
        toks = token_set(c.args)
        if toks:
            out.append(toks[0])
    return sorted(set(out))


def extract_pkg_modules(commands: Iterable[Command]) -> list[str]:
    out: list[str] = []
    for c in commands:
        toks = token_set(c.args)
        if not toks:
            continue
        if c.name == "pkg_check_modules" and len(toks) >= 2:
            out.append(toks[-1])
        elif c.name in {"otr_pkgconfig", "otr_pkgconfig_"}:
            for t in toks[1:]:
                if re.fullmatch(r"[A-Za-z0-9_.+:-]+", t):
                    out.append(t)
    return sorted(set(out))


def extract_cache_vars(commands: Iterable[Command]) -> list[str]:
    out: set[str] = set()
    for c in commands:
        if c.name != "set":
            continue
        toks = token_set(c.args)
        if not toks:
            continue
        if "CACHE" in toks:
            out.add(toks[0])
    return sorted(out)


def extract_links(target: TargetDecl, commands: Iterable[Command]) -> list[str]:
    target_aliases = {target.target_name, target.display_name, "${self}"}
    out: list[str] = []

    for c in commands:
        if c.name not in {"target_link_libraries", "target_link_options", "target_link_directories", "link_libraries", "link_directories"}:
            continue

        text = normalize_space(c.args)
        toks = token_set(text)

        if c.name in {"target_link_libraries", "target_link_options", "target_link_directories"}:
            if not toks:
                continue
            tgt = toks[0]
            if tgt not in target_aliases and target.target_name not in text and "${self}" not in text:
                continue
            rest = [x for x in toks[1:] if x not in {"PRIVATE", "PUBLIC", "INTERFACE"}]
            if rest:
                out.append(f"{c.name}: " + " ".join(rest))
            continue

        if c.name in {"link_libraries", "link_directories"}:
            if c.context == target.context or is_prefix_context(c.context, target.context):
                out.append(f"{c.name}: {text}")

    return sorted(set(out))


def target_kind(t: TargetDecl) -> str:
    if t.decl_kind == "add_executable":
        return "executable"
    if t.decl_kind == "add_custom_target":
        return "custom-target"
    if t.decl_kind == "add_library":
        if "STATIC" in t.extra_flags:
            return "static-library"
        if "MODULE" in t.extra_flags:
            return "module-library"
        if "SHARED" in t.extra_flags:
            return "shared-library"
        return "library"
    if t.decl_kind == "otr_module":
        if "EXECUTABLE" in t.extra_flags:
            return "executable-wrapper"
        if "STATIC" in t.extra_flags:
            return "static-wrapper"
        return "module-wrapper"
    return "target"


def implicit_notes(t: TargetDecl) -> list[str]:
    notes: list[str] = []
    if t.decl_kind != "otr_module":
        return notes

    if "NO-QT" not in t.extra_flags:
        notes.append("Qt imported targets via otr_module")
    if "NO-COMPAT" not in t.extra_flags:
        notes.append("opentrack-api/opentrack-options/opentrack-compat via otr_module")
    if "NO-I18N" not in t.extra_flags and "NO-QT" not in t.extra_flags:
        notes.append("i18n setup via otr_module")
    notes.append("libm on UNIX via otr_compat")
    return notes


def markdown_escape(text: str) -> str:
    return text.replace("|", "\\|")


def format_context(context: tuple[str, ...]) -> str:
    return " && ".join(context) if context else "(none)"


def make_report(root: pathlib.Path, targets: list[TargetDecl], commands: list[Command], only_targets: set[str] | None) -> str:
    by_file: dict[pathlib.Path, list[Command]] = {}
    for c in commands:
        by_file.setdefault(c.file, []).append(c)

    filtered = [t for t in targets if only_targets is None or t.target_name in only_targets]
    filtered.sort(key=lambda t: (t.target_name, str(t.file), t.line))

    summary: list[str] = [
        "| Target | Kind | Declares | File | Gate | find_package | pkg-config | cache vars |",
        "|---|---|---|---|---|---|---|---|",
    ]
    details: list[str] = []

    for t in filtered:
        nearby = commands_near_target(t, by_file.get(t.file, []))
        packages = extract_find_packages(nearby)
        pkgmods = extract_pkg_modules(nearby)
        cache_vars = extract_cache_vars(nearby)
        links = extract_links(t, nearby)
        notes = implicit_notes(t)

        summary.append(
            "| "
            + " | ".join(
                [
                    markdown_escape(t.display_name),
                    markdown_escape(target_kind(t)),
                    markdown_escape(t.target_name),
                    markdown_escape(f"{t.file.relative_to(root)}:{t.line}"),
                    markdown_escape(format_context(t.context)),
                    markdown_escape(", ".join(packages) if packages else "-"),
                    markdown_escape(", ".join(pkgmods) if pkgmods else "-"),
                    markdown_escape(", ".join(cache_vars) if cache_vars else "-"),
                ]
            )
            + " |"
        )

        details.append(f"### {t.target_name}")
        details.append(f"- Display name: `{t.display_name}`")
        details.append(f"- Declaration: `{t.file.relative_to(root)}:{t.line}`")
        details.append(f"- Declaration kind: `{t.decl_kind}`")
        details.append(f"- Kind: `{target_kind(t)}`")
        details.append(f"- Gate context: `{format_context(t.context)}`")
        details.append(f"- Declaration args: `{t.args}`")
        if notes:
            details.append(f"- Implicit notes: {', '.join(f'`{n}`' for n in notes)}")
        if packages:
            details.append(f"- find_package: {', '.join(f'`{x}`' for x in packages)}")
        if pkgmods:
            details.append(f"- pkg-config checks: {', '.join(f'`{x}`' for x in pkgmods)}")
        if cache_vars:
            details.append(f"- Cache vars in scope: {', '.join(f'`{x}`' for x in cache_vars)}")
        if links:
            details.append("- Link directives:")
            for x in links:
                details.append(f"  - `{x}`")
        details.append("")

    body: list[str] = []
    body.append("# CMake Deep Dependency Report")
    body.append("")
    body.append(f"- Root: `{root}`")
    body.append(f"- Targets discovered: `{len(targets)}`")
    body.append(f"- Targets included in this report: `{len(filtered)}`")
    body.append("")
    body.append("## Target Summary")
    body.extend(summary)
    body.append("")
    body.append("## Target Details")
    body.extend(details)
    return "\n".join(body).rstrip() + "\n"


def read_configured_targets(build_dir: pathlib.Path) -> set[str]:
    target_file = build_dir / "CMakeFiles" / "TargetDirectories.txt"
    if not target_file.exists():
        return set()

    text = target_file.read_text(encoding="utf-8", errors="ignore")
    matches = re.findall(r"/CMakeFiles/([^/]+)\.dir", text)
    return {m for m in matches if m}


def gather_cmake_files(root: pathlib.Path) -> list[pathlib.Path]:
    out: list[pathlib.Path] = []
    for p in root.rglob("CMakeLists.txt"):
        parts = p.parts
        if ".git" in parts:
            continue
        if any(seg.startswith(".build") for seg in parts):
            continue
        out.append(p)
    return sorted(out)


def main() -> int:
    parser = argparse.ArgumentParser(description="Scan a CMake project and produce a Markdown dependency report.")
    parser.add_argument("--root", type=pathlib.Path, default=pathlib.Path.cwd(), help="Project root path")
    parser.add_argument("--build-dir", type=pathlib.Path, help="Configured CMake build dir for --configured-only filtering")
    parser.add_argument("--configured-only", action="store_true", help="Only include targets found in build-dir/CMakeFiles/TargetDirectories.txt")
    parser.add_argument("--output", type=pathlib.Path, default=pathlib.Path("cmake-deep-deps-report.md"), help="Output Markdown path, or '-' for stdout")
    args = parser.parse_args()

    root = args.root.resolve()
    cmake_files = gather_cmake_files(root)

    commands: list[Command] = []
    for p in cmake_files:
        commands.extend(parse_commands(p))

    targets = collect_targets(commands)

    configured: set[str] | None = None
    if args.configured_only:
        if not args.build_dir:
            raise SystemExit("--configured-only requires --build-dir")
        configured = read_configured_targets(args.build_dir.resolve())

    report = make_report(root, targets, commands, configured)

    if str(args.output) == "-":
        print(report, end="")
    else:
        out_path = args.output if args.output.is_absolute() else root / args.output
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(report, encoding="utf-8")
        print(f"Wrote report: {out_path}")

    print(f"Scanned CMake files: {len(cmake_files)}")
    print(f"Discovered targets: {len(targets)}")
    if configured is not None:
        print(f"Configured target filter size: {len(configured)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
