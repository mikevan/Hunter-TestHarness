#pragma once
#include <stdint.h>
#include <stdio.h>
#include "core/ports/MonitorSource.h"
#include "core/logic/ReconSelection.h"

namespace harness {
// Controlled observations enter the same core sink as a real Hunter's monitor.
// No extra wire type, expected answer, outage claim, or drone label is emitted.
class Monitor : public layertime::MonitorSource {
public:
    using BeforeCandidate = void (*)(const layertime::recon::Candidate &, void *);
    void seed(uint32_t value) { if (!_everStarted) _run = value; }
    void setNow(uint32_t now) { _now = now; }
    void beforeCandidate(BeforeCandidate fn, void *context) { _before = fn; _beforeContext = context; }
    void start(layertime::ReconTarget target) override {
        if (target == layertime::ReconTarget::None) { stopManual(); return; }
        _selected = target;
        if (_running) return; // repeated Start is not a restart
        if (++_run == 0) ++_run;
        _started = _now; _next = 0; _running = true; _complete = false; _everStarted = true;
    }
    void stopManual() override { _running = false; _selected = layertime::ReconTarget::None; }
    void resetDetectorState() override {} // core owns event history; never reset the scenario clock here
    void setEarlyWarningEnabled(bool enabled) override { _early = enabled; }
    void setCandidateSink(layertime::recon::CandidateSink sink, void *context) override { _sink = sink; _context = context; }
    layertime::AcquisitionStatus acquisition() const override {
        layertime::AcquisitionStatus a;
        a.monitoring = _running; a.selected = _selected;
        a.active = _running ? _selected : layertime::ReconTarget::None;
        a.earlyWarningEnabled = _early; a.earlyWarningResting = _early && !_running;
        return a;
    }
    void poll() override {
        if (!_running) return;
        const uint32_t elapsed = _now - _started;
        while (_next < 12 && elapsed >= offset(_next)) {
            const bool wifi = _next < 8;
            layertime::recon::Candidate c;
            c.detector = wifi ? layertime::ReconTarget::Deauth : layertime::ReconTarget::AirTag;
            c.sourceKind = wifi ? layertime::SourceKind::Wifi : layertime::SourceKind::Ble;
            c.band = layertime::Band::Band2_4GHz;
            c.channel = wifi ? 6 : 0; c.rssi = wifi ? -48 : -55;
            c.confidence = layertime::Confidence::High;
            c.atMs = _started + offset(_next);
            char address[19];
            snprintf(address, sizeof(address), "02:%02X:%02X:%02X:%02X:%02X",
                     unsigned((_run >> 24) & 255), unsigned((_run >> 16) & 255),
                     unsigned((_run >> 8) & 255), unsigned(_run & 255), wifi ? 1 : 2);
            c.address = address;
            c.detail = wifi ? "Deauthentication burst" : "Find My advertisement";
            ++_next;
            if (!layertime::recon::wants(true, _selected, false, c.detector)) continue;
            if (_before) _before(c, _beforeContext);
            if (_sink) _sink(c, _context);
        }
        if (elapsed >= 61000) { _complete = true; stopManual(); }
    }
    uint32_t runId() const { return _run; }
    uint32_t elapsedMs() const { return _everStarted ? _now - _started : 0; }
    bool running() const { return _running; }
    bool complete() const { return _complete; }
    static uint32_t offset(uint8_t i) { return i < 8 ? 20000U + i * 1000U : 50000U + (i - 8) * 1000U; }
private:
    uint32_t _now = 0, _started = 0, _run = 0;
    uint8_t _next = 0;
    bool _running = false, _complete = false, _everStarted = false, _early = true;
    layertime::ReconTarget _selected = layertime::ReconTarget::None;
    layertime::recon::CandidateSink _sink = nullptr;
    void *_context = nullptr;
    BeforeCandidate _before = nullptr;
    void *_beforeContext = nullptr;
};
} // namespace harness
