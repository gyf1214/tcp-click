// Puller listen on multiple output and pull from one input
#ifndef __CLICK_INFRA_PULLER
#define __CLICK_INFRA_PULLER
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class InfraPuller : public Element {
public:
    const char *class_name() const { return "InfraPuller"; }
    const char *port_count() const { return "1/-"; }
    const char *processing() const { return PULL; }
    Packet *pull(int);
};

CLICK_ENDDECLS
#endif
