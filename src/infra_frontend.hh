#ifndef __CLICK_INFRA_FRONTEND
#define __CLICK_INFRA_FRONTEND
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class InfraFrontend : public Element {
public:
    const char *class_name() const { return "InfraFrontend"; }
    const char *port_count() const { return "-/1"; }
    const char *processing() const { return PUSH; }
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
