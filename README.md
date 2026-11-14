# Hunter-TestHarness

Independent full copy of Hunter, including its working changes and pinned dependencies. The upstream remote has pushing disabled. Default firmware is the separate `tdongle_c5_harness` target; the original Hunter checkout is untouched.

Connect Vixen to the advertised `Hunter-Test-C5-…` source, then press BOOT. Over real wall-clock time it transmits baseline counts (0–10 s), local Wi-Fi/BLE drop (10–20 s), verified outage (20 s), drone-signature observation (50 s), and completion (61 s). Both observations use the source's common run clock. Each run has a new ID and sequenced 20-byte BLE notifications. A disconnect aborts the source run; a new press starts a new run. No historical file is played on the phone.

This firmware sends controlled observation telemetry over BLE. The receiving app quarantines it and shows the source identity outside model input. It does not establish physical RF outage or drone-detector accuracy. The model receives ordinary evidence and the configured policy, without test labels. The SD card records each accepted notification with exact frame bytes, run/session IDs and source time. The RGB LED stays off. Baseline counts are 120 Wi-Fi and 80 BLE records per ten seconds; outage counts are zero. Source health/selected coverage bits are stipulated fixture inputs.

Card maintenance is a separate build, never triggered by BOOT or startup. INFO identifies the attached chip; clearing requires its exact `CLEAR-SD-<chip MAC>` confirmation. No private device identity is embedded in source.

Wire extension: HELLO capability `0x8000`, Data type `0xE0`, length 20: version u8 at 1, stage u8 at 2, flags u8 at 3 (receiver health bit 0, selected coverage bit 1, SD recording bit 2, SD error bit 3), run u32 at 4, elapsed ms u32 at 8, sequence u16 at 12, Wi-Fi/BLE/drone counts u16 at 14/16/18, all little-endian. Stage 1 baseline; 2 drop; 3 confirmed outage; 4 drone; 5 complete. Never decode these packets as ordinary Hunter RF detections.

Build: `platformio run -e tdongle_c5_harness`. Upload only to an independently identified spare board. This branch retains the original Hunter documentation below for provenance.

---

# Hunter

LilyGO T-Dongle-C5 sensor, SD logging, PCAP evidence and card-transfer product. The Hunter logo is unchanged.

Clone with `git clone --recurse-submodules https://github.com/mikevan/Hunter.git`, or run `git submodule update --init --recursive` in an existing checkout. Core and C5 board support live under `libraries/`; Sensors lives under `sensors/`. No sibling repositories are required.

Run `python tools/dependencies_check.py` before building. The build checks the recorded Git commits, clean dependency worktrees and the existing content hashes. Update each gitlink and its lock entry together when adopting a new library revision.

Run native host checks with `python test/run_host_tests.py` using a C++17 compiler.

Build with `pio run -e tdongle_c5_hunter`. No hardware is flashed by a build. The legacy `tdongle_c5_vixen` environment is the old fox-hunt beacon, not the Vixen flagship.

Source ancestry is recorded in `EXTRACTION.json`; original Git history remains in LayerTime. EXTRACTION.json records the initial local preparation; subsequent checkpoints are recorded in Git history.

GitHub Actions runs dependency guard tests, host checks and a clean firmware build on every push and pull request. Successful runs retain firmware, ELF files and logs for 14 days. Shared-library candidate runs test temporary pins without updating this product. See the [hardware acceptance checklist](https://github.com/mikevan/LayerTime/blob/main/docs/firmware/hardware-acceptance.md).
