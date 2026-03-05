# IMU High-Rate Integration Test Plan

## Test Date: March 5, 2026

## Implementation Summary
Successfully implemented Option A for high-rate IMU integration:
- **API Extension**: Added `IHighrateSource` interface and `IFilter::set_tracker()` hook
- **Producer**: `tracker-fusion` implements `IHighrateSource` with 1000Hz buffered sample export
- **Consumer**: `alpha-spectrum` binds tracker, retrieves samples, integrates rotation deltas into Predictive head

## Modified Components
1. `api/plugin-api.hpp` + `.cpp` - Core interface definitions
2. `logic/runtime-libraries.cpp` - Tracker-to-filter binding
3. `tracker-fusion/fusion.h` + `.cpp` - High-rate provider implementation
4. `filter-alpha-spectrum/ftnoir_filter_alpha_spectrum.h` + `.cpp` - High-rate consumer implementation

## Build Status
✅ All targets compile without errors  
✅ Libraries deployed to `build/install/libexec/opentrack/`  
✅ Diagnostic logging enabled in alpha-spectrum filter

## Test Scenario 1: Basic Integration Path Validation

### Prerequisites
- Oculus Rift DK1 connected (for tracker-openhmd)
- Camera available (for tracker-pt as position source)
- OpenTrack built and installed in `build/install/`

### Configuration
1. **Tracker**: Fusion
   - Rotation tracker: OpenHMD (DK1 at 1000Hz)
   - Position tracker: PointTracker (camera)
   - Enable high-rate mode: ✅
   - Buffer size: 50ms

2. **Filter**: Alpha Spectrum
   - Predictive head: Enabled
   - MTM mode: Enabled (or disabled for deterministic path)

3. **Protocol**: UDP or Mouse (for visual output verification)

### Expected Diagnostic Output
```
fusion: high-rate polling thread started for source 1
alpha-spectrum: set_tracker called, highrate_source = available
alpha-spectrum: integrated X high-rate samples  (first 3 calls, then every 250th)
```

### Success Criteria
- [x] No segfaults or crashes on startup
- [ ] Diagnostic logs confirm tracker binding (`highrate_source = available`)
- [ ] High-rate polling thread starts when fusion tracker initializes
- [ ] Alpha-spectrum receives non-zero sample counts
- [ ] Rotation output responds with lower latency compared to low-rate-only mode

## Test Scenario 2: Fallback Behavior

### Configuration
Same as Scenario 1, but **disable high-rate mode** in Fusion tracker settings.

### Expected Behavior
```
alpha-spectrum: set_tracker called, highrate_source = available
```
(No "integrated X samples" messages - filter receives tracker but gets zero samples)

### Success Criteria
- [ ] Filter operates normally without high-rate data
- [ ] Predictive head uses velocity extrapolation (fallback path)
- [ ] No performance degradation

## Test Scenario 3: Non-Fusion Tracker

### Configuration
- **Tracker**: PointTracker only (no Fusion)
- **Filter**: Alpha Spectrum

### Expected Behavior
```
alpha-spectrum: set_tracker called, highrate_source = null
```

### Success Criteria
- [ ] Filter detects absence of `IHighrateSource` capability
- [ ] No attempts to retrieve high-rate samples
- [ ] Filter operates as before (baseline behavior)

## Manual Testing Steps

1. Launch OpenTrack from build directory:
   ```bash
   cd /home/nos/headtracking/opentrack/build/install
   ./bin/opentrack 2>&1 | tee opentrack-test.log
   ```

2. Configure tracker → filter → protocol chain via GUI

3. Start tracking (click Start button)

4. Observe console output for diagnostic messages

5. Test head rotation with DK1:
   - Smooth slow rotations (verify base tracking)
   - Fast rotations (verify high-rate enhancement)
   - Compare latency feel with high-rate mode enabled vs. disabled

6. Check `opentrack-test.log` for:
   - No error messages
   - Expected diagnostic output pattern
   - Sample count progression

## Integration Validation Checklist

### Code Path Verification
- [x] `IHighrateSource` symbols exported from `opentrack-api.so`
- [x] `fusion_tracker` implements `IHighrateSource` interface
- [x] `runtime_libraries.cpp` calls `pFilter->set_tracker(&*pTracker)`
- [x] `alpha_spectrum::set_tracker()` performs `dynamic_cast<IHighrateSource*>`
- [x] `integrate_highrate_samples()` called every filter frame
- [x] Predictive head conditional: `if (is_rotation && has_highrate_source && ...)`

### Runtime Validation
- [ ] OpenTrack starts without crashes
- [ ] Fusion tracker loads both sub-trackers (rotation + position)
- [ ] High-rate polling thread starts when enabled
- [ ] Alpha-spectrum receives tracker binding callback
- [ ] Dynamic cast succeeds for Fusion tracker
- [ ] Sample retrieval returns non-empty buffers during motion
- [ ] Gyro integration accumulates rotation deltas correctly
- [ ] Predictive head uses integrated deltas (not velocity extrapolation)

## Advanced Testing (Future)

### Entropy Feedback Loop Theory
- High-rate depletion affecting MTM mode probabilities
- Position refinement without new low-rate measurements
- Entropy accounting for sample integration
- Mode landscape sensitivity to high-rate stream

**Status**: Theory documented in `/memories/repo/opentrack-project.md`  
**Priority**: After basic functionality validated

## Known Issues / Limitations

1. **First sample initialization**: First high-rate sample establishes baseline pose, not integrated
2. **Wrap handling**: 360° discontinuities handled per-sample, but rapid multi-turn motion untested
3. **Buffer sizing**: Default 50ms @ 1000Hz = 50 samples; may need tuning for different IMU rates
4. **Timestamp accuracy**: Depends on QThread::usleep(1000) precision on target platform

## Next Steps

1. Execute Test Scenario 1 with DK1 + camera
2. Capture and analyze diagnostic logs
3. Measure rotation latency improvement (subjective + objective if possible)
4. Validate fallback behavior (Scenarios 2 & 3)
5. Profile CPU usage with high-rate mode enabled
6. Document findings and commit tested configuration

## Notes

- Diagnostic logging can be removed after validation (performance-sensitive path)
- Consider exposing high-rate buffer stats to UI for tuning
- Future: Unit tests for `integrate_highrate_samples()` with synthetic data
