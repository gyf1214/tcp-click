#include "tcp_backend.hh"
#include "tcp_socket.hh"
#include "tcp_packet.hh"
#include "ip_packet.hh"
#include "infra_log.hh"
#include <click/confparse.hh>
CLICK_DECLS

int TcpBackend::configure(Vector<String> &args, ErrorHandler *errh) {
    if (cp_va_kparse(args, this, errh,
    "IP", cpkP + cpkM, cpIPAddress, &self,
    "TIMEOUT", cpkP + cpkM, cpTimestamp, &timeout, cpEnd) < 0) {
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
    tcb[i].swnd.init(this, sending_timer, i);
    tcb[i].rwnd.init();
}

void TcpBackend::clean_link(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;
    if (swnd.timer) {
        delete swnd.timer;
        swnd.timer = NULL;
    }
    swnd.wnd.clear();
    while (!swnd.wait.empty()) {
        Packet *p = swnd.wait.front();
        return_send(p, true);
        swnd.wait.pop_front();
    }

    TcpRecvWindow &rwnd = tcb[i].rwnd;
    while (!rwnd.wait.empty()) {
        Packet *p = rwnd.wait.front();
        return_send(p, true);
        rwnd.wait.pop_front();
    }
}

void TcpBackend::return_send(Packet *p, bool error) {
    uint8_t method = error ? Error : p->anno_u8(SocketMethod);
    Packet *q = SocketPacket(method, p->anno_u8(SocketId),
    p->anno_u32(SocketSequence));
    p->kill();
    output(1).push(q);
}

WritablePacket *TcpBackend::packet_from_wnd(uint8_t i, uint32_t seq, uint32_t size) {
    TcpSendWindow &swnd = tcb[i].swnd;

    Log("send seq %u len %u", seq, size);
    WritablePacket *q = Packet::make(TcpSize + size);
    TcpHeader *tcp_q = (TcpHeader *)q->data();
    // make packet
    q->set_anno_u8(SendProto, IpProtoTcp);
    q->set_anno_u32(SendIp, tcb[i].ip);
    tcp_q->syn(tcb[i].sport, tcb[i].dport, seq);
    tcp_from_wnd(tcp_q->data, swnd.buf, TcpBufferSize, seq, seq + size);

    return q;
}

bool TcpBackend::try_grow_send(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;

    uint32_t grow = swnd.max_grow();
    if (!grow) {
        return false;
    }
    // grow window and send
    WritablePacket *q = packet_from_wnd(i, swnd.seq_back, grow);
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
    Log("push %u data to buffer", p->length());
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

bool TcpBackend::try_buffer_recv(uint8_t i, Packet *p) {
    TcpRecvWindow &rwnd = tcb[i].rwnd;
    uint32_t len = rwnd.max_recv();

    if (!len) {
        return false;
    }
    WritablePacket *q = p->uniqueify();
    uint32_t l0 = q->length();
    if (l0 > len) {
        q->take(l0 - len);
    } else {
        len = l0;
    }
    Log("pop %u data from buffer", len);
    // take data from buffer
    tcp_from_wnd((char *)q->data(), rwnd.buf, TcpBufferSize,
    rwnd.seq_front, rwnd.seq_front + len);
    // update pointer
    rwnd.seq_front += len;
    output(1).push(q);

    return true;
}

void TcpBackend::try_resolve_recv(uint8_t i) {
    TcpRecvWindow &rwnd = tcb[i].rwnd;

    while (!rwnd.wait.empty()) {
        Packet *p = rwnd.wait.front();
        if (try_buffer_recv(i, p)) {
            rwnd.wait.pop_front();
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
        uint32_t ack = ntohl(tcp_p->acknowledge);
        uint32_t wnd = ntohs(tcp_p->window);
        Log("ack %u, wnd %u", ack, wnd);

        bool recved = false;
        while (!swnd.wnd.empty()) {
            if (!swnd.wnd.front() || ack > swnd.seq_front) {
                recved = true;
                swnd.seq_front += swnd.wnd.front();
                swnd.wnd.pop_front();
            } else {
                break;
            }
        }

        swnd.rwnd = wnd;
        if (!swnd.rwnd) {
            Warn("zero window");
            swnd.timer->schedule_after(timeout);
        }

        if (ack > swnd.seq_front) {
            recved = true;
            swnd.seq_front = ack;
            if (swnd.seq_back < swnd.seq_front) {
                swnd.seq_back = swnd.seq_front;
            }
        }

        if (!recved) {
            // TODO: Reno Fast Recovery
            Warn("dup ack");
            // fast retransmission
            if (++swnd.fails >= 3) {
                Log("fast resend");
                send_timeout(i);
            }
        } else {
            swnd.fails = 0;
            // cwnd control
            if (swnd.cwnd <= swnd.c_threshold) {
                // slow start
                ++swnd.cwnd;
            } else {
                // linear;
                ++swnd.succs;
                if (swnd.succs >= swnd.cwnd) {
                    ++swnd.cwnd;
                    swnd.succs = 0;
                }
            }
            Log("update cwnd %u", swnd.cwnd);

            // as buffer moves forward, try resolve waiting send requests
            try_resolve_send(i);
            // try send more packets
            while (try_grow_send(i));
            // update timer
            if (!swnd.wnd.empty() || !swnd.rwnd) {
                swnd.timer->schedule_after(timeout);
            } else {
                swnd.timer->unschedule();
            }

            // if fin and no data to send
            if (swnd.fin && swnd.seq_front == swnd.buf_back) {
                Log("reset tcb");
                clean_link(i);
                // inform frontend
                output(2).push(SocketPacket(Close, i, 0));
            }
        }
    } else if (tcp_p->flags & Syn) {
        // Syn Data Packet
        TcpRecvWindow &rwnd = tcb[i].rwnd;
        size_t len = p->length() - TcpSize;
        uint32_t seq = ntohl(tcp_p->sequence);
        uint32_t tail = seq + len;
        Log("syn %u, len %u", seq, len);

        if (tail > rwnd.max_tail()) {
            Warn("tail exceeds");
        } else if (seq < rwnd.seq_back || rwnd.check_disorder(seq, tail)) {
            Warn("dup sequence");
        } else {
            // mark recved & move forward
            rwnd.mark_disorder(seq, tail);
            rwnd.forward();
            // as buffer moves forward, try resolve waiting recv requests
            try_resolve_recv(i);
            Log("update ack %u, wnd %u", rwnd.seq_back, rwnd.max_grow());
        }
        // send back ack
        WritablePacket *q = Packet::make(TcpSize);
        TcpHeader *tcp_q = (TcpHeader *)q->data();
        q->set_anno_u8(SendProto, IpProtoTcp);
        q->set_anno_u32(SendIp, tcb[i].ip);
        tcp_q->ack(tcb[i].sport, tcb[i].dport, rwnd.seq_back, rwnd.max_grow());

        output(0).push(q);
    } else {
        Warn("unknown tcp packet");
    }
    // must kill packet
    p->kill();
}

void TcpBackend::send_timeout(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;

    // update cwnd
    if (swnd.cwnd >= swnd.wnd.size()) {
        swnd.c_threshold = swnd.cwnd / 2;
        swnd.cwnd = 1;
        Log("congestion threshold %u", swnd.c_threshold);

        // clear sending window
        while (swnd.wnd.size() > 1) {
            swnd.wnd.pop_back();
            swnd.seq_back = swnd.seq_front;
        }
    }

    // resend
    uint32_t len = swnd.wnd.front();
    Log("resend %d", i);
    Packet *p = packet_from_wnd(i, swnd.seq_front, len);
    swnd.seq_back += len;
    output(0).push(p);

    swnd.timer->schedule_after(timeout);
}

void TcpBackend::send_probe(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;

    Log("window probe %d", i);
    Packet *p = packet_from_wnd(i, swnd.seq_front, 0);
    swnd.wnd.push_back(0);
    output(0).push(p);

    swnd.timer->schedule_after(timeout);
}

void TcpBackend::sending_timer(Timer *t, void *data) {
    click_chatter("timeout");
    uint8_t i = (intptr_t)data;
    TcpBackend *e = (TcpBackend *)t->element()->cast("TcpBackend");

    if (e->tcb[i].swnd.wnd.empty()) {
        e->send_probe(i);
    } else {
        e->send_timeout(i);
    }

}

void TcpBackend::push(int, Packet *p) {
    uint8_t i = p->anno_u8(SocketId);

    switch (p->anno_u8(SocketMethod)) {
    case Data:
        push_tcp(i, p);
        break;
    case Connect:
        Log("establish tcb");
        build_link(i, p->anno_u32(RecvIp), p->anno_u16(SrcPort), p->anno_u16(DstPort));
        p->kill();
        break;
    case Close:
        Log("tcb fin");
        if (tcb[i].swnd.seq_front == tcb[i].swnd.buf_back) {
            output(2).push(p);
        } else {
            tcb[i].swnd.fin = true;
            p->kill();
        }
        break;
    case Free:
        Log("reset tcb");
        clean_link(i);
        p->kill();
        break;
    case Send:
        if (!try_buffer_send(i, p)) {
            tcb[i].swnd.wait.push_back(p);
        }
        while (try_grow_send(i));
        // if wnd not empty then issue timeout timer
        if (!tcb[i].swnd.wnd.empty() && !tcb[i].swnd.timer->scheduled()) {
            tcb[i].swnd.timer->schedule_after(timeout);
        }
        break;
    case Recv:
        if (!try_buffer_recv(i, p)) {
            tcb[i].rwnd.wait.push_back(p);
        }
        break;
    default:
        Warn("unknown request");
        p->kill();
    }
}



CLICK_ENDDECLS
EXPORT_ELEMENT(TcpBackend);
