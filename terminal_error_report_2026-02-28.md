# Terminal Error Report (Agent-Side)

Date: 2026-02-28
Scope: `PlasmaNozzle` Phase 2 implementation/validation session

## Why this report exists

You reported that the VS Code GUI did not match what I was seeing from tool-run terminal output. This report documents the exact error classes observed from my side, likely causes, and final resolved status.

## Observed error classes

### 1) Intermittent stdlib dependency errors during precompile/test

Observed messages included:
- `Package PlasmaNozzle does not have Statistics in its dependencies`
- `Package PlasmaNozzle does not have Random in its dependencies`
- `Package Test not found in current path`
- `Statistics is a direct dependency, but does not appear in the manifest`

### 2) Runtime symbol/import mismatch

Observed message:
- `UndefVarError: norm not defined in PlasmaNozzle.FlowDiagnostics`

### 3) Non-deterministic test behavior across repeated runs

Observed pattern:
- Some runs failed (including one mode assertion), while later runs passed cleanly.
- Multiple subsequent full runs eventually stabilized at `16 / 16` passing.

### 4) Terminal capture/noise inconsistencies

Observed messages included one-off shell noise unrelated to package tests (e.g. VS Code app/shell startup noise) and occasional partial output captures where only test start banners appeared.

## Probable root causes

1. **Project/Manifest drift during iterative edits**
   - During the session, dependencies (`Statistics`, `Random`, `Test`) and imports were adjusted while test processes and precompile caches were active.
   - This can create transient states where one process sees stale dependency metadata.

2. **Incorrect UUID episode for `Statistics` during recovery attempts**
   - One failing run showed an invalid UUID path for `Statistics`; later state is corrected.

3. **Precompile cache race/staleness effects across repeated invocations**
   - Repeated `Pkg.test()` with changing dependency metadata can briefly produce contradictory results until `Project.toml` + manifest + cache are aligned.

4. **A real code import issue occurred once**
   - `norm` missing in `FlowDiagnostics` was a legitimate import-level failure at that point in time, later corrected.

## Why GUI and agent output could differ

- The agent tools may execute in a shell process with independent command sequencing from what is currently focused in GUI terminals.
- If multiple terminals/runs are active, the GUI may show the latest successful run while an earlier failing process is what the agent is reading (or vice versa).
- Truncated tool captures can temporarily hide completion lines until rerun.

## Final stabilized state

At end of session:
- Repeated full tests succeeded: `PlasmaNozzle | 16 / 16` pass.
- 3D script runs succeeded and produced outputs in `outputs/`.
- Project dependency entries are currently correct in `Project.toml`:
  - `Statistics = 10745b16-79ce-11e8-11f9-7d13ad32a3b2`
  - `Random = 9a3f8284-a2c9-5f02-9a11-845980a1fd5c`
  - `[extras] Test = 8dfed614-e22c-5e08-85e1-65c5234f0b40`
- Git working tree is clean.

## Addendum: Additional terminal issues during Phase 2 visualization (same date)

During the follow-up request to generate a Phase 2 visualization, more terminal-side workflow interruptions were observed:

1. **Foreground command capture truncation during long Julia runs**
   - Symptom: output stopped after `Activating project ...` while the process continued.
   - Consequence: next command in the same shared terminal could send `^C` to the still-running process.

2. **Process collision in shared terminal session**
   - Symptom: follow-up `ls`/`tail` commands interrupted active Julia runs and produced long interrupt stack traces.
   - Consequence: misleading logs (partial stack traces) and zero-byte temporary artifacts in some attempts.

3. **Recurring shell startup noise in agent terminal wrappers**
   - Repeated lines:
     - `error: app/com.visualstudio.code/x86_64/stable not installed`
     - `bash: : No such file or directory`
   - This appears to be environment/shell-wrapper noise, not a `PlasmaNozzle` runtime error.

### Mitigation that worked

- Use isolated/background terminal invocations for long jobs and explicitly await completion.
- Verify completion by checking output artifact integrity (size/type), not only terminal stream continuity.
- Persist run metrics to a dedicated log file and inspect that file directly.

