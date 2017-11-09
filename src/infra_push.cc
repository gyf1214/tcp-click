#include "infra_pusher.hh"
CLICK_DECLS

void InfraPusher::push(int, Packet *p) {
    output(0).push(p);
}

CLICK_ENDDECLS
