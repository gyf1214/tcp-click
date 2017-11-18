#include "infra_frontend.hh"
#include "infra_anno.hh"
CLICK_DECLS

InfraFrontend::InfraFrontend() : power(1) {}

void InfraFrontend::push(int port, Packet *p) {
    if (!power) {
        p->kill();
        return;
    }
    p->set_anno_s16(FromInterface, port);
    output(0).push(p);
}

void InfraFrontend::add_handlers() {
    add_data_handlers("power", Handler::OP_READ | Handler::OP_WRITE, &power);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfraFrontend)
