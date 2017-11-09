#include "infra_frontend.hh"
#include "infra_anno.hh"
CLICK_DECLS

void InfraFrontend::push(int port, Packet *p) {
    p->set_anno_s16(FromInterface, port);
    output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraFrontend)
