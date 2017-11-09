#include "infra_backend.hh"
#include "infra_anno.hh"
CLICK_DECLS

void InfraBackend::push(int, Packet *p) {
    int port = p->anno_s16(ToInterface);
    output(port).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraBackend)