Validated artifact from this Phase 2 run path:
- `outputs/phase2_bifurcation_3d_static.png` (non-zero, valid PNG)

Captured Phase 2 run summary (from `outputs/phase2_quick.log`):
- `Snapshot frame (peak-vorticity mode): 79`
- `Argon looped: 384 / 384`
- `Deuterium accelerated: 342 / 384`
- `phase2 config: algebra=octonion, budget=adaptive, coupling=minimal`

## Preventive checks for future sessions

1. Run in this order after dependency edits:
   1) `julia --project=. -e 'using Pkg; Pkg.resolve()'`
   2) `julia --project=. -e 'using Pkg; Pkg.instantiate()'`
   3) `julia --project=. -e 'using Pkg; Pkg.test()'`

2. Avoid overlapping test/precompile runs in multiple terminals while editing `Project.toml`.

3. If output looks contradictory, run one explicit clean check in a single terminal:
   - `julia --project=. -e 'using PlasmaNozzle; println("loaded")'`
   - `julia --project=. -e 'using Pkg; Pkg.test()'`

4. Prefer one active validation terminal per session for reproducible logs.

## Addendum: VS Code UI crash candidate while validating Linux package (2026-03-01)

Context: while validating Linux packaging/linking behavior for `opentrack`, a terminal command likely correlated with a VS Code UI crash.

### Exact command attempted

`(cd .pkg/opentrack/bin && ./opentrack --help >/tmp/opentrack-help.txt 2>&1 || true); head -n 20 /tmp/opentrack-help.txt`

### Most likely trigger

- The risky segment is `./opentrack --help` (launching the packaged Qt executable from the integrated terminal).
- Even with `--help`, some builds may initialize Qt/GUI-related subsystems early.
- This can interact badly with compositor/graphics/embedded terminal/editor state and present as a VS Code UI crash.

### Why this is likely the culprit

- Other packaging checks run immediately before this were non-launch diagnostics and completed normally:
   - `readelf -d .pkg/opentrack/bin/opentrack | grep -E 'RPATH|RUNPATH'`
   - `ldd .pkg/opentrack/bin/opentrack | grep -E 'opentrack-(...)\\.so'`
- The only step that actually executed the app binary was the `--help` invocation.

### Mitigation for future validation

- Avoid launching GUI-capable binaries from the integrated terminal when tracking editor stability issues.
- Prefer non-exec checks (`readelf`, `ldd`, file inspection) for packaging validation.
- If execution is required, run from an external terminal session and capture logs to file.

## Addendum: Integrated terminal command interruption during long package builds (2026-03-02)

Context: while building `monado-git` from `makepkg` in VS Code integrated terminal, follow-up commands from the agent interrupted the active build.

### Exact commands observed

1) Build command:

`cd /home/nos/headtracking/monado/docs && makepkg -f`

2) Follow-up status probe that caused interruption:

`pwd && ls -1 /home/nos/headtracking/monado/docs | head`

### Observed symptom

- Active `ninja` build was interrupted with `^C` and terminated.
- Terminal output showed:
   - `ninja: build stopped: interrupted by user.`
   - `==> ERROR: Aborted by user! Exiting...`

### Likely trigger

- In this VS Code environment, issuing a new foreground command while a long foreground process is running can inject interrupt behavior into the shared integrated terminal session instead of queueing the next command.

### Mitigation

- Run long tasks (`makepkg`, full builds, test suites) in a background terminal session and use explicit output polling.
- Avoid sending any additional foreground command to that terminal until completion is confirmed.
- For progress checks, read log/artifact files or use terminal output retrieval APIs rather than executing another command in the same live foreground session.

## Addendum: Monado service apparent hang during integrated-terminal smoke test (2026-03-02)

Context: after successful `monado-git` package build, a smoke test command launched service binaries from the package root.

### Exact command segment

`"$pkgroot/bin/monado-service" --help >/tmp/monado-service-help.txt 2>&1`

### Observed symptom

