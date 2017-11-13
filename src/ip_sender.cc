#include "ip_sender.hh"
#include "infra_anno.hh"
#include <click/confparse.hh>
CLICK_DECLS

int IpSender::configure(Vector<String> &args, ErrorHandler *errh) {
    if (cp_va_kparse(args, this, errh,
    "PROTO", cpkM+cpkP, cpByte, &proto,
    "DST", cpkM+cpkP, cpIPAddress, &dst, cpEnd) < 0) {
        return -1;
    }
    return 0;
}

Packet *IpSender::simple_action(Packet *p) {
    p->set_anno_u8(SendProto, proto);
    p->set_anno_u32(SendIp, dst);
    return p;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IpSender)
