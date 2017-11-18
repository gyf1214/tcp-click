#ifndef __CLICK_TCP_PACKET
#define __CLICK_TCP_PACKET
#include <click/config.h>
CLICK_DECLS

enum TcpFlag {
    Ack = 0x0800,
    Rst = 0x2000,
    Syn = 0x4000,
    Fin = 0x8000,
    TcpMagic = 0x0005,
};

struct TcpHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t sequence;
    uint32_t acknowledge;
    uint16_t flags;
    uint16_t window;
    // checksum not implemented
    uint32_t zero;
    char data[0];

    void init(uint16_t sport, uint16_t dport, uint16_t f);
    void syn(uint16_t sport, uint16_t dport, uint32_t seq);
    void ack(uint16_t sport, uint16_t dport, uint32_t ac, uint16_t win);
    void synack(uint16_t sport, uint16_t dport);
    void fin(uint16_t sport, uint16_t dport);
    void rst(uint16_t sport, uint16_t dport);
};

const size_t TcpSize = sizeof(TcpHeader);

inline void TcpHeader::init(uint16_t sport, uint16_t dport, uint16_t f) {
    src_port = htons(sport);
    dst_port = htons(dport);
    flags = f | TcpMagic;
    sequence = acknowledge = 0;
}

inline void TcpHeader::syn(uint16_t sport, uint16_t dport, uint32_t seq) {
    init(sport, dport, Syn);
    sequence = htonl(seq);
}

inline void TcpHeader::ack(uint16_t sport, uint16_t dport, uint32_t ac, uint16_t win) {
    init(sport, dport, Ack);
    acknowledge = htonl(ac);
    window = htons(win);
}

inline void TcpHeader::synack(uint16_t sport, uint16_t dport) {
    init(sport, dport, Syn | Ack);
}

inline void TcpHeader::fin(uint16_t sport, uint16_t dport) {
    init(sport, dport, Fin);
}

inline void TcpHeader::rst(uint16_t sport, uint16_t dport) {
    init(sport, dport, Rst);
}

CLICK_ENDDECLS
#endif