- Command appeared to hang for a while in integrated terminal.
- Captured output showed normal service startup instead of usage text, including:
   - `The Monado service has started.`
   - system builder enumeration logs.

### Likely trigger

- `monado-service --help` behaves like service startup, not an immediate help/exit CLI path.
- Concurrent heavy filesystem I/O (for example Baloo file extractor indexing) can amplify perceived hangs during startup/log flush.

### Mitigation

- Do not use `monado-service --help` as a fast smoke probe.
- For non-blocking checks, run `monado-service` in background and inspect logs/health via `monado-ctl`.
- During package smoke testing, prefer finite commands (`monado-cli info`, `monado-cli probe`) and avoid daemon commands in shared foreground terminals.

## Addendum: Prompt/input collision while background logs are active (2026-03-02)

Context: during live Option 1 validation with `monado-service` and bridge running in background, a foreground probe command was attempted in the shared integrated terminal session.

### Exact command attempted

`sleep 2; echo 'bridge_frames='$(grep -c 'CIRCUIT CLOSED' /tmp/monado-opt1-live-bridge.log 2>/dev/null || echo 0); tail -n 8 /tmp/monado-opt1-live-bridge.log 2>/dev/null || true; echo '--- service remote ---'; grep -E 'Config selected remote|Using builder remote|Listening on port' /tmp/monado-opt1-live-service.log | tail -n 10 || true`

### Observed symptom

- Terminal showed interleaved/truncated prompt text (`^CenTrack -> Monado ...`) and command exited with code `130`.
- Intended probe output did not complete reliably.

### Likely trigger

- Shared integrated terminal state collided with active background-output context, causing control/input handling to be unreliable for immediate foreground probes.

### Mitigation

- Prefer log-file reads for status checks while long-running background processes are active.
- Keep one terminal dedicated to background tasks and use separate terminals (or tool-level file reads) for diagnostics.

## Addendum: Low-disk + terminal instability during package verification (2026-03-04)

Context: while rebuilding and repackaging `opentrack` after a reported VS Code UI crash, disk utilization and integrated terminal behavior were checked.

### Exact commands involved

1) Disk check:

`df -h "$PWD" && du -sh build 2>/dev/null || true`

2) Foreground package attempts (intermittent capture/interruption):

`pkg="opentrack-linux-install-$(date +%Y%m%d-%H%M%S).tar.gz" && tar -C install -czf "$pkg" . && gzip -t "$pkg" && tar -tzf "$pkg" >/dev/null && ls -lh "$pkg" && echo "PACKAGE_OK:$pkg"`

3) Isolated background packaging with log capture (stable):

`set -e; cd /home/nos/headtracking/opentrack/build; log=/tmp/opentrack-package-$(date +%Y%m%d-%H%M%S).log; pkg="opentrack-linux-install-$(date +%Y%m%d-%H%M%S).tar.gz"; { ...tar/gzip/tar -tzf checks...; } >"$log" 2>&1`

### Observed symptom

- Filesystem nearly full during operation (`/dev/sdc1 ... 5.7G free, 98% used`).
- One foreground terminal instance reported closure.
- One foreground probe returned `^C`/exit `130` during package-file listing.
- Background+log workflow completed and produced validated artifacts.

### Likely trigger

- Very high filesystem utilization increased risk of UI/terminal instability during I/O-heavy archive operations.
- Shared foreground integrated-terminal state remained vulnerable to interruption/capture glitches.

### Mitigation

- Pre-check free space before packaging; keep several GB headroom (preferably >10GB).
- Use isolated background packaging plus explicit log verification for long archive operations.
- Validate artifacts via `gzip -t` and `tar -tzf` before declaring package success.

## Addendum: Integrated terminal partial-capture during packaging probe (2026-03-04)

Context: while resuming `opentrack` validation after a UI crash report, a non-exec packaging probe in the shared integrated terminal produced partial/no output on the first attempt.

### Exact command attempted

