// Ordinary Hunter card protocol adapter, reused from C5App.cpp.
link::card::Streamer gCardLink;
C5Link::EvidenceSend gCardLinkSend = nullptr;
void *gCardLinkSendContext = nullptr;
bool gCardLinkUsedBus = false;
uint32_t gCardLinkFiles = 0;

// Clear Card (2026-11-06): the request that started the clear, answered when
// it ends. One clear at a time; reads and pulls are refused while it runs.
uint8_t gCardClearReqId = 0;
C5Link::EvidenceSend gCardClearSend = nullptr;
void *gCardClearSendContext = nullptr;

bool cardLinkActive() { return gCardLink.state() == link::card::Streamer::State::Running; }

void sendCardResult(uint8_t reqId, uint8_t op, CommandResult r, C5Link::EvidenceSend send, void *ctx)
{
    uint8_t out[link::kMaxFrame];
    link::Result res;
    res.reqId = reqId;
    res.commandType = op;
    res.commandResult = static_cast<uint8_t>(r);
    send(out, link::encodeResult(res, out), ctx);
}

struct CardListContext {
    uint8_t reqId;
    C5Link::EvidenceSend send;
    void *sendContext;
    uint8_t count;
    bool ok;
};
void cardListToLink(const char *name, uint32_t size, void *context)
{
    CardListContext *c = static_cast<CardListContext *>(context);
    link::card::FileId id;
    if (!c->ok || c->count == 255 || !link::card::fileIdFromName(name, id)) return;
    uint8_t out[link::card::kCardEntrySize];
    c->ok = c->send(out, link::card::encodeCardEntry(c->reqId, id, size, out), c->sendContext);
    if (c->ok) ++c->count;
}

size_t cardLinkRead(uint8_t *out, size_t max, void *)
{
    bool bus = false;
    const size_t n = gSdLog.readChunk(out, max, bus);
    gCardLinkUsedBus = gCardLinkUsedBus || bus;
    return n;
}

void answerCardRequest(const uint8_t *request, size_t len, size_t frameSize, C5Link::EvidenceSend send, void *sendContext)
{
    const uint8_t reqId = len >= 2 ? request[1] : 0;
    const bool busy = cardLinkActive() || gSdLog.reading();
    if (request[0] == link::card::kOpCardList) {
        if (len != link::card::kCardListSize) {
            uint8_t out[link::kMaxFrame];
            link::Error e;
            e.reqId = reqId;
            e.status = link::LinkStatus::BadLength;
            send(out, link::encodeError(e, out), sendContext);
            return;
        }
        if (busy || !gSdLog.hasCardLog()) {
            sendCardResult(reqId, request[0], CommandResult::NotReady, send, sendContext);
            return;
        }
        CardListContext c{reqId, send, sendContext, 0, true};
        bool usedBus = false;
        const bool listed = gSdLog.listCard(cardListToLink, &c, usedBus);
        if (usedBus) applyLed();
        if (!listed) {
            sendCardResult(reqId, request[0], CommandResult::NotReady, send, sendContext);
            return;
        }
        if (!c.ok) return; // the connection went
        uint8_t out[link::kMaxFrame];
        link::End e;
        e.reqId = reqId;
        e.count = c.count;
        send(out, link::encodeEnd(e, out), sendContext);
        return;
    }
    if (request[0] == link::card::kOpCardClear) {
        // Clear Card (Michael, 2026-11-06): erases the card completely, every
        // file and folder. LayerScope has already asked the operator to
        // confirm. Refused while any card read runs.
        if (len != link::card::kCardClearSize) {
            uint8_t out[link::kMaxFrame];
            link::Error e;
            e.reqId = reqId;
            e.status = link::LinkStatus::BadLength;
            send(out, link::encodeError(e, out), sendContext);
            return;
        }
        if (busy || !gSdLog.hasCardLog()) {
            sendCardResult(reqId, request[0], CommandResult::NotReady, send, sendContext);
            return;
        }
        bool usedBus = false;
        const bool started = gSdLog.beginClear(usedBus);
        if (usedBus) applyLed();
        if (!started) {
            sendCardResult(reqId, request[0], CommandResult::NotReady, send, sendContext);
            return;
        }
        // serviceCardClear() erases a few files per loop pass and answers at the end.
        gCardClearReqId = reqId;
        gCardClearSend = send;
        gCardClearSendContext = sendContext;
        Serial.printf("Card clear over the Link started at %lu ms\n", static_cast<unsigned long>(millis()));
        return;
    }
    link::card::FileId id;
    uint32_t offset = 0;
    uint8_t rid = 0;
    const link::card::ReadCheck check = link::card::decodeCardRead(request, len, rid, id, offset);
    if (check == link::card::ReadCheck::BadLength) {
        uint8_t out[link::kMaxFrame];
        link::Error e;
        e.reqId = reqId;
        e.status = link::LinkStatus::BadLength;
        send(out, link::encodeError(e, out), sendContext);
        return;
    }
    if (check == link::card::ReadCheck::BadFile) {
        sendCardResult(reqId, request[0], CommandResult::InvalidArgument, send, sendContext);
        return;
    }
    if (busy || !gSdLog.hasCardLog()) {
        sendCardResult(reqId, request[0], CommandResult::NotReady, send, sendContext);
        return;
    }
    char name[link::card::kCardNameSize];
    uint32_t size = 0;
    bool usedBus = false;
    if (!link::card::fileName(id, name, sizeof(name)) || !gSdLog.beginRead(name, size, usedBus)) {
        if (usedBus) applyLed();
        sendCardResult(reqId, request[0], CommandResult::InvalidArgument, send, sendContext);
        return;
    }
    if (usedBus) applyLed();
    gCardLinkSend = send;
    gCardLinkSendContext = sendContext;
    gCardLink.start(reqId, size, offset, frameSize);
    ++gCardLinkFiles;
}

