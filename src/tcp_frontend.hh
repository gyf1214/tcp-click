// Tcp Frontend
// input(0) : incoming packets
// input(1) : incoming socket requests
// output(0) : tcp backend
// output(1) : socket response;
#ifndef __CLICK_TCP_FRONTEND
#define __CLICK_TCP_FRONTEND
#include <click/config.h>
#include <click/element.hh>
#include <vector>
#include <queue>
#include "tcp_socket.hh"
CLICK_DECLS

class TcpFrontend : public Element {
    std::vector<TcpSocket> sockets;
    uint32_t self;
public:
    const char *class_name() const { return "TcpFrontend"; }
    const char *port_count() const { return "2/2"; }
    const char *processing() const { return PUSH; }
    int find_socket(uint32_t, uint16_t, uint16_t);
    int find_empty_socket();
    void push_socket(uint8_t, Packet *);
    void push_tcp(Packet *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
