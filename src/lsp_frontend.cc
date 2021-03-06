#include "lsp_frontend.hh"
#include "ip_packet.hh"
#include "infra_anno.hh"
#include "infra_log.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <algorithm>
CLICK_DECLS

LspFrontend::LspFrontend() : state(Sleep), timer(this), sequence(0) {}

int LspFrontend::configure(Vector<String> &args, ErrorHandler *errh) {
    if (cp_va_kparse(args, this, errh,
    "IP", cpkP + cpkM, cpIPAddress, &self,
    "INTERVAL", cpkP + cpkM, cpTimestamp, &interval,
    "TIMEOUT", cpkP + cpkM, cpTimestamp, &timeout, cpEnd) < 0) {
        return -1;
    }
    return 0;
    return 0;
}

int LspFrontend::initialize(ErrorHandler *) {
    timer.initialize(this);
    timer.schedule_now();
    Log("initialized");
    return 0;
}

void LspFrontend::run_timer(Timer *) {
    if (state == Sleep) {
        Log("discover neighbour");
        state = WaitAck;

        // clear previous neighbour table
        portInfo.clear();
        // broadcast hello
        output(0).push(build_packet(LspHello, IpAny, AnnoAnyPort));

        timer.schedule_after(timeout);
    } else {
        Log("broadcast sequence");
        state = Sleep;

        // push to broadcast and also to backend
        Packet *p = build_sequence();
        Packet *q = p->clone();
        q->set_anno_s16(ToInterface, AnnoAnyPort);
        output(1).push(p);
        output(0).push(q);

        timer.schedule_after(interval);
    }
}

WritablePacket *LspFrontend::build_packet(LspType type, uint32_t dst, int port) {
    size_t size = IpSize + LspSizeShort;
    WritablePacket *q = Packet::make(size);
    IpHeader *ip_q = (IpHeader *)q->data();
    LspHeader *lsp_q = (LspHeader *)ip_q->data;

    ip_q->init(size, IpProtoLsp, self, dst);
    lsp_q->init(type);
    q->set_anno_s16(ToInterface, port);

    return q;
}

WritablePacket *LspFrontend::build_sequence() {
    // count neighbours
    int n = portInfo.size();

    size_t size = IpSize + LspSize + n * sizeof(uint32_t);
    WritablePacket *q = Packet::make(size);
    IpHeader *ip_q = (IpHeader *)q->data();
    LspHeader *lsp_q = (LspHeader *)ip_q->data;
    LspSequenceData *seq_q = lsp_q->data;

    ip_q->init(size, IpProtoLsp, self, IpAny);
    lsp_q->init(LspSequence);
    seq_q->sequence = ++sequence;
    seq_q->count = n;
    for (int i = 0; i < n; ++i) {
        seq_q->entry[i] = portInfo[i];
    }

    return q;
}

bool LspFrontend::check_sequence(uint32_t ip, uint32_t seq) {
    int n = sequenceInfo.size();
    for (int i = 0; i < n; ++i) {
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
    Log("packet from %08x@%d", ip->src, port);

    // discard if magic not match
    if (lsp->magic != LspMagic) {
        Log("warning: magic not match");
        p->kill();
        return;
    }
    if (lsp->type == LspAck && state == WaitAck) {
        // receive ack when discovering neighbour
        Log("ack");
        if (port >= (int)portInfo.size()) portInfo.resize(port + 1, 0);
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

            // forward to all ports
            Packet *q = p->clone();
            q->set_anno_s16(ToInterface, AnnoAnyPort);
            output(1).push(p);
            output(0).push(q);
        } else {
            Log("older sequence");
            // discard
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
