# Generic CMake Deep Dependency Report Tool

This tool scans a CMake project and writes a Markdown report showing:

- declared targets (`add_library`, `add_executable`, `add_custom_target`, and `otr_module` wrappers)
- gate context from surrounding `if(...)` blocks
- dependency checks (`find_package`, `pkg_check_modules`, `otr_pkgconfig`)
- link directives (`target_link_libraries`, `target_link_options`, `link_libraries`, etc.)
- nearby cache variables (`set(... CACHE ...)`)

## Script

- `cmake-deps-report.py`
- `run-report.sh` (wrapper)

## Wrapper

The wrapper defaults to configured-only mode and switches to full report mode with `-v`/`--verbose`.

```bash
./contrib-noinst/cmake-deps-report/run-report.sh \
  --root . \
  --build-dir build \
  --output cmake-deep-deps-report.md
```

Full report with verbose mode:

```bash
./contrib-noinst/cmake-deps-report/run-report.sh \
  --root . \
  --output cmake-deep-deps-report-all.md \
  --verbose
```

## Usage

From any CMake repo root:

```bash
python path/to/cmake-deps-report.py \
  --root . \
  --output cmake-deep-deps-report.md
```

Filter to configured targets only (after a CMake configure step):

```bash
python path/to/cmake-deps-report.py \
  --root . \
  --build-dir build \
  --configured-only \
  --output cmake-deep-deps-report.md
```

Write to stdout:

```bash
python path/to/cmake-deps-report.py --root . --output -
```

## `glow` examples

Pipe directly to `glow` via stdin:

```bash
./contrib-noinst/cmake-deps-report/run-report.sh --root . --output - --verbose | glow -
```

Generate then open file in `glow`:

```bash
python path/to/cmake-deps-report.py --root . --output cmake-deep-deps-report.md

glow cmake-deep-deps-report.md
```

## Notes

- The parser is static and heuristic-based; it does not evaluate CMake variables or generator expressions.
- For wrapper macros (such as `otr_module`), it reports both the wrapper declaration and inferred target names.
- The configured-only mode reads `<build-dir>/CMakeFiles/TargetDirectories.txt`.
