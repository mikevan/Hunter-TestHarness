#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include "HarnessMonitor.h"
#include "HarnessEvidence.h"
#include "core/app/LayerTimeCore.h"
#include "core/link/LinkServer.h"
#include "core/link/LinkEvidence.h"
#include "core/logic/EvidencePcap.h"
using namespace layertime;
struct Frames {
    std::vector<std::vector<uint8_t>> rows;
    static void add(const uint8_t *b, size_t n, void *p) { static_cast<Frames *>(p)->rows.emplace_back(b, b+n); }
    static bool send(const uint8_t *b, size_t n, void *p) { add(b,n,p); return true; }
};
struct Fixture : EventLog {
    harness::Monitor monitor; LayerTimeCore core; link::LinkServer server;
    evidence::EvidenceStore store; std::vector<uint8_t> pool; unsigned saved=0;
    Fixture():server(core),pool(evidence::EvidenceStore::poolBytes()) {
        assert(store.attach(pool.data(),pool.size()));
        monitor.seed(100); monitor.setNow(1000); monitor.beforeCandidate(prepare,this);
        CorePorts ports; ports.monitor=&monitor; ports.eventLog=this; core.attach(ports); server.refresh();
    }
    static void prepare(const recon::Candidate &c,void *p) { harness::prepareEvidence(static_cast<Fixture *>(p)->store,c); }
    void append(const MonitorEvent &e) override {
        ++saved; uint8_t mac[6];
        if(e.sourceKind==SourceKind::Wifi && evidence::EvidenceStore::parseMac(e.sourceId,mac))
            assert(store.openWindow(e.eventId,mac,e.lastSeen.uptimeMs,10000));
    }
    void advance(uint32_t now) {monitor.setNow(now);core.tick(now);server.refresh();}
    Frames request(const std::vector<uint8_t> &req,uint32_t now=1000) {
        Frames out;link::NodeIdentity node;node.sessionId=1;
        server.handle(node,0,req.data(),req.size(),now,Frames::add,&out);return out;
    }
    void command(uint8_t kind,int arg=-1,CommandResult expected=CommandResult::Ok) {
        std::vector<uint8_t> req={3,7,kind};if(arg>=0)req.push_back(static_cast<uint8_t>(arg));
        auto out=request(req);assert(out.rows.size()==1);link::Result result;
        assert(link::decodeResult(out.rows[0].data(),out.rows[0].size(),result));
        assert(result.reqId==7 && result.commandType==kind && result.commandResult==static_cast<uint8_t>(expected));
    }
};
int main() {
    // Normal command/result and status, with idempotent Start and real event/text replies.
    Fixture f;f.command(1,1);const auto run=f.monitor.runId();f.advance(11000);f.command(1,1);
    assert(f.monitor.runId()==run && f.core.eventCount()==0);
    f.advance(21000);assert(f.core.eventCount()==1 && f.core.event(0).detector==ReconTarget::Deauth);
    f.advance(28000);assert(f.core.event(0).count==8 && f.saved==1);
    auto out=f.request({4,9,0,0,0,0},28000);assert(out.rows.size()==2);
    link::EventSummary summary;assert(link::decodeEventSummary(out.rows[0].data(),out.rows[0].size(),summary));
    assert(summary.detector==5 && summary.count==8 && summary.eventId==1);
    auto text=f.request({5,10,1,0,0,0,0},28000);std::string address;
    for(const auto &row:text.rows)if(row[0]==0x86){link::Text chunk;assert(link::decodeText(row.data(),row.size(),chunk));address.append(reinterpret_cast<const char *>(chunk.bytes),chunk.length);}
    assert(address==f.core.event(0).sourceId);
    // Raw packet evidence uses the standard Link 0.2 store and byte stream.
    evidence::WindowInfo info;assert(f.store.info(1,32000,info));assert(info.retained==48 && !info.collecting);
    uint8_t req[9];link::evidence::Access access;access.store=&f.store;Frames frames;
    auto len=link::evidence::encodeGetFrames(8,1,req);
    link::evidence::answerGetFrames(access,req,len,true,1,32000,Frames::send,&frames);
    link::evidence::FrameWindow window;assert(link::evidence::decodeFrameWindow(frames.rows[0].data(),frames.rows[0].size(),window));assert(window.retained==48);
    frames.rows.clear();len=link::evidence::encodeGetFrameBytes(9,1,0,0,req);
    link::evidence::answerGetFrameBytes(access,req,len,true,20,Frames::send,&frames);
    assert(!frames.rows.empty() && frames.rows.front()[0]==0x89);
    std::vector<uint8_t> pcap(evidence::kMaxWindowPcapBytes);assert(evidence::buildWindowPcap(f.store,1,32000,pcap.data(),pcap.size(),info)>24);
    f.advance(51000);assert(f.core.eventCount()==2 && f.core.event(1).detector==ReconTarget::AirTag);
    f.advance(62000);assert(!f.monitor.running() && f.monitor.complete() && f.core.event(1).count==4);
    f.server.reset();f.server.refresh();auto reconnect=f.request({4,11,0,0,0,0},62000);assert(reconnect.rows.size()==3 && f.core.eventCount()==2);
    f.command(1,1);assert(f.monitor.runId()!=run);f.advance(82000);assert(f.core.eventCount()==3 && f.core.event(2).eventId==3);
    f.command(2);assert(f.core.eventCount()==3);f.advance(132000);assert(f.core.eventCount()==3);
    f.command(4);f.command(3);assert(f.core.eventCount()==0);f.command(1,1);f.advance(152000);assert(f.core.event(0).eventId>3);
    // Selected sensors are filtered by the actual core command path.
    Fixture masked;uint32_t mask=1u<<5;
    auto result=masked.request({3,12,15,uint8_t(mask),uint8_t(mask>>8),uint8_t(mask>>16),uint8_t(mask>>24)});
    assert(result.rows[0][0]==0x83 && result.rows[0][4]==0);
    masked.command(1,1);masked.advance(62000);assert(masked.core.eventCount()==1 && masked.core.event(0).detector==ReconTarget::Deauth);
    auto invalid=masked.request({3,12,15,255,255,255,255});assert(invalid.rows[0][4]==2);
    masked.command(16,30);masked.command(16,31,CommandResult::InvalidArgument);
    Fixture selected;selected.command(1,10);selected.advance(62000);assert(selected.core.eventCount()==1 && selected.core.event(0).detector==ReconTarget::AirTag);
    // Stop cancels pending observations; source timing survives uint32 uptime wrap.
    Fixture stopped;stopped.command(1,1);stopped.advance(11000);stopped.command(2);stopped.advance(62000);assert(stopped.core.eventCount()==0);
    Fixture wrap;wrap.monitor.setNow(0xfffff000u);wrap.command(1,1);wrap.advance(uint32_t(0xfffff000u+61000u));assert(wrap.core.eventCount()==2 && wrap.monitor.complete());
    // Legacy vendor payload is not emitted by any standard response.
    for(const auto &row:reconnect.rows)assert(row[0]!=0xE0);
    puts("PASS: normal commands/status/events/text, timing, idempotent Start, Stop, sensor filters, reconnect retention, event identities, raw evidence and PCAP, uptime wrap");
}
