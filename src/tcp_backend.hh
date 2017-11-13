// Tcp Window Control Backend
// input(0) : connect to frontend
// output(0) : sending backend
// output(1) : socket client
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
public:
    const char *class_name() const { return "TcpBackend"; }
    const char *port_count() const { return "1/2"; }
    const char *processing() const { return PUSH; }
    int configure(Vector<String> &, ErrorHandler *);
    static void sending_timer(Timer *, void *);
    void build_link(uint8_t, uint32_t, uint16_t, uint16_t);
    void clean_link(uint8_t);
    void return_error(Packet *);
    bool try_grow_send(uint8_t);
};

CLICK_ENDDECLS
#endif
