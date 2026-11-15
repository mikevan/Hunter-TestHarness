#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace harness {
// Repeat complete summary records only. Text chunk indices and bytes stay intact.
// END.count reports delivered summaries; event observation counts never change.
template<class Emit>
void reply(const uint8_t *frame, size_t len, bool changedReply, Emit emit) {
    if (changedReply && len && frame[0] == 0x84) {
        emit(frame,len); emit(frame,len); emit(frame,len);
    } else if (changedReply && len >= 9 && frame[0] == 0x85) {
        uint8_t end[20];
        if (len > sizeof(end)) return; // standard END is nine bytes
        memcpy(end,frame,len); end[3]=static_cast<uint8_t>(frame[3]*3);
        emit(end,len);
    } else emit(frame,len);
}
}
