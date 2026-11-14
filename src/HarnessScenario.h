#pragma once
#include <stdint.h>
namespace harness {
constexpr uint16_t kCapability=0x8000;
constexpr uint8_t kFrameType=0xe0;
inline uint8_t stage(uint32_t elapsed) {
    return elapsed<10000?1:elapsed<20000?2:elapsed<50000?3:elapsed<60000?4:5;
}
inline void put16(uint8_t *b,uint16_t n) { b[0]=n&255;b[1]=n>>8; }
inline void put32(uint8_t *b,uint32_t n) { for(int i=0;i<4;i++)b[i]=(n>>(8*i))&255; }
inline void frame(uint8_t *b,uint32_t run,uint32_t elapsed,uint16_t sequence) {
    b[0]=kFrameType;b[1]=1;b[2]=stage(elapsed);b[3]=3;
    put32(b+4,run);put32(b+8,elapsed);put16(b+12,sequence);
    put16(b+14,b[2]==1?120:0);put16(b+16,b[2]==1?80:0);put16(b+18,b[2]>=4?1:0);
}
}
