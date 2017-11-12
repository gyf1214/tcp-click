#include "ip_sender.hh"
#include "infra_anno.hh"
#include <click/args.hh>
#include <click/error.hh>
CLICK_DECLS

int IpSender::configure(Vector<String> &args, ErrorHandler *errh) {
    String ip_str;
    if (Args(args, this, errh)
    .read_mp("PROTO", proto)
    .read_mp("DST", ip_str)
    .complete() < 0) {
        return -1;
    }
    if (!IPAddressArg().parse(ip_str, (struct in_addr &)dst, this)) {
        return errh->error("IP should be ip address");
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
