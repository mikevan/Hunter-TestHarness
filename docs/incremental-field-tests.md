# Three progressively harder MVP tests

The easiest operator-run test proved that the ordinary application invoked its offline model and displayed the supported sequence. These next tests increase the burden on the complete capture-to-analysis workflow. They are defined here before execution; they are not yet implemented as scenario firmware and have not passed.

## First repair and repeat the baseline

Vixen 140 saved EAPOL summaries but its automatic packet-preservation filter selected only deauth. Vixen 141 extends the ordinary filter to native Wi-Fi EAPOL windows, retaining the same model and inference contract. No BLE observation is fabricated into a Wi-Fi PCAP.

Repeat the existing 20/32/50-second baseline using the unchanged source firmware. Expected: eight deauth, four EAPOL and four Remote ID sightings; correct source-clock order and a 30-second deauth-to-Remote-ID gap; one accepted AI assessment; an assessment within 30 seconds of the final required evidence reaching Vixen; notification submitted; monitoring still active after input completion. Preserve both closed native windows: 48 deauth and 24 EAPOL frames. Remote ID must retain its complete service-data payload. The original interpretation pass remains valid, while the original packet-preservation gap remains recorded.

Do not mark this repair physically verified until the new EAPOL transfer is observed. Independent SD readback is a separate evidence check: compare ordered packet payloads, counts and source identities rather than requiring whole PCAP hashes to match after timestamp wrapping. Preserve each original file and its hash.

## Test 1: Timing variation and duplicate delivery

**What gets harder:** the source no longer presents evenly spaced sightings, and each event summary is delivered three times; text replies stay intact. Source counts and source timestamps remain unchanged by duplicate transport. Repeated replies must respect ordinary request IDs and framing, such as replies to normal retries; malformed or unsolicited messages are not equivalent duplicates.

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

## Repaired baseline verification and T1 preparation — 2026-11-15

The operator reran the baseline on Vixen 141. The saved phone evidence contained
48 deauthentication frames and 24 EAPOL frames. Each EAPOL frame retained the
complete 95-byte EAPOL-Key body, rather than just an EtherType marker. The
closed PCAP and payload hashes matched the journal. The AI returned
`disconnect_sequence_then_remote_id` for the 30-second source-clock gap;
its result was current, the notification was posted, and monitoring continued.
The saved report completed 5,778 ms after Vixen first received the Remote ID
summary, within the 30-second analysis target.
This verifies phone-side preservation and normal AI integration for synthetic
observations. Independent source-SD parity and real RF detection were not tested.
The previous build's EAPOL preservation failure remains a historical failure.

T1 uses the separate `tdongle_c5_harness_t1` firmware target. The baseline target
remains available. Vixen 141 and the model stay unchanged. T1 is not yet a
physical-test pass.

The exact observation offsets (milliseconds from operator Start) are:

- Deauth: 20000, 21000, 22500, 24000, 25000, 26800, 28000, 29000.
- EAPOL: 37000, 38700, 41000, 43000.
- Remote ID: 82000, 83500, 85700, 87000.

Each normal event-summary reply is delivered three times, with the same event
identity, observation count, source timestamps and request ID. END reports the
number of summary frames delivered. Packet transport is unchanged. Text replies
remain intact: the original plan to duplicate text fragments was narrowed
because repeating fragments inside one reply changes the reconstructed text.
This run does not claim to test repeated complete text transactions.

Use a battery after programming. Connect through Vixen's normal interface;
use normal Stop then Start, or the dongle's physical button, to begin a fresh
run. Leave the same sensors enabled. Expect the final stage from 82 seconds,
then allow 30 seconds for analysis. Only the operator starts the run.
The fixture ends at 95 seconds but monitoring continues until operator Stop.
Check the 62-second relationship, counts 8/4/4, preserved 48/24 packet windows,
and absence of duplicate notifications for identical completed input.

Computer checks passed for both baseline and T1: ordinary commands, source
timing, unchanged counts, complete 48/24 packet windows, repeated summaries
with matching END count, intact text replies, and continued monitoring. These
checks do not substitute for the pending operator-run T1 on the phone.


## T1 result and T2 source preparation — 2026-11-15

The operator's T1 run passed on frozen Vixen 141: source counts 8/4/4, 48 deauth
and 24 complete EAPOL-Key frames preserved, correct 62-second relationship,
one accepted analysis and notification, and continued monitoring. The report
completed 2,500 ms after receipt of the first required drone observation.
The comparison was read-only and did not operate either device.

T2 is prepared as the separate `tdongle_c5_harness_t2` target. It inherits T1's
summary duplication, keeps text and packet transport unchanged, and adds:

- Network B: eight deauth observations at 55, 56, 57, 58, 59, 60, 61 and 62 seconds.
- Network C: four EAPOL observations at 65, 67, 69 and 71 seconds.

