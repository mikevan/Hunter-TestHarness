#pragma once
#include <string.h>
#include "core/logic/ReconCandidate.h"
#include "core/logic/EvidenceStore.h"
namespace harness {
inline void prepareEvidence(layertime::evidence::EvidenceStore &store, const layertime::recon::Candidate &c) {
    if (c.sourceKind != layertime::SourceKind::Wifi) return;
    uint8_t mac[6];
    if (!layertime::evidence::EvidenceStore::parseMac(c.address, mac)) return;
    uint8_t frame[131] = {0};
    const bool exchange=c.detector==layertime::ReconTarget::Eapol;
    frame[0]=exchange ? 0x08 : 0xc0; frame[1]=exchange ? 0x02 : 0;
    memset(frame+4,0xff,6);memcpy(frame+10,mac,6);memcpy(frame+16,mac,6);
    size_t length=26;
    if(exchange) {
        // Complete EAPOL-Key M1 body (95 bytes), observed AP -> station.
        const uint8_t llc[]={0xaa,0xaa,3,0,0,0,0x88,0x8e};memcpy(frame+24,llc,8);
        frame[32]=2;frame[33]=3;frame[34]=0;frame[35]=95;
        frame[36]=2;frame[37]=0;frame[38]=0x88;frame[39]=0;frame[40]=16;
        frame[48]=1; // replay counter; the first 32-byte nonce follows
        for(unsigned i=49;i<81;++i)frame[i]=static_cast<uint8_t>(i);
        length=sizeof(frame);
    } else frame[24]=7;
    for(unsigned i=0;i<6;++i) {
        frame[22]=static_cast<uint8_t>(i<<4);
        store.noteFrame(frame,length,c.rssi,c.channel,c.atMs-250+i*50);
    }
}
} // namespace harness
