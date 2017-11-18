// IP Frontend
// input(0) : incoming packets
// input(1) : pull input for routing table
// input(2) : self sending packets
// output(0) : sending backend
// output(1) : lsp frontend
// output(2) : packet for self & not lsp
#ifndef __CLICK_IP_FRONTEND
#define __CLICK_IP_FRONTEND
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class IpFrontend : public Element {
    // self ip
    uint32_t self;
public:
    const char *class_name() const { return "IpFrontend"; }
    const char *port_count() const { return "3/3"; }
    const char *processing() const { return "hlh/h"; }
    int configure(Vector<String> &, ErrorHandler *);
    void push(int, Packet *);
    void send(Packet *);
    void route(Packet *, uint32_t dst);
};

CLICK_ENDDECLS
#endif