The original network A still has eight deauth observations from 20 to 29 seconds
and four EAPOL observations from 37 to 43 seconds. Drone observations remain at
82, 83.5, 85.7 and 87 seconds. A, B and C have distinct per-run source addresses;
only A has both kinds of Wi-Fi observation. Expected event counts in source
order: 8/4/8/4/4. Four packet windows must preserve 48/24/48/24 frames, respectively.
The complete drone service-data payload must remain intact.

The phone is now Vixen 149, after the separately approved product UI work. The
T1 outcome remains a build-141 result. T2 freezes the installed Vixen 149 APK and
existing model: no application, prompt, model, sensor-setting or inference
changes are part of this preparation. Its installed APK hash is recorded in
the local preparation ledger. T2's expected positive interpretation is still
the valid A sequence with a 62-second source-clock gap and one notification;
B's disconnects must not be paired with C's handshakes. This is an acceptance
criterion, not a claim that the frozen product already passes.

The physical T2 test is not yet run. Only the operator starts it through normal
Start/Stop or the physical button. The harness stays idle after programming;
completion of the observations at 95 seconds does not stop monitoring. No SD
format, deletion or original-Hunter update is part of this preparation.

T2 preparation completed: all three host profiles passed, the prototype app
firmware was hash-verified after upload, and read-only INFO confirmed
`T2-separate-streams`, idle, zero events, SD ready, zero write errors and LED off.
The prototype was disconnected from BLE at that check; the operator connects
through Vixen before pressing Start. No physical scenario was started.


## T2 operator result — FAIL, 2026-11-15

Frozen Vixen 149 received all five source events in one Hunter session, with
counts 8/4/8/4/4. The four closed native packet windows were pulled read-only
from saved phone files and verified against their journal SHA-256 hashes:
48 A deauth, 24 A complete EAPOL-Key, 48 B deauth and 24 C complete EAPOL-Key
frames. Their source-address suffixes matched A/A/B/C. The drone service data
and source-clock first sightings were saved; the valid A-to-drone gap was
62,000 ms. Monitoring was still active after the expected report deadline.

No signature-analysis journal entry and no saved assessment existed for this
session when checked after the operator reported 2 minutes 30 seconds without
a report. This fails T2's required interpretation and notification. Packet
preservation passed; the end-to-end test did not.

`LiveSignatureInput.prepare` first selects the newest deauth record within a
producer/session, then searches only that record's network for EAPOL. In this
run the newer deauth is B, which correctly has no matching EAPOL. The selector
therefore returns no prepared input instead of considering the older valid A
pair. C's EAPOL remains separate, but the A pair is hidden. This is a procedural
candidate-selection failure before inference, not an observed wrong answer
from the model. Missing analysis must not be described as a negative finding.

The frozen app and harness were not operated or modified during comparison.
Original journal, native captures, hashes and the failed outcome are preserved
in the local T2 operator ledger. No corrected result replaces this failure.


## T2 operator rerun — PASS on build 150, 2026-11-15

The first start after installing 150 was stopped after roughly 3.4 seconds,
before the first 20-second observation. It is recorded as an aborted attempt,
not an AI failure or a passing run. The operator restarted without assistant
device control. The subsequent complete run passed.

All five source records arrived with counts 8/4/8/4/4. The accepted assessment
selected events 1 and 2 from network A and drone event 5; unrelated B and C
records 3 and 4 were retained separately. The model returned
`disconnect_sequence_then_remote_id`; the saved report correctly stated the
62-second source-clock gap. Completion was 7,509 ms after first drone-summary
receipt, within the 30-second target. The journal recorded one accepted AI
analysis and one posted notification despite repeated summaries. Monitoring
remained active after fixture completion.

Four closed PCAPs were pulled from saved phone files and matched their journal
SHA-256 hashes. They held 48/24/48/24 frames with matching A/A/B/C source MACs,
zero dropped/truncated frames, and complete 95-byte EAPOL-Key bodies. The drone
service-data payload and full input/raw result were retained. Comparison used
read-only app-file access; neither device was operated by the observer.

The original frozen-149 failure remains above and in its separate evidence
ledger. This rerun used the same harness firmware and model with Vixen 150's
procedural selection repair. It verifies this live synthetic BLE scenario;
independent source SD parity and physical RF detection were not tested here.


### Remaining recommendation gap

The operator noted after the successful T2 correlation rerun that build 150's
report contains no action recommendation. This remains an outstanding MVP report
requirement. The T2 acceptance result covers its defined correlation, packet
preservation, duplicate handling, notification and latency checks; it does not
establish a complete actionable MVP report or validate the recommendation
proposed in conversation. No recommendation change was made for this run.
