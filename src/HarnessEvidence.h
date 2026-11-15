#pragma once
#include <string.h>
#include "core/logic/ReconCandidate.h"
#include "core/logic/EvidenceStore.h"
namespace harness {
inline void prepareEvidence(layertime::evidence::EvidenceStore &store, const layertime::recon::Candidate &c) {
    if (c.sourceKind != layertime::SourceKind::Wifi) return;
    uint8_t mac[6];
    if (!layertime::evidence::EvidenceStore::parseMac(c.address, mac)) return;
    uint8_t frame[26] = {0xC0, 0x00};
    memset(frame + 4, 0xFF, 6); memcpy(frame + 10, mac, 6); memcpy(frame + 16, mac, 6);
    frame[24] = 7;
    for (unsigned i = 0; i < 6; ++i) {
        frame[22] = static_cast<uint8_t>(i << 4);
        store.noteFrame(frame, sizeof(frame), c.rssi, c.channel, c.atMs - 250 + i * 50);
    }
}
} // namespace harness
