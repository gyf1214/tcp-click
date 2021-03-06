#include "infra_backend.hh"
#include "infra_anno.hh"
CLICK_DECLS

InfraBackend::InfraBackend() : power(1) {}

void InfraBackend::push(int, Packet *p) {

    if(!power){
        p -> kill();
        return ;
    }

    int port = p->anno_s16(ToInterface);
    if (port < 0) {
        int n = noutputs();
        for (int i = 0; i < n - 1; ++i) {
            Packet *q = p->clone();
            output(i).push(q);
        }
        output(n - 1).push(p);
    } else {
        output(port).push(p);
    }
}

void InfraBackend::add_handlers() {
    add_data_handlers("power", Handler::OP_READ | Handler::OP_WRITE, &power);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraBackend)
