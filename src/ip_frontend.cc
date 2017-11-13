#include "ip_frontend.hh"
#include "lsp_packet.hh"
#include "infra_log.hh"
#include "infra_anno.hh"
#include "ip_packet.hh"
#include <click/confparse.hh>
CLICK_DECLS

int IpFrontend::configure(Vector<String> &args, ErrorHandler *errh) {
    String ip_str;
    if (cp_va_kparse(args, this, errh,
    "IP", cpkP + cpkM, cpIPAddress, &self, cpEnd) < 0) {
        return -1;
    }
    return 0;
}

void IpFrontend::send(Packet *p) {
    // send packet from self
    WritablePacket *q = p->push(IpSize);
    IpHeader *ip_q = (IpHeader *)q->data();
    ip_q->init(q->length(), q->anno_u8(SendProto), self, q->anno_u32(SendIp));
    route(q, ip_q->dst);
}

void IpFrontend::push(int port, Packet *p) {
    if (port == 2) {
        send(p);
        return;
    }

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
    ip_q->calc_checksum();
    Log("ip packet %08x -> %08x", ip_q->src, ip_q->dst);
    if (ip_q->protocol == IpProtoLsp) {
        // foward to lsp frontend
        Log("lsp packet");
        output(1).push(q);
    } else if (ip_q->dst == self) {
        // forward to self
        Log("packet for self");
        q->set_anno_u8(RecvProto, ip_q->protocol);
        q->set_anno_u32(RecvIp, ip_q->src);
        q->pull(IpSize);
        output(2).push(q);
    } else {
        // routing
        Log("routing");
        route(q, ip_q->dst);
    }
}

void IpFrontend::route(Packet *q, uint32_t dst) {
    Packet *r = input(1).pull();
    const LspRouting *lsp_r = (const LspRouting *)r->data();
    int n = lsp_r->count;
    for (int i = 0; i < n; ++i) {
        uint32_t ip = lsp_r->entry[i].ip;
        int port = lsp_r->entry[i].port;
        if (ip == dst && port >= 0) {
            Log("forward to %d", port);
            q->set_anno_s16(ToInterface, port);
            output(0).push(q);
            return;
        }
    }
    Warn("unknown route to %08x", dst);
    q->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IpFrontend)
