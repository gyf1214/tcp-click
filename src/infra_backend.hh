#ifndef __CLICK_INFRA_BACKEND
#define __CLICK_INFRA_BACKEND
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class InfraBackend : public Element {
public:
    const char *class_name() const { return "InfraBackend"; }
    const char *port_count() const { return "-/-"; }
    const char *processing() const { return PUSH; }
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
