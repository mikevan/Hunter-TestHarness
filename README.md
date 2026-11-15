# Hunter-TestHarness

People relying on a warning system need evidence that it works. Hunter-TestHarness makes controlled inputs repeatable while the application follows its ordinary field workflow.

The dongle uses Hunter's shared core, commands, event records, packet evidence and card transfer. Vixen has no scenario screen or harness-specific analysis route. The operator starts the source; the ordinary application records and analyzes what arrives.

## Run it yourself

1. Put an SD card in the prototype and power it from a battery. Its display should say **Ready / stopped** and **SD ready**. The RGB LED stays off.
2. In Vixen **Ops**, scan, select **Hunter-Test**, and connect. Wait for **Connected**. USB is not required.
3. In **Mission**, enable **Deauth**, **EAPOL** and **Remote ID**. Leave trackers off. Wait for the selection to be applied. In Vixen 140, the ordinary **All Sensors** shortcut selects the default non-tracker set, including Remote ID on a supporting Hunter.
4. Return to **Ops**. Press the dongle's **BOOT** button once, or use Vixen's normal **Start**. Use one control. No assistant action is needed.
5. Watch the ordinary event view and the analysis status on Ops. Leave monitoring running after the input finishes.

| Time from Start | Evidence |
| --- | --- |
| 0â€“19 seconds | Baseline; no fixture detection. |
| 20â€“27 seconds | Eight deauthentication sightings; the packet window contains controlled deauthentication frames. |
| 32â€“35 seconds | Four EAPOL M1 sightings from the same observed transmitter address, with a complete EAPOL-Key packet body. |
| 50â€“53 seconds | Four structured, unverified Remote ID Basic ID sightings. The normal detail field preserves the service-data bytes. |
| 61 seconds | Input finishes; **Data done; monitoring**. Only the operator's Stop ends monitoring. |

Vixen's ordinary analysis workflow should move from waiting to analyzing and then display the supported sequence and its limitations. A timeout, unavailable model or rejected result is a test failure to preserve, not an all-clear. The model interprets a bounded evidence pattern; references and exact timing arithmetic come from procedural code. Its concise finding is rendered from the accepted pattern and preserved facts.

There are **three raw detector records per complete run**. Vixen may group the two Wi-Fi records into one emitter row, so expect two additional emitter rows rather than three. Prior evidence remains. Notification delivery follows ordinary permissions and sleep settings.

To repeat, **Stop, then Start**. Repeated Start while already monitoring does not reset the clock or replay a completed fixture. Reconnection does not clear source events. Neither power-up nor BOOT formats the card.

## What this test establishes

This tests live BLE transport, normal commands, preservation, source-clock timing and automatic offline interpretation. The RF observations are synthetic; the source SD record identifies the fixture. No deauthentication traffic or drone-identification impersonation is transmitted over the air.

Deauthentication requests do not prove a successful outage. EAPOL does not prove compromise. Remote ID is an unverified identification broadcast, not authenticated aircraft presence or intent. This sequence alone does not establish the separate Wi-Fi/BLE-outage evacuation policy or a coordinated attack. Actual RF receiver validation remains a separate field exercise.

## Evidence and independence

The source uses normal SD event logging and Wi-Fi PCAP windows. Vixen receives ordinary summaries, source/detail text, source uptime and packet windows. No expected answer or simulation instruction is sent to its model. Compare normal Vixen exports with the source card; preserve both.

Freeze the application and model before a run. Change scenarios on the test source and record all artifact identities. The owner may approve a separate qualification package before the field run; remove it afterward. It supplies no product controls or records to field collections. See [the compatibility record](docs/normal-hunter-compatibility.md) and [Vixen's bounded AI workflow](https://github.com/mikevan/Vixen/blob/field/ecosystem-evidence-parity/docs/live-signature-mvp.md).

## Build

```text
platformio run -e tdongle_c5_harness
python test/run_harness_tests.py --cxx g++
```

Use the default harness target only for the spare prototype. Identify its hardware before upload. The real Hunter device is separate. Serial `INFO` is read-only bench status; field operation needs no computer. [The legacy Hunter reference](docs/legacy-readme.md) retains inherited documentation.

The [three incremental test definitions](docs/incremental-field-tests.md) specify the next scenarios and their pass criteria. They remain planned until the repaired baseline is verified.
