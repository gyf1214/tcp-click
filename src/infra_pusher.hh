// Pusher receive from multiple input and send to one output
#ifndef __CLICK_INFRA_PUSHER
#define __CLICK_INFRA_PUSHER
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class InfraPusher : public Element {
public:
    const char *class_name() const { return "InfraPusher"; }
    const char *port_count() const { return "-/1"; }
    const char *processing() const { return PUSH; }
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
