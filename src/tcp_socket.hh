#ifndef __CLICK_TCP_SOCKET
#define __CLICK_TCP_SOCKET
#include <click/config.h>
#include <click/packet.hh>
#include <click/task.hh>
#include <queue>
CLICK_DECLS

enum TcpSocketMethod {
    Return, Data, New, Bind,
    Listen, Accept, Connect,
    Send, Recv, Close, Error,
};

enum TcpState {
    Nil, Closed, Listening,
    Syn_Rcvd, Syn_Sent,
    Established,
    Fin_Wait1, Fin_Wait2, Closing,
    Close_Wait,
};

struct TcpSocket {
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    TcpState state;
    std::queue<Packet *> listenWait;
    std::queue<Packet *> acceptWait;
};

CLICK_ENDDECLS
#endif