`set -e
plugin_path=$(find . -type f -name 'opentrack-filter-alpha-spectrum.so' | head -n 1)
echo "PLUGIN=$plugin_path"
readelf -d "$plugin_path" | grep -E 'RPATH|RUNPATH' || true
ldd "$plugin_path" | sed -n '1,80p'`

### Observed symptom

- Tool capture returned only the prompt + first line (`set -e`) with no subsequent output.
- Re-running as a single-line command immediately produced expected output (plugin path, RUNPATH, and `ldd` lines).

### Likely trigger

- Shared integrated-shell state/prompt handling issue with multiline command capture in this environment.
- Not a binary-execution crash path (no GUI executable was launched in this probe).

### Mitigation

- Prefer single-line non-interactive commands for integrated-terminal diagnostics.
- Keep packaging checks non-exec (`readelf`, `ldd`) and avoid GUI-capable binary launches from integrated terminal.

## Addendum: Known-good install/package workflow (future reference, 2026-03-04)

Context: after adding Alpha Spectrum adaptive + auto-stable calibration changes, install and packaging were completed successfully with the following flow.

### Install stage (recommended)

- Use CMake Tools build target `install` (avoids manual generator mismatch issues).
- Successful install root observed:
   - `/home/nos/headtracking/opentrack/build/install`
- Filter binary installed at:
   - `/home/nos/headtracking/opentrack/build/install/libexec/opentrack/opentrack-filter-alpha-spectrum.so`

### Package stage (portable archive)

When no CMake `package` target / CPack config is available, use a verified tarball flow:

`out=build/opentrack-install-20260304-linux.tar.gz && tmp=${out}.tmp && rm -f "$tmp" && tar -C build -cf - install | gzip -9 > "$tmp" && gzip -dc "$tmp" >/dev/null && tar -tzf "$tmp" >/dev/null && mv -f "$tmp" "$out" && ls -lh "$out"`

### Integrity check (required)

`ls -lh build/opentrack-install-20260304-linux.tar.gz && gzip -t build/opentrack-install-20260304-linux.tar.gz && tar -tzf build/opentrack-install-20260304-linux.tar.gz >/dev/null && echo PACKAGE_OK`

### Notes

- Prefer one-line commands in integrated terminal to reduce partial-capture behavior.
- If long-running packaging is interrupted in foreground, rerun in background and then verify with `gzip -t` + `tar -tzf`.

## Addendum: VS Code crash with successful post-restart build + incremental packaging guidance (2026-03-04)

Context: VS Code crashed again during active development, but after restart the CMake build completed successfully.

### Observed symptom

- Editor process crashed/restarted.
- Rebuild after restart succeeded, indicating code/build state remained valid.

### Clarification on packaging scope

- Yes: the current tar command repackages the full install tree each time it is run.
- No: full repackaging is not required for every filter iteration.

### Incremental workflow (recommended for rapid filter testing)

1) Build only (or just the filter target when available).
2) Copy/refresh only the updated plugin `.so` for testing:
    - Build output:
       - `/home/nos/headtracking/opentrack/build/filter-alpha-spectrum/opentrack-filter-alpha-spectrum.so`
    - Installed plugin location:
       - `/home/nos/headtracking/opentrack/build/install/libexec/opentrack/opentrack-filter-alpha-spectrum.so`
3) Repackage full archive only for share/release checkpoints.

### Mitigation

- Use plugin-only refresh for most local tests to reduce I/O pressure and terminal fragility.
- Reserve full tar packaging for milestone validations.

## Addendum: VS Code UI crash during active filter UI refactor (2026-03-05)

Context: during iterative `filter-alpha-spectrum` UI refactoring and documentation updates, VS Code crashed once and restarted; subsequent CMake build succeeded.

### Exact command context

- No long-running/GUI-launch command was active at crash time (editor-focused refactor).
- Most recent safe plugin refresh command in session:
   - `cp -f build/filter-alpha-spectrum/opentrack-filter-alpha-spectrum.so build/install/libexec/opentrack/opentrack-filter-alpha-spectrum.so && sha256sum build/filter-alpha-spectrum/opentrack-filter-alpha-spectrum.so build/install/libexec/opentrack/opentrack-filter-alpha-spectrum.so`

