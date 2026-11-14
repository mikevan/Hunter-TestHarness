# Hunter-TestHarness

People relying on a warning system need evidence that it works. We must be able to show what succeeds, what fails, and what observations support that judgement.

Hunter-TestHarness exists to make that proof repeatable. It lets us ask whether Vixen preserves a developing event's evidence, interprets the changing pattern, and delivers a useful warning in time.

## How we evaluate it

A dedicated, battery-powered device sends controlled observations over a real Bluetooth connection. Press BOOT to start a new timed scenario. The SD card records each accepted notification with its exact bytes, run/session identifiers, and source time so the transmitted record can be compared with the receiving system.

A valid end-to-end acceptance test uses the released Vixen application and its ordinary field workflow. Pre-existing instrumentation may measure that workflow; it must not change its decisions or provide a separate route to success. Repeat runs need independent identities, preserved evidence, and explicit expected outcomes.

## What the device provides

Connect a compatible receiver to `Hunter-Test-C5-…`, then press BOOT. Over real wall-clock time it transmits baseline counts (0–10 s), a local Wi-Fi/BLE drop (10–20 s), stipulated outage observations (20 s), a drone-signature observation (50 s), and completion (61 s). Both observations use the source's common run clock. Each run has a new ID and sequenced 20-byte BLE notifications. A disconnect aborts the source run; a new press starts a new run. The RGB LED stays off.

Baseline counts are 120 Wi-Fi and 80 BLE records per ten seconds; outage counts are zero. Source health and selected coverage are stipulated fixture inputs. No historical file is played on the phone, and the device does not create a physical outage or establish drone-detector accuracy.

## Current integration limit

This is a controlled telemetry source, not a demonstration that the ordinary field warning workflow succeeds. Its experimental vendor message format still requires integration with Vixen's ordinary evidence workflow. The earlier separate test interface is not the acceptance path. Test provenance belongs in the evidence record; it must not coach the model toward the expected answer.

## Implementation and operation

This repository is an independent full copy of Hunter with pinned dependencies. The upstream remote has pushing disabled. The default firmware target is `tdongle_c5_harness`; the original Hunter checkout remains separate.

Card maintenance is a separate build, never triggered by BOOT or startup. INFO identifies the attached chip; clearing requires its exact `CLEAR-SD-<chip MAC>` confirmation. No private device identity is embedded in source.

Wire extension: HELLO capability `0x8000`, Data type `0xE0`, length 20: version u8 at 1, stage u8 at 2, flags u8 at 3 (receiver health bit 0, selected coverage bit 1, SD recording bit 2, SD error bit 3), run u32 at 4, elapsed ms u32 at 8, sequence u16 at 12, Wi-Fi/BLE/drone counts u16 at 14/16/18, all little-endian. Stage 1 baseline; 2 drop; 3 confirmed outage; 4 drone; 5 complete. Never decode these packets as ordinary Hunter RF detections.

Build: `platformio run -e tdongle_c5_harness`. Upload only to an independently identified spare board. The original Hunter documentation is retained in [the legacy reference](docs/legacy-readme.md). Hunter is the capture product; this repository is the controlled test source.
