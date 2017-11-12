#ifndef __CLICK_TCP_SOCKET
#define __CLICK_TCP_SOCKET
#include <click/config.h>
#include <click/task.hh>
CLICK_DECLS

enum TcpSocketMethod {
    Return, New, Bind,
    Listen, Accept, Connect,
    Send, Recv, Close,
};

enum TcpState {
    Nil, Closed, Listening,
    Syn_Rcvd, Syn_Sent,
    Established,
    Fin_Wait1, Fin_Wait2, Closing,
    Close_Wait,
};

struct TcpSocket {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t dst_ip;
    TcpState state;
};

CLICK_ENDDECLS
#endif