// Once per loop pass while a clear runs: erase a few more files.
void serviceCardClear()
{
    if (!gSdLog.clearing()) return;
    bool usedBus = false;
    const uint32_t now = millis();
    const C5SdLog::ClearStep s =
        gSdLog.stepClear(now, 1, "harness", usedBus);
    if (usedBus) applyLed();
    if (s == C5SdLog::ClearStep::Running || s == C5SdLog::ClearStep::Idle) return;
    gServer.setMemoryOnly(!gSdLog.hasCardLog());
    const uint32_t erased = gSdLog.clearErased();
    Serial.printf("Card clear over the Link %s: %lu files erased, now logging to %s at %lu ms\n",
                  s == C5SdLog::ClearStep::Done ? "done" : "ended with files that would not erase",
                  static_cast<unsigned long>(erased),
                  gSdLog.hasCardLog() ? gSdLog.counters().fileName : "nothing (no card log)",
                  static_cast<unsigned long>(now));
    for (uint8_t i = 0; gSdLog.clearLeftover(i) != nullptr; ++i)
        Serial.printf("Card clear could not erase %s\n", gSdLog.clearLeftover(i));
    if (gCardClearSend == nullptr) return;
    if (s == C5SdLog::ClearStep::Done) {
        uint8_t out[link::kMaxFrame];
        link::End e;
        e.reqId = gCardClearReqId;
        e.count = static_cast<uint8_t>(erased > 255 ? 255 : erased);
        gCardClearSend(out, link::encodeEnd(e, out), gCardClearSendContext);
    } else {
        sendCardResult(gCardClearReqId, link::card::kOpCardClear, CommandResult::NotReady, gCardClearSend,
                       gCardClearSendContext);
    }
    gCardClearSend = nullptr;
}

// Once per loop pass: the next few frames of a card read, then close the file.
void serviceCardLink()
{
    if (!cardLinkActive()) return;
    gCardLinkUsedBus = false;
    const link::card::Streamer::State s = gCardLink.step(cardLinkRead, nullptr, gCardLinkSend, gCardLinkSendContext, 8);
    if (s != link::card::Streamer::State::Running) {
        bool bus = false;
        gSdLog.endRead(bus);
        gCardLinkUsedBus = gCardLinkUsedBus || bus;
        if (s == link::card::Streamer::State::Failed)
            Serial.printf("card read over the Link stopped after %lu bytes (connection gone or card error)\n",
                          static_cast<unsigned long>(gCardLink.sent()));
    }
    if (gCardLinkUsedBus) applyLed();
}

