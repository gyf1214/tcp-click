// Tcp Window Control Backend
// input(0) : connect to frontend
// output(0) : sending backend
// output(1) : socket client
// output(2) : tcp frontend
#ifndef __CLICK_TCP_BACKEND
#define __CLICK_TCP_BACKEND
#include <click/config.h>
#include <click/element.hh>
#include <click/vector.hh>
#include "tcp_window.hh"
CLICK_DECLS

class TcpBackend : public Element {
    Vector<TcpBlock> tcb;
    uint32_t self;
    Timestamp timeout;
public:
    const char *class_name() const { return "TcpBackend"; }
    const char *port_count() const { return "1/3"; }
    const char *processing() const { return PUSH; }
    int configure(Vector<String> &, ErrorHandler *);
    static void sending_timer(Timer *, void *);
    WritablePacket *packet_from_wnd(uint8_t, uint32_t, uint32_t);
    void send_timeout(uint8_t);
    void build_link(uint8_t, uint32_t, uint16_t, uint16_t);
    void clean_link(uint8_t);
    void return_send(Packet *, bool = false);
    bool try_grow_send(uint8_t);
    bool try_buffer_send(uint8_t, Packet *);
    void try_resolve_send(uint8_t);
    bool try_buffer_recv(uint8_t, Packet *);
    void try_resolve_recv(uint8_t);
    void push_tcp(uint8_t, Packet *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
