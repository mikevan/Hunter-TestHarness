# Normal Hunter compatibility repair

The prior harness connected successfully but rejected Vixen's normal `ReconStart` requests with `UnknownOp`. It advertised only display/button capabilities and sent scenario stages in custom `0xE0` notifications. The ordinary Vixen decoder did not interpret those as Hunter events. Connection success therefore did not demonstrate monitoring, evidence preservation, or AI analysis.

## Repair boundary

Only Hunter-TestHarness changes. It now attaches the same shared `LayerTimeCore` and `LinkServer` used by Hunter. The fixture source implements the standard monitor port; command results, Status/change sequence, event summaries, text fragmentation, and event history come from the existing core. The custom vendor sender is removed.

The test source uses normal sensor-selection and allow-list semantics. Start is idempotent while running, Stop cancels remaining observations, event IDs are retained across reconnect and never reused by Clear Events, and each new fixture run uses fresh locally administered source identifiers. Disconnect does not abort acquisition or erase events.

The standard evidence store and Link 0.2 handlers serve controlled deauth frames; normal SD logging, PCAP persistence, and card-transfer handlers are reused. The LED is forced off after display and SD bus operations. Existing SD files are not formatted or cleared during installation.

## Scope of the repaired fixture

The current ordinary interface supports deauth and AirTag observations, not a typed outage or drone observation. The supported fixture therefore produces deauth at 20 seconds and AirTag at 50 seconds, completing at 61 seconds. It is a compatibility and evidence test. It must not be presented as the original outage/drone signature or a successful evacuation warning. Vixen's model admission and ordinary AI integration remain unchanged.

## Validation

The host integration test exercises actual shared-core command/result messages, source timing, sensor filtering, repeated Start, Stop, reconnect mirrors, monotonic event IDs, fragmented source text, normal packet-window/byte responses, PCAP construction, and uptime wrap. It checks that ordinary replies do not emit the retired vendor frame.

Firmware compilation and physical installation are separate from the operator-run test. Bench identification and read-only boot checks do not count as a field run. The owner starts and observes the scenario using the [normal operating steps](../README.md#run-it-yourself), without an assistant injecting commands or changing Vixen.

## Completed checks for this repair

- Shared-library dependency checks and all 529 inherited Hunter host checks passed.
- The new normal-protocol integration test passed its command, timing, event/text, sensor-filter, retention, and packet-evidence assertions.
- The final harness firmware built and was flashed to the independently identified test prototype. Read-only boot status confirmed normal-protocol firmware, a live phone connection, SD logging ready, zero write errors, zero dropped records, and the LED off.
- The ordinary phone handshake applied its evidence-hold command successfully. The harness remained stopped with zero events; the assistant did not start the operator's scenario.
- Vixen remained on build 139 and its source checkout was not changed. Existing SD contents were retained.

These checks establish firmware and protocol readiness for the operator-run compatibility scenario. They are not a completed phone scenario, RF sensing validation, or AI-warning acceptance.

## Monitoring lifetime correction

The first normal-protocol fixture automatically stopped monitoring at 61 seconds. The operator observed Vixen displaying STOPPED after the run. This was source behavior, not a two-hit limit or Vixen shutting down, and it did not represent ordinary ongoing Hunter monitoring.

Firmware v3 separates fixture completion from monitoring state. At 61 seconds the sample sequence is exhausted, the display says Data done; monitoring, and the normal monitoring flag and selection remain active. Only the operator's Stop command or BOOT stop action ends monitoring. Start while monitoring remains idempotent; Stop then Start begins another fixture. The integration test advances past five minutes, checks that monitoring remains active and counts stay unchanged, and verifies explicit Stop/restart. Vixen is unchanged.

## Sensor-selection instruction correction

The operator saw two deauth events from separate starts and no AirTag. The recorded sensor mask excluded AirTag. The earlier instruction to select All Sensors was incorrect for this Vixen version: its quick preset uses the default list, which excludes trackers. The self-service steps now require enabling AirTag explicitly in Mission alongside Deauth and avoiding the default preset afterward. No app setting is changed by the assistant, and Vixen itself remains unchanged.

## LED and connection indicators

The owner confirmed the RGB LED is dark after the dedicated GPIO 4/5 shutdown update. The manufacturer's current C5 pin map assigns APA102 clock/data to those pins; earlier hardware shared the display SPI bus, so the legacy bus shutdown remains too. Repeated writes of unchanged display labels were removed. The eye follows the actual phone link; the face/cheek remains red with no implemented mesh connection. No mesh connection is invented for the fixture.
