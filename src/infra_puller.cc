#include "infra_puller.hh"
CLICK_DECLS

Packet *InfraPuller::pull(int) {
    return input(0).pull();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraPuller)
