#ifndef __CLICK_TCP_PACKET
#define __CLICK_TCP_PACKET
#include <click/config.h>
CLICK_DECLS

enum TcpFlag {
    Ack = 0x08,
    Rst = 0x20,
    Syn = 0x40,
    Fin = 0x80,    
};

struct TcpPacket {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t sequence;
    uint32_t acknowledge;
    uint16_t flags;
    uint16_t window;
    // checksum not implemented
    uint32_t zero;
    char data[0];
};

CLICK_ENDDECLS
#endif
