#ifndef __CLICK_INFRA_FRONTEND
#define __CLICK_INFRA_FRONTEND
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class InfraFrontend : public Element {
    uint8_t power;
public:
    InfraFrontend();
    const char *class_name() const { return "InfraFrontend"; }
    const char *port_count() const { return "-/1"; }
    const char *processing() const { return PUSH; }
    void push(int, Packet *);
    void add_handlers();
};

CLICK_ENDDECLS
#endif
