#ifndef __CLICK_TCP_SOCKET
#define __CLICK_TCP_SOCKET
#include "infra_anno.hh"
#include <click/config.h>
#include <click/packet.hh>
#include <click/task.hh>
#include <click/deque.hh>
CLICK_DECLS

enum TcpSocketMethod {
    Return, Data, New, Free,
    Listen, Accept, Connect,
    Send, Recv, Close, Error,
};

enum TcpState {
    Nil, Closed, Listening,
    Syn_Rcvd, Syn_Sent,
    Established,
    Fin_Wait1, Fin_Wait2, Closing,
    Close_Wait, Last_Ack,
};

struct TcpSocket {
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    TcpState state;
    Deque<Packet *> listenWait;
    Deque<Packet *> acceptWait;
    Packet *connectWait;
    Packet *closeWait;
};

inline WritablePacket *SocketPacket(uint8_t method, uint8_t id, uint32_t sequence, size_t size = 0) {
    WritablePacket *p = Packet::make(size);
    p->set_anno_u8(SocketMethod, method);
    p->set_anno_u8(SocketId, id);
    p->set_anno_u32(SocketSequence, sequence);
    return p;
}

CLICK_ENDDECLS
#endif
