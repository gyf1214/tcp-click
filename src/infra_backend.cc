#include "infra_backend.hh"
#include "infra_anno.hh"
CLICK_DECLS

void InfraBackend::push(int, Packet *p) {
    int port = p->anno_s16(ToInterface);
    if (port < 0) {
        int n = noutputs();
        for (int i = 0; i < n - 1; ++i) {
            if (Packet *q = p->clone()) {
                output(i).push(q);
            }
        }
        output(n - 1).push(p);
    } else {
        output(port).push(p);
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraBackend)
