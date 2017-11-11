#include "ip_frontend.hh"
#include "ip_packet.hh"
#include "lsp_packet.hh"
#include "infra_log.hh"
#include "infra_anno.hh"
#include <click/args.hh>
#include <click/error.hh>
CLICK_DECLS

int IpFrontend::configure(Vector<String> &args, ErrorHandler *errh) {
    String ip_str;
    if (Args(args, this, errh)
    .read_mp("IP", ip_str)
    .complete() < 0) {
        return -1;
    }
    if (!IPAddressArg().parse(ip_str, (struct in_addr &)self, this)) {
        return errh->error("IP should be ip address");
    }
    return 0;
}

void IpFrontend::push(int, Packet *p) {
    WritablePacket *q = p->uniqueify();
    IpHeader *ip_q = (IpHeader *)q->data();
    uint16_t checksum = click_in_cksum((unsigned char *)ip_q, IpSize);
    if (ip_q->magic != IpMagic || checksum) {
        Warn("ip header error");
        // discard on header error
        q->kill();
        return;
    }
    if (!--ip_q->ttl) {
        Warn("ttl reach zero");
        // discard on ttl zero;
        q->kill();
        return;
    }
    ip_q->Checksum();
    Log("ip packet %08x -> %08x", ip_q->src, ip_q->dst);
    if (ip_q->protocol == IpProtoLsp) {
        // foward to lsp frontend
        Log("lsp packet");
        output(1).push(q);
    } else if (ip_q->dst == self) {
        // forward to self
        Log("packet for self");
        output(2).push(q);
    } else {
        // routing
        Log("routing");
        Packet *r = input(1).pull();
        const LspRouting *lsp_r = (const LspRouting *)r->data();
        int n = lsp_r->count;
        for (int i = 0; i < n; ++i) {
            uint32_t ip = lsp_r->entry[i].ip;
            int port = lsp_r->entry[i].port;
            if (ip == ip_q->dst && port > 0) {
                Log("forward to %d", port);
                q->set_anno_s16(ToInterface, port);
                output(0).push(q);
                return;
            }
        }
        Warn("unknown route to %08x", ip_q->dst);
        q->kill();
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IpFrontend)
