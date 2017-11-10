#ifndef __CLICK_ISP_PACKET
#define __CLICK_ISP_PACKET
#include <click/config.h>
#include <click/integers.hh>
CLICK_DECLS

enum LspType {
    LspAck = 0,
    LspHello = 1,
    LspSequence = 2,
};

const uint16_t LspMagic = 0x35fc;

struct LspSequenceData {
    uint32_t sequence;
    uint32_t count;
    uint32_t entry[0];
};

struct LspHeader {
    uint16_t magic;
    uint16_t type;
    LspSequenceData data[0];

    void Init(LspType);
};

const size_t LspSizeShort = sizeof(LspHeader);
const size_t LspSize = LspSizeShort + sizeof(LspSequenceData);

inline void LspHeader::Init(LspType t) {
    magic = LspMagic;
    type = t;
}

struct LspRoutingEntry {
    uint32_t ip;
    int32_t port;
};

struct LspRouting {
    uint32_t count;
    LspRoutingEntry entry[0];
};

const size_t LspSizeRouting = sizeof(LspRouting);
const size_t LspSizeRoutingEntry = sizeof(LspSizeRoutingEntry);

CLICK_ENDDECLS
#endif
