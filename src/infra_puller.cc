#include "infra_puller.hh"
#include <click/config.h>
CLICK_DECLS

Packet *InfraPuller::pull(int) {
    return input(0).pull();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraPuller)