### Observed symptom

- VS Code UI terminated unexpectedly during editing.
- After restart, branch state remained intact and CMake Tools build completed successfully.

### Likely trigger

- Editor/UI instability under sustained workspace load and repeated UI regeneration (`uic`/`moc`) cycles.

### Mitigation

- Continue using plugin-only build/refresh for tight loops.
- Avoid foreground long-running terminal operations while editing UI-heavy files.
- Keep crash notes appended with exact command context and post-restart validation outcome.

## Addendum: Partial terminal capture during full gold package run (2026-03-05)

Context: while executing a full install+package verification flow, integrated terminal output truncated after `set -euo pipefail` even though the command completed successfully.

### Exact command style

- Foreground shell block with install + tar + integrity checks redirected to log:
   - `cmake --install build --prefix build/install`
   - `tar -C build -czf build/opentrack-linux-install-<stamp>.tar.gz install`
   - `gzip -t` and `tar -tzf`

### Observed symptom

- Tool capture showed only command preamble line.
- Follow-up log inspection confirmed successful completion and valid package.

### Mitigation

- Keep full packaging log-backed and validate from log/artifact rather than trusting live stream continuity.
- Proven good outputs in this run:
   - `PACKAGE_OK:build/opentrack-linux-install-20260305-135043.tar.gz`
   - SHA256: `e4e91354ebb443cb1aeb43f4968cfea62518240aa7d35d8b12981d8a32d246b8`

## Addendum: Additional VS Code crash during filter-alpha-spectrum edits (2026-03-05)

Context: another VS Code crash occurred during ongoing `filter-alpha-spectrum` refinement work.

### Immediate observed state after restart

- No failing build/test command was active at crash capture time.
- Last terminal command (post-restart history) was a packaging log verification command and it had already completed with exit code `0`.
- Recent active editing context included translation/UI cleanup (`filter-alpha-spectrum/lang/stub.ts`).

### Likely trigger class

- Editor/UI instability under ongoing multi-file refactor activity, not a deterministic CMake or packaging failure.

### Mitigation

- Continue using short, bounded command batches with log-backed verification for longer flows.
- Keep frequent checkpointing during UI/translation edits (small commits/stashes) to minimize loss window.
- Avoid running unrelated long foreground terminal blocks while performing rapid UI text/object-name refactors.

## Addendum: Repeated VS Code freezes with WM impact, process-state capture (2026-03-05)

Context: fourth/fifth crash in a row during ongoing `opentrack` work; user captured `ps -aux | grep code` immediately after freeze/recovery.

### Exact command captured

- `ps -aux | grep "code"`

### Observed symptom pattern (from capture)

- Main VS Code process present in uninterruptible sleep state: `code` with state `Dsl`.
- Additional heavy process states included:
   - `code --type=zygote` in `Dl` with high CPU.
   - `ms-vscode.cpptools` process at sustained high CPU (`~43%`).
   - `cpptools-srv` workers with high RSS usage (hundreds of MB each).
   - Node utility service process with very high memory footprint (`~1.4–1.5 GB RSS`).
- User-visible impact: editor freeze severe enough to stall/freeze window manager responsiveness.

### Likely trigger class

- Resource-pressure lockup in Electron/extension worker set (not a compiler/package command failure).
- Most suspicious contributors in this capture are C/C++ language services (`cpptools*`) plus large Node/Electron utility-worker memory growth during active multi-file edits.

### Mitigation

- Immediate recovery path when freeze recurs:
   - Save buffers if possible, then terminate only `cpptools`/`cpptools-srv` first; restart VS Code if UI remains stalled.
- Session hardening for this workspace:
   - Keep only one C/C++ language service active (avoid duplicate indexers/extensions).
   - Exclude heavy/generated dirs from indexing (especially `build/`, `.pkg/`, and large generated trees).
   - Avoid leaving multiple large log/grep streams active while editing UI/translation files.
   - Continue log-backed terminal validation; treat GUI responsiveness as separate from build correctness.
