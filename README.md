# Hunter-TestHarness

People relying on a warning system need evidence that it works. Hunter-TestHarness makes controlled inputs repeatable while the application under test follows its ordinary field workflow.

The harness now speaks the normal Hunter protocol. Vixen's **Start**, **Stop**, sensor selection, event list, text requests, packet evidence, and card transfer use the shared Hunter core and wire handlers. No Vixen test screen, custom vendor frame, or agent-controlled trigger is required.

## Run it yourself

1. Put an SD card in the test dongle and power it from a battery. Its display should say **Ready / stopped** and **SD ready**. The RGB LED stays off.
2. Open Vixen. In **Ops**, tap **Scan**, select **Hunter-Test**, and tap **Connect**. Wait for **Connected**. USB is not required.
3. Open **Mission**, enable **AirTag**, and ensure **Deauth** is enabled. Wait for the selection to be applied, then return to Ops. **Do not tap the current All Sensors preset afterward:** that preset restores a default list that excludes trackers, including AirTag. A disabled sensor or narrower selection intentionally filters its observations.
4. Tap Vixen's normal **Start**, or press the dongle's **BOOT** button once. Use either control; do not press both to restart. The dongle shows **Running t+…s**, and Vixen's monitoring status should become active.
5. Leave it running for at least 61 seconds and watch the normal **Events** view. No assistant action is needed.

| Time from Start | Expected result with both sensors enabled |
| --- | --- |
| 0–19 seconds | Baseline; no fixture detection yet. |
| 20 seconds | A normal deauth event appears. Further sightings through 27 seconds bring its count to eight. Its normal packet-evidence window contains controlled 802.11 deauthentication frames. |
| 50 seconds | A normal AirTag event appears. Further sightings through 53 seconds bring its count to four. |
| 61 seconds | Fixture input finishes. The dongle shows **Data done; monitoring**. Monitoring, the connection, and saved events remain active until you press Stop. |

Expect **two additional event rows per complete run**, not necessarily two total: previous runs are retained. Notification delivery follows Vixen's normal settings and permissions. BLE AirTag observations do not have a Wi-Fi PCAP attached.

Tap **Stop** or press **BOOT** while monitoring is active to stop. To repeat the scenario, **Stop, then Start**; this creates a new run with fresh fixture identifiers. Repeated Start while monitoring is already active does not reset its clock or replay a completed fixture. Reconnecting does not clear events or restart the scenario; the source continues recording while the phone is away.

## What this test establishes

This is a controlled normal-protocol compatibility test: commands, status, evidence preservation, event display, and supported detection handling. The fixture observations and deauth packet bytes are synthetic and remain identified as such in the source's SD record. The harness does not transmit deauth traffic or impersonate a tracker over the air; it delivers controlled evidence through its Hunter BLE connection.

**This run does not demonstrate the outage-followed-by-drone AI warning.** The current ordinary protocol has no dedicated outage/drone observation types, and Vixen's existing model qualification gate remains unchanged. An AirTag is not a drone. The original scenario's intended stages must not be relabelled as successful supported detections.

## Evidence and independence

The source uses Hunter's normal SD event log and stores its Wi-Fi packet windows as PCAP evidence. Fixture run provenance is recorded on the source card. Vixen receives ordinary event summaries, source/detail text, and packet windows; expected answers are not sent as model instructions.

Use Vixen's normal evidence exports and the source SD records for comparison. Preserve prior records. **Clear Events** clears the current event mirror; **Clear Card** is the ordinary explicitly confirmed destructive card action. Neither is required to start a run. Power-up and BOOT never format the card.

The application stays fixed for a test. Change scenarios on this test device, record the firmware/application/model identities, and judge each result against the actual evidence. See [the compatibility repair and validation record](docs/normal-hunter-compatibility.md).

## Build and inspect

```text
platformio run -e tdongle_c5_harness
python test/run_harness_tests.py --cxx g++
```

The host tests use a native C++17 compiler and the pinned shared libraries. Upload only after independently identifying the spare test board. The original Hunter checkout and Vixen are separate and unchanged by this firmware work.

Serial `INFO` is read-only bench status. Field operation requires no serial console. Card maintenance remains a separate MAC-confirmed firmware target. The upstream Hunter remote has pushing disabled. [The legacy Hunter reference](docs/legacy-readme.md) preserves inherited product documentation.
