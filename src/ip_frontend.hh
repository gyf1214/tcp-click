// IP Frontend
// input(0) : incoming packets
// input(1) : pull input for routing table
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
    const char *port_count() const { return "2/3"; }
    const char *processing() const { return "hl/h"; }
    int configure(Vector<String> &, ErrorHandler *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
