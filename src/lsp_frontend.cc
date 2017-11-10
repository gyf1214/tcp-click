#include "lsp_frontend.hh"
#include "ip_packet.hh"
#include "infra_anno.hh"
#include "infra_log.hh"
#include <click/args.hh>
#include <click/error.hh>
using namespace std;
CLICK_DECLS

LspFrontend::LspFrontend() : state(Sleep), timer(this) {}

int LspFrontend::configure(Vector<String> &args, ErrorHandler *errh) {
    String ip_str;
    if (Args(args, this, errh)
    .read_mp("IP", ip_str)
    .read_mp("INTERVAL", interval)
    .read_mp("TIMEOUT", timeout)
    .complete() < 0) {
        return -1;
    }
    if (!IPAddressArg().parse(ip_str, (struct in_addr &)self, this)) {
        return errh->error("IP should be ip address");
    }
    return 0;
}

int LspFrontend::initialize(ErrorHandler *) {
    timer.initialize(this);
    timer.reschedule_after(interval);
    Log("initialized");
    return 0;
}

void LspFrontend::run_timer(Timer *) {
    if (state == Sleep) {
        Log("discover neighbour");
        state = WaitAck;

        // broadcast hello
        output(0).push(build_packet(LspHello, IpAny, -1));

        timer.reschedule_after(timeout);
    } else {
        Log("broadcast sequence");
        state = Sleep;
        // TODO: broadcast sequence
        timer.reschedule_after(interval);
    }
}

WritablePacket *LspFrontend::build_packet(LspType type, uint32_t dst, int port) {
    size_t size = IpSize + LspSizeShort;
    WritablePacket *q = Packet::make(size);
    IpHeader *ip_q = (IpHeader *)q->data();
    LspHeader *lsp_q = (LspHeader *)ip_q->data;

    ip_q->Init(size, LspProto, self, dst);
    lsp_q->Init(type);
    q->set_anno_s16(ToInterface, port);

    return q;
}

bool LspFrontend::check_sequence(uint32_t ip, uint32_t seq) {
    for (int i = 0; i < sequenceInfo.size(); ++i) {
        if (sequenceInfo[i].first == ip) {
            if (sequenceInfo[i].second >= seq) {
                return false;
            } else {
                sequenceInfo[i].second = seq;
                return true;
            }
        }
    }
    sequenceInfo.push_back(make_pair(ip, seq));
    return true;
}

void LspFrontend::push(int, Packet *p) {
    const IpHeader *ip = (const IpHeader *)p->data();
    const LspHeader *lsp = (const LspHeader *)ip->data;
    int port = p->anno_s16(FromInterface);
    Log("packet from %x@%d", ip->src, port);

    // discard if magic not match
    if (lsp->magic != LspMagic) {
        Log("warning: magic not match");
        p->kill();
        return;
    }
    if (lsp->type == LspAck && state == WaitAck) {
        // receive ack when discovering neighbour
        Log("ack");
        if (port >= (int)portInfo.size()) portInfo.resize(port + 1);
        portInfo[port] = ip->src;
        // discard
        p->kill();
    } else if (lsp->type == LspHello) {
        // return ack when receiving hello
        Log("hello");
        WritablePacket *q = build_packet(LspAck, ip->dst, port);
        // send back
        p->kill();
        output(0).push(q);
    } else if (lsp->type == LspSequence) {
        // forward sequence packet to backend
        const LspSequenceData *seq = lsp->data;

        Log("sequence");
        if (ip->src != self && check_sequence(ip->src, seq->sequence)) {
            Log("update sequence: %d", seq->sequence);
            // forward
        } else {
            Log("older sequence");
            p->kill();
        }
    } else {
        Warn("type unknown or excessive ack");
        // discard on unknown type
        p->kill();
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(LspFrontend)
