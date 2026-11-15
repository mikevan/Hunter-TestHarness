#if defined(LAYERTIME_HARNESS_APP)
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <esp_random.h>
#include <esp_heap_caps.h>
#include "C5Link.h"
#include "C5Display.h"
#include "C5Led.h"
#include "C5SdLog.h"
#include "TDongleC5Pins.h"
#include "BringUpLogic.h"
#include "HarnessMonitor.h"
#include "HarnessEvidence.h"
#include "core/app/LayerTimeCore.h"
#include "core/link/LinkServer.h"
#include "core/link/LinkEvidence.h"
#include "core/logic/EvidencePcap.h"
using namespace layertime;
using namespace layertime::tdongle_c5;
namespace {
C5Link gLink;
C5Display gDisplay;
C5Led gLed;
C5SdLog gSdLog;
harness::Monitor gMonitor;
LayerTimeCore gCore;
link::LinkServer gServer(gCore);
evidence::EvidenceStore gEvidence;
ButtonDebouncer gButton;
uint8_t *gPcap = nullptr;
uint32_t gLoggedRun = 0, gLastDisplay = 0, gTransfers = 0;
bool gWasConnected = false, gWasRunning = false, gWasComplete = false;
String gSerial;
// Current LilyGO C5 boards wire APA102 clock/data to GPIO 4/5, separate
// from LCD/SD SPI (6/2). Keep the legacy shared-bus off command as well.
// Pin map: https://github.com/Xinyuan-LilyGO/T-Dongle-C5/blob/master/readme.md
void offDedicatedLed() {
    constexpr int clock = 4, data = 5;
    pinMode(clock, OUTPUT); pinMode(data, OUTPUT);
    uint8_t frame[kApa102FrameBytes]; encodeApa102(Rgb{0,0,0}, 0, frame);
    for (uint8_t byte : frame) for (int bit=7; bit>=0; --bit) {
        digitalWrite(clock, LOW); digitalWrite(data, (byte >> bit) & 1);
        delayMicroseconds(1); digitalWrite(clock, HIGH); delayMicroseconds(1);
    }
    digitalWrite(clock, LOW); digitalWrite(data, LOW);
}
void applyLed() { gLed.off(); offDedicatedLed(); }
void statusLine(uint8_t line, const char *text) {
    static char previous[C5Display::kLineCount][64] = {};
    if (strncmp(previous[line], text, sizeof(previous[line])) == 0) return;
    snprintf(previous[line], sizeof(previous[line]), "%s", text);
    gDisplay.setLine(line, text);
}

// Seed valid 802.11 deauthentication frames into the normal evidence store.
// This never transmits Wi-Fi or BLE advertisements representing a real tracker.
void prepareObservation(const recon::Candidate &c, void *) {
    harness::prepareEvidence(gEvidence, c);
}
class Recorder : public EventLog {
public:
    void append(const MonitorEvent &e) override {
        gSdLog.append(e);
        uint8_t mac[6];
        if (e.sourceKind == SourceKind::Wifi && evidence::EvidenceStore::parseMac(e.sourceId, mac))
            gEvidence.openWindow(e.eventId, mac, e.lastSeen.uptimeMs,
                                 evidence::EvidenceStore::followMsFor(gCore.settings().evidenceHoldSec));
    }
    void repeated(const MonitorEvent &e) override { gSdLog.repeated(e); }
} gRecorder;

#include "HarnessCard.inc"

void answerEvidence(const uint8_t *request, size_t len, uint32_t now, size_t frameSize,
                    C5Link::EvidenceSend send, void *sendContext, void *) {
    if (link::card::isCardOp(request[0])) { answerCardRequest(request, len, frameSize, send, sendContext); return; }
    const uint32_t id = link::evidence::requestEventId(request, len);
    bool known = false; uint8_t kind = 0;
    for (uint8_t i = 0; i < gCore.eventCount(); ++i) {
        if (gCore.event(i).eventId == id) { known = true; kind = static_cast<uint8_t>(gCore.event(i).sourceKind); break; }
    }
    link::evidence::Access access; access.store = &gEvidence;
    if (request[0] == link::evidence::kOpGetFrames)
        link::evidence::answerGetFrames(access, request, len, known, kind, now, send, sendContext);
    else link::evidence::answerGetFrameBytes(access, request, len, known, frameSize, send, sendContext);
}
void saveWindows(uint32_t now) {
    if (!gPcap || !gSdLog.hasCardLog() || gSdLog.reading()) return;
    uint32_t id;
    if (!gEvidence.nextToSave(now, id)) return;
    evidence::WindowInfo info;
    const size_t size = evidence::buildWindowPcap(gEvidence, id, now, gPcap, evidence::kMaxWindowPcapBytes, info);
    SdEvidenceCounts counts;
    counts.seen = info.seen; counts.kept = info.retained; counts.dropped = info.dropped;
    counts.truncated = info.truncated; counts.collecting = info.collecting;
    bool bus = false;
    if (size && gSdLog.saveEvidence(id, gPcap, size, counts, now, 1, bus)) gEvidence.markSaved(id, info.seen);
    if (bus) applyLed();
}
void noteRun(uint32_t now) {
    if (gMonitor.runId() == gLoggedRun || !gMonitor.running()) return;
    gLoggedRun = gMonitor.runId();
    char mode[40]; snprintf(mode, sizeof(mode), "FIXTURE run=%08lx", static_cast<unsigned long>(gLoggedRun));
    gSdLog.noteMode(mode, now);
    // Use the normal event-log queue: the SD driver allows one open file,
    // so a second sidecar must not be opened alongside the active log.
    gSdLog.noteMode(harness::Monitor::profile(), now);
    gSdLog.noteMode("synthetic; no RF transmission", now);
    Serial.printf("RUN id=%lu profile=%s deauth=%lu EAPOL=%lu RemoteID=%lu ms\n", static_cast<unsigned long>(gLoggedRun), harness::Monitor::profile(), static_cast<unsigned long>(harness::Monitor::offset(0)), static_cast<unsigned long>(harness::Monitor::offset(8)), static_cast<unsigned long>(harness::Monitor::offset(harness::Monitor::observationCount()-4)));
}
void showStatus(uint32_t now) {
    if (now - gLastDisplay < 500) return;
    gLastDisplay = now;
    gDisplay.setIndicators(gLink.connected() ? LinkIndicator::Connected : LinkIndicator::NoWatch, MeshIndicator::NotConnected);
    statusLine(0, "Hunter-Test");
    statusLine(1, gLink.connected() ? "Phone connected" : "Connect Vixen");
    char state[40];
    if (gMonitor.running() && gMonitor.complete()) snprintf(state, sizeof(state), "Data done; monitoring");
    else if (gMonitor.running()) snprintf(state, sizeof(state), "Scenario running");
    else snprintf(state, sizeof(state), "%s", gMonitor.complete() ? "Stopped / data done" : "Ready / stopped");
    statusLine(2, state);
    snprintf(state, sizeof(state), "%u events / %s", gCore.eventCount(), gSdLog.hasCardLog() ? "SD ready" : "NO SD LOG");
    statusLine(3, state);
    statusLine(4, gMonitor.running() ? "BOOT or phone: Stop" : "BOOT or phone: Start");
}
}
void setup() {
    Serial.begin(115200); Serial.setTxTimeoutMs(2);
    C5SdLog::holdCardDeselected(); pinMode(pins::kBootButton, INPUT);
    offDedicatedLed();
    gDisplay.setIndicators(LinkIndicator::NoWatch, MeshIndicator::NotConnected);
    gDisplay.begin(); gLed.begin(); applyLed();
    gSdLog.begin(); gSdLog.service(millis(), 1, "harness", false); applyLed();
    void *pool = heap_caps_malloc(evidence::EvidenceStore::poolBytes(), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (pool) gEvidence.attach(pool, evidence::EvidenceStore::poolBytes());
    gPcap = static_cast<uint8_t *>(heap_caps_malloc(evidence::kMaxWindowPcapBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    gMonitor.seed(esp_random()); gMonitor.beforeCandidate(prepareObservation, nullptr);
    CorePorts ports; ports.monitor = &gMonitor; ports.eventLog = &gRecorder; gCore.attach(ports);
    gLink.attachServer(&gServer); gLink.attachEvidence(answerEvidence, nullptr);
    uint16_t caps = link::kCapLocalWifiMonitor | link::kCapLocalBleMonitor | link::kCapDisplay |
                    link::kCapButton | link::kCapRemoteId | link::kCapObservationTime | link::kCapSensorSet | link::kCapEvidenceHold |
                    link::card::kCapCardRead | link::card::kCapCardClear;
    if (gEvidence.attached()) caps |= link::evidence::kCapRawCapture;
    gServer.setMemoryOnly(!gSdLog.hasCardLog()); gServer.refresh();
    const bool ok = gLink.begin(caps); gLink.updateStatus();
    Serial.printf("Hunter-TestHarness v6 normal-protocol name=%s session=%u ble=%u sd=%u evidence=%u led=off\n",
                  gLink.advertisedName(), gLink.sessionId(), ok, gSdLog.hasCardLog(), gEvidence.attached());
}
void loop() {
    const uint32_t now = millis(); gMonitor.setNow(now);
    if (gButton.update(digitalRead(pins::kBootButton) == LOW, now)) {
        LayerTimeCommand cmd;
        if (gMonitor.running()) cmd.type = CommandType::ReconStop;
        else { cmd.type = CommandType::ReconStart; cmd.reconTarget.target = ReconTarget::All; }
        gCore.execute(cmd);
    }
    gLink.serviceRequests(now); // ordinary COMMAND/GET_CHANGED/GET_TEXT
    gMonitor.setEarlyWarningEnabled(gCore.settings().earlyWarningEnabled);
    noteRun(now); gCore.tick(now);
    const bool changed = gWasConnected != gLink.connected();
    if (changed) gSdLog.noteLink(gLink.connected(), gLink.peerAddress(), now, gLink.sessionId());
    if (!gWasComplete && gMonitor.complete()) gSdLog.noteMode("fixture-complete-monitoring", now);
    if (gWasRunning && !gMonitor.running()) gSdLog.noteMode("monitoring-stopped", now);
    gWasComplete = gMonitor.complete();
    gWasRunning = gMonitor.running(); gWasConnected = gLink.connected();
    if (gSdLog.service(now, 1, "harness", changed)) applyLed();
    serviceCardLink(); serviceCardClear(); saveWindows(now);
    gServer.setMemoryOnly(!gSdLog.hasCardLog()); gServer.refresh(); gLink.updateStatus(); gLink.service(now);
    showStatus(now); gDisplay.service();
    if (gDisplay.transfers() != gTransfers) { applyLed(); gTransfers = gDisplay.transfers(); }
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            gSerial.trim();
            if (gSerial == "INFO") Serial.printf("HARNESS_INFO v6 profile=%s normal-protocol connected=%u running=%u fixtureComplete=%u run=%lu elapsed=%lu events=%u sd=%u file=%s lines=%lu writeErrors=%lu dropped=%lu evidenceFiles=%lu requests=%lu lastCommand=%u result=%u led=off\n",
                harness::Monitor::profile(), gLink.connected(), gMonitor.running(), gMonitor.complete(), static_cast<unsigned long>(gMonitor.runId()), static_cast<unsigned long>(gMonitor.elapsedMs()),
                gCore.eventCount(), gSdLog.hasCardLog(), gSdLog.counters().fileName, static_cast<unsigned long>(gSdLog.counters().lines),
                static_cast<unsigned long>(gSdLog.counters().writeErrors), static_cast<unsigned long>(gSdLog.dropped()),
                static_cast<unsigned long>(gSdLog.counters().evidenceFiles), static_cast<unsigned long>(gLink.requestsAnswered()),
                gServer.counters().lastCommandType, gServer.counters().lastCommandResult);
            gSerial = "";
        } else if (gSerial.length() < 80) gSerial += c;
    }
    delay(5);
}
#endif
