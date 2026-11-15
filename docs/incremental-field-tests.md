# Three progressively harder MVP tests

The easiest operator-run test proved that the ordinary application invoked its offline model and displayed the supported sequence. These next tests increase the burden on the complete capture-to-analysis workflow. They are defined here before execution; they are not yet implemented as scenario firmware and have not passed.

## First repair and repeat the baseline

Vixen 140 saved EAPOL summaries but its automatic packet-preservation filter selected only deauth. Vixen 141 extends the ordinary filter to native Wi-Fi EAPOL windows, retaining the same model and inference contract. No BLE observation is fabricated into a Wi-Fi PCAP.

Repeat the existing 20/32/50-second baseline using the unchanged source firmware. Expected: eight deauth, four EAPOL and four Remote ID sightings; correct source-clock order and a 30-second deauth-to-Remote-ID gap; one accepted AI assessment; an assessment within 30 seconds of the final required evidence reaching Vixen; notification submitted; monitoring still active after input completion. Preserve both closed native windows: 48 deauth and 24 EAPOL frames. Remote ID must retain its complete service-data payload. The original interpretation pass remains valid, while the original packet-preservation gap remains recorded.

Do not mark this repair physically verified until the new EAPOL transfer is observed. Independent SD readback is a separate evidence check: compare ordered packet payloads, counts and source identities rather than requiring whole PCAP hashes to match after timestamp wrapping. Preserve each original file and its hash.

## Test 1: Timing variation and duplicate delivery

**What gets harder:** the source no longer presents evenly spaced sightings, and some summaries/text are delivered twice. Source counts and source timestamps remain unchanged by duplicate transport. Repeated replies must respect ordinary request IDs and framing, such as replies to normal retries; malformed or unsolicited messages are not equivalent duplicates.

Deauth starts at 20 seconds; EAPOL for the same observed network identifier starts at 37 seconds; Remote ID starts at 82 seconds. The supported gap is **62 seconds**, within the 90-second window. Sightings continue irregularly until 29, 43 and 87 seconds respectively.

**Pass:** correct sequence and 62-second source-clock gap; original counts retained; duplicate delivery does not create a second positive notification or repeated accepted analysis for the same completed input. Both packet windows and the original Remote ID payload survive. Monitoring continues. Receipt delays must not replace source timestamps.

## Test 2: Separate the streams

**What gets harder:** retain Test 1, then add newer deauth from network B at 55 seconds and unrelated EAPOL from network C at 65 seconds. Only network A supplies the matching deauth/EAPOL pair. There is one Remote ID follow-on at 82 seconds.

**Pass:** the application still finds A's supported sequence and its 62-second gap. It must not stitch B and C together, lose A merely because B is newer, invent another drone, claim independent corroboration, or attribute the Remote ID transmitter to a particular Wi-Fi actor. Preserve the distractors as evidence. The result remains a temporal association, with one positive notification for the completed candidate.

This deliberately exercises candidate selection before inference as well as the model. A missing analysis is a failure even if the model passed single-pair qualification cases. There is no prediction that the current selector already passes this test.

## Test 3: Interrupted delivery and the temporal boundary

**What gets harder:** retain the unrelated streams and repeated deliveries. The source temporarily makes its BLE connection unavailable from 28 to 58 seconds while continuing to record on the same boot/session clock. It does not reboot or clear its records. Remote ID begins at 111 seconds, **91 seconds after A's first deauth**.

**Pass:** the application reconnects, restores recoverable records and packet windows, explicitly accounts for any unrecoverable loss, and uses the restored source clock to report **outside the 90-second window**. No positive matched-sequence notification is permitted. Replayed old records are not new RF sightings. Missing evidence must be labelled incomplete, never treated as all-clear. An accurate timing decision does not compensate for a missing recoverable packet window: interpretation can pass while the full-evidence test fails. An unreported gap always fails.

This is a negative control: rejecting the combined signature is the successful result. Actual source capture gaps, a reboot or an unaligned clock invalidate the planned timing reference and must be reported rather than repaired into a passing answer.

## Freeze and execution rules

After the repaired baseline, keep Vixen 141 and its model, prompt, grammar, sensor selection and inference settings unchanged across Tests 1â€“3. Only the test source's scenario changes. The same ordinary Start/Stop or BOOT controls operate every scenario; no application test interface, injected collection or assistant trigger is permitted.

Record source firmware and APK/model hashes, operator start, source first/last timestamps, counts, packet payloads, transport gaps, raw model output, assessment latency, notification submission and continued monitoring. Qualify actual notification display separately from submission to Android. Observer logging is read-only and must not initiate a scenario or alter decisions. Preserve a failure before making a separately versioned product fix; restart qualification after such a fix instead of changing the application mid-series.

[Machine-readable reference plan](../tests/scenarios/incremental-v1.json) keeps expected answers outside model input. These authored synthetic cases are evaluation references, not independent field incidents or training data. RF receiver sensitivity, a real outage, drone intent and T-Ultra corroboration remain separate field validations.
