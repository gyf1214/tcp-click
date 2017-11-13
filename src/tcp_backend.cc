#include "tcp_backend.hh"
#include "tcp_socket.hh"
#include "tcp_packet.hh"
#include "ip_packet.hh"
#include "infra_log.hh"
#include <click/confparse.hh>
CLICK_DECLS

int TcpBackend::configure(Vector<String> &args, ErrorHandler *errh) {
    if (cp_va_kparse(args, this, errh,
    "IP", cpkP + cpkM, cpIPAddress, &self, cpEnd) < 0) {
        return -1;
    }
    return 0;
}

void TcpBackend::build_link(uint8_t i, uint32_t ip, uint16_t sport, uint16_t dport) {
    if (i >= tcb.size()) {
        tcb.resize(i + 1);
    }
    tcb[i].ip = ip;
    tcb[i].sport = sport;
    tcb[i].dport = dport;
    tcb[i].swnd.init(this, sending_timer);
}

void TcpBackend::clean_link(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;
    swnd.timer.assign();
    swnd.timer.clear();
    swnd.wnd.clear();
    while (!swnd.wait.empty()) {
        Packet *p = swnd.wait.front();
        return_send(p, true);
        swnd.wait.pop_front();
    }
}

void TcpBackend::return_send(Packet *p, bool error) {
    uint8_t method = error ? Error : p->anno_u8(SocketMethod);
    Packet *q = SocketPacket(method, p->anno_u8(SocketId),
    p->anno_u8(SocketSequence));
    p->kill();
    output(1).push(q);
}

bool TcpBackend::try_grow_send(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;

    uint32_t grow = swnd.max_grow();
    if (!grow) {
        return false;
    }
    Log("send sequence %d len %d", swnd.seq_back, grow);
    // grow window and send
    WritablePacket *q = Packet::make(TcpSize + grow);
    TcpHeader *tcp_q = (TcpHeader *)q->data();
    // make packet
    q->set_anno_u8(SendProto, IpProtoTcp);
    q->set_anno_u32(SendIp, tcb[i].ip);
    tcp_q->syn(tcb[i].sport, tcb[i].dport, swnd.seq_back);
    tcp_from_wnd(tcp_q->data, swnd.buf, TcpBufferSize, swnd.seq_back, swnd.seq_back + grow);
    // update pointer
    swnd.seq_back += grow;
    swnd.wnd.push_back(grow);
    // send
    output(0).push(q);
    return true;
}

bool TcpBackend::try_buffer_send(uint8_t i, Packet *p) {
    TcpSendWindow &swnd = tcb[i].swnd;

    uint32_t cap = swnd.max_buffer();
    if (cap < p->length()) {
        return false;
    }
    Log("push %d data to buffer", p->length());
    // buffer sending data
    tcp_to_wnd(swnd.buf, (const char *)p->data(), TcpBufferSize,
    swnd.buf_back, swnd.buf_back + p->length());
    // update pointer
    swnd.buf_back += p->length();
    return_send(p);
    return true;
}

void TcpBackend::try_resolve_send(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;

    while (!swnd.wait.empty()) {
        Packet *p = swnd.wait.front();
        if (try_buffer_send(i, p)) {
            swnd.wait.pop_front();
        } else {
            break;
        }
    }
}

void TcpBackend::push_tcp(uint8_t i, Packet *p) {
    const TcpHeader *tcp_p = (const TcpHeader *)p->data();

    if (tcp_p->flags & Ack) {
        // Ack
        TcpSendWindow &swnd = tcb[i].swnd;
        swnd.rwnd = tcp_p->window;
        bool recved = false;
        Log("ack %d, rwnd %d", tcp_p->acknowledge, tcp_p->window);
        while (!swnd.wnd.empty() && tcp_p->acknowledge > swnd.seq_front) {
            recved = true;
            swnd.seq_front += swnd.wnd.front();
            swnd.wnd.pop_front();
        }
        if (swnd.seq_front != tcp_p->acknowledge) {
            Warn("error in sliding window");
            // TODO: anything to deal with this error?
        }
        if (!recved) {
            ++swnd.fails;
            // TODO: Reno Fast Retransmission & Fast Recovery
            Log("dup ack");
        } else {
            swnd.fails = 0;
            // as buffer moves forward, try resolve waiting send requests
            try_resolve_send(i);
            // try send more packets
            while (try_grow_send(i));
        }
    } else if (tcp_p->flags & Syn) {
        // TODO: recv control
    } else {
        Warn("unknown tcp packet");
    }
    // must kill packet
    p->kill();
}

void TcpBackend::sending_timer(Timer *, void *) {
    // TODO: implement timeout
}

void TcpBackend::push(int, Packet *p) {
    uint8_t i = p->anno_u8(SocketId);

    switch (p->anno_u8(SocketMethod)) {
    case Data:
        push_tcp(i, p);
        break;
    case Connect:
        Log("establish tcb");s
        build_link(i, p->anno_u32(RecvIp), p->anno_u16(SrcPort), p->anno_u16(DstPort));
        p->kill();
        break;
    case Close:
        Log("close tcb");
        p->kill();
        clean_link(i);
        break;
    case Send:
        if (!try_buffer_send(i, p)) {
            tcb[i].swnd.wait.push_back(p);
        }
        while (try_grow_send(i));
        break;
    case Recv:
        // TODO
        p->kill();
        break;
    default:
        Warn("unknown request");
        p->kill();
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(TcpBackend);
