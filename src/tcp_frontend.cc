#include "tcp_frontend.hh"
#include "infra_anno.hh"
#include "infra_log.hh"
#include "tcp_packet.hh"
#include "ip_packet.hh"
#include <click/args.hh>
#include <click/error.hh>
CLICK_DECLS

int TcpFrontend::configure(Vector<String> &conf, ErrorHandler *errh) {
    String ip_str;
    if (Args(conf, this, errh)
    .read_mp("IP", ip_str)
    .complete() < 0) {
        return -1;
    }
    if (!IPAddressArg().parse(ip_str, (struct in_addr &)self, this)) {
        return errh->error("IP should be ip address");
    }
    return 0;
}

int TcpFrontend::find_socket(uint32_t ip, uint16_t src_port, uint16_t dst_port) {
    int n = sockets.size();
    for (int i = 0; i < n; ++i) {
        if (sockets[i].dst_ip == ip && sockets[i].src_port == src_port
        && sockets[i].dst_port == dst_port && sockets[i].state != Nil) {
            return i;
        }
    }
    return -1;
}

int TcpFrontend::find_bind_socket(uint16_t port, bool listen) {
    int n = sockets.size();
    for (int i = 0; i < n; ++i) {
        if (sockets[i].src_port == port && (!listen || sockets[i].state == Listening)) {
            return i;
        }
    }
    return -1;
}

int TcpFrontend::find_empty_socket() {
    int n = sockets.size();
    for (int i = 0; i < n; ++i) {
        if (sockets[i].state == Nil) {
            return i;
        }
    }
    sockets.resize(n + 1);
    sockets[n].closeWait = NULL;
    sockets[n].connectWait = NULL;
    sockets[n].state = Nil;
    return n;
}

void TcpFrontend::send_return(Packet *p, bool err) {
    WritablePacket *q = Packet::make(0);
    q->set_anno_u8(SocketMethod, err ? Error : p->anno_u8(SocketMethod));
    q->set_anno_u8(SocketId, p->anno_u8(SocketId));
    q->set_anno_u32(SocketSequence, p->anno_u32(SocketSequence));
    p->kill();
    output(1).push(q);
}

void TcpFrontend::send_short(uint32_t ip, uint16_t sport, uint16_t dport, uint16_t flags) {
    WritablePacket *q = Packet::make(TcpSize);
    TcpHeader *tcp_q = (TcpHeader *)q->data();
    tcp_q->init(sport, dport, flags);
    q->set_anno_u32(SendIp, ip);
    q->set_anno_u8(SendProto, IpProtoTcp);
    output(0).push(q);
}

void TcpFrontend::send_short(int i, uint16_t flags) {
    send_short(sockets[i].dst_ip, sockets[i].src_port, sockets[i].dst_port, flags);
}

void TcpFrontend::create_accept(int i, uint32_t ip, uint16_t port) {
    Log("%d", sockets[i].src_port);
    int id = find_empty_socket();
    Log("%d", sockets[i].src_port);
    sockets[id].dst_ip = ip;
    sockets[id].dst_port = port;
    Log("%d", sockets[id].src_port);
    sockets[id].src_port = sockets[id].src_port;
    Log("%d", sockets[id].src_port);
    sockets[id].state = Syn_Rcvd;
    Log("%d", sockets[id].src_port);
}

void TcpFrontend::queue_accept(int i, Packet *p) {
    if (sockets[i].listenWait.empty()) {
        sockets[i].acceptWait.push_back(p);
    } else {
        Packet *q = sockets[i].listenWait.front();
        sockets[i].listenWait.pop_front();
        q->set_anno_u32(SocketSequence, p->anno_u32(SocketSequence));
        output(1).push(q);
        p->kill();
    }
}

void TcpFrontend::queue_listen(uint8_t id0, uint8_t id) {
    Packet *q = Packet::make(0);
    q->set_anno_u8(SocketMethod, Return);
    q->set_anno_u8(SocketId, id);
    q->set_anno_u8(SocketParentId, id0);

    if (sockets[id0].acceptWait.empty()) {
        sockets[id0].listenWait.push_back(q);
    } else {
        Packet *p = sockets[id0].acceptWait.front();
        sockets[id0].acceptWait.pop_front();
        q->set_anno_u32(SocketSequence, p->anno_u32(SocketSequence));
        output(1).push(q);
        p->kill();
    }
}

void TcpFrontend::free_wait(int i) {
    if (sockets[i].connectWait) {
        send_return(sockets[i].connectWait, true);
        sockets[i].connectWait = NULL;
    }
    if (sockets[i].closeWait) {
        send_return(sockets[i].closeWait, true);
        sockets[i].closeWait = NULL;
    }
    while (!sockets[i].acceptWait.empty()) {
        // free waiting accept requests
        // respond error
        send_return(sockets[i].acceptWait.front(), true);
        sockets[i].acceptWait.pop_front();
    }
    while (!sockets[i].listenWait.empty()) {
        // free sockets for accept
        Packet *p = sockets[i].listenWait.front();
        int id = p->anno_u8(SocketId);
        sockets[id].state = Nil;
        p->kill();
        sockets[i].listenWait.pop_front();
    }
}

void TcpFrontend::reset_socket(uint8_t i, bool rst) {
    if (rst) {
        send_short(i, Rst);
    }
    if (sockets[i].state == Listening || sockets[i].state == Closed) return;
    sockets[i].state = Closed;
    free_wait(i);
    back_close(i);
}

void TcpFrontend::push_socket(Packet *p) {
    uint8_t method = p->anno_u8(SocketMethod);
    if (method == New) {
        uint16_t port = p->anno_u16(SrcPort);
        if (find_bind_socket(port, false) >= 0) {
            Warn(":%d in use", port);
            send_return(p, true);
        } else {
            int i = find_empty_socket();
            sockets[i].state = Closed;
            sockets[i].src_port = port;
            p->set_anno_u8(SocketId, i);
            Log("new socket %d:%d", i, port);
            send_return(p, false);
        }
        return;
    }

    int i = p->anno_u8(SocketId);
    int n = sockets.size();
    if (i >= n || sockets[i].state == Nil) {
        p->set_anno_u8(SocketMethod, Error);
        Warn("operate on invalid socket %d", i);
        output(1).push(p);
        return;
    }

    Log("operate socket %d", i);
    // FSM switch
    switch (method) {
    case Listen:
        if (sockets[i].state != Closed) {
            Warn("listen called on open socket");
            send_return(p, true);
        } else {
            sockets[i].state = Listening;
            Log("listen");
            send_return(p, false);
        }
        break;
    case Accept:
        if (sockets[i].state != Listening) {
            Warn("accept called on non-listening socket");
            send_return(p, true);
        } else {
            Log("accept");
            queue_accept(i, p);
        }
        break;
    case Connect:
        if (sockets[i].state != Closed) {
            Warn("listen called on open socket");
            send_return(p, true);
        } else {
            sockets[i].state = Syn_Sent;
            sockets[i].dst_ip = p->anno_u32(SendIp);
            sockets[i].dst_port = p->anno_u16(DstPort);
            sockets[i].connectWait = p;
            Log("connect");
            send_short(i, Syn);
        }
        break;
    case Send:
        if (sockets[i].state != Established) {
            Warn("send called on non-open socket");
            send_return(p, true);
        } else {
            Log("send %d", p->length());
            output(2).push(p);
        }
        break;
    case Recv:
        if (sockets[i].state != Established) {
            Warn("send called on non-open socket");
            send_return(p, true);
        } else {
            Log("recv %d", p->length());
            output(2).push(p);
        }
        break;
    case Close:
        switch (sockets[i].state) {
        case Syn_Sent:
        case Listening:
            free_wait(i);
            sockets[i].state = Closed;
            Log("close");
            send_return(p, false);
            break;
        case Established:
            sockets[i].state = Fin_Wait1;
            sockets[i].closeWait = p;
            Log("close fin");
            back_close(i);
            send_short(i, Fin);
            break;
        case Close_Wait:
            sockets[i].state = Last_Ack;
            sockets[i].closeWait = p;
            Log("close wait");
            back_close(i);
            send_short(i, Fin);
            break;
        case Closed:
            Warn("duplicate close");
            send_return(p, false);
            break;
        default:
            Warn("close called on invalid state, reset");
            reset_socket(i);
            send_return(p, true);
        }
        break;
    case Free:
        if (sockets[i].state != Closed) {
            Warn("free called on open socket");
            send_return(p, true);
        } else {
            sockets[i].state = Nil;
            Log("free");
            send_return(p, false);
        }
        break;
    default:
        Warn("invalid method");
        send_return(p, true);
    }
}

void TcpFrontend::push_tcp(Packet *p) {
    const TcpHeader *tcp_p = (const TcpHeader *)p->data();

    if (!(tcp_p->flags & TcpMagic)) {
        Warn("invalid tcp packet");
        p->kill();
        return;
    }

    // our's sport is their's dport, our's dport is their's sport
    uint32_t ip = p->anno_u32(RecvIp);
    uint16_t sport = ntohs(tcp_p->dst_port);
    uint16_t dport = ntohs(tcp_p->src_port);
    uint16_t flag = tcp_p->flags;
    Log("tcp %08x:%d -> :%d, flag %04x", ip, dport, sport, flag);

    int i = find_socket(ip, sport, dport);
    if (i < 0) {
        i = find_bind_socket(sport);
    }
    if (i < 0) {
        Warn("no socket found");
        send_short(ip, sport, dport, Rst);
        p->kill();
        return;
    }

    Log("received socket %d", i);
    Log("%d", sockets[i].src_port);

    if (sockets[i].state == Closed) {
        Warn("received on closed");
        send_short(i, Rst);
        p->kill();
        return;
    }

    // FSM
    if (flag & Rst) {
        // RST
        Log("reset");
        reset_socket(i, false);
        p->kill();
    } else if ((flag & Syn) && (flag & Ack)) {
        // SYN ACK
        if (sockets[i].state == Syn_Sent) {
            sockets[i].state = Established;
            Log("establish");
            // inform backend
            back_establish(i);
            // respond previous connect call with success
            send_return(sockets[i].connectWait, false);
            // send back ack
            send_short(i, Ack);
            p->kill();
        } else {
            Warn("SYN ACK on invalid state");
            reset_socket(i);
            p->kill();
        }
    } else if (flag & Syn) {
        // SYN
        switch (sockets[i].state) {
        case Established:
            // data packet
            Log("data");
            output(2).push(p);
            break;
        case Listening:
            Log("syn to accept");
            Log("%d", sockets[i].src_port);
            create_accept(i, ip, dport);
            Log("%d", sockets[i].src_port);
            send_short(ip, sockets[i].src_port, dport, Syn | Ack);
            p->kill();
            break;
        default:
            Warn("SYN on invalid state");
            reset_socket(i);
            p->kill();
        }
    } else if ((flag & Fin) && (flag & Ack)) {
        // FIN ACK
        if (sockets[i].state == Fin_Wait1) {
            sockets[i].state = Closed;
            Log("FIN ACK on fin_wait1");
            send_return(sockets[i].closeWait, false);
            sockets[i].closeWait = NULL;
        } else {
            Warn("FIN ACK on invalid state");
            reset_socket(i);
        }
        p->kill();
    } else if (flag & Fin) {
        // FIN
        switch (sockets[i].state) {
        case Established:
            sockets[i].state = Close_Wait;
            Log("fin close");
            send_short(i, Ack);
            break;
        case Fin_Wait1:
            sockets[i].state = Closing;
            Log("FIN on fin_wait1");
            send_short(i, Ack);
            break;
        case Fin_Wait2:
            sockets[i].state = Closed;
            Log("FIN on fin_wait2");
            send_return(sockets[i].closeWait, false);
            sockets[i].closeWait = NULL;
            send_short(i, Ack);
            break;
        default:
            Warn("FIN on invalid state");
            reset_socket(i);
        }
        p->kill();
    } else if (flag & Ack) {
        // ACK
        int id0;
        switch (sockets[i].state) {
        case Established:
            Log("data ack");
            output(2).push(p);
            break;
        case Syn_Rcvd:
            id0 = find_bind_socket(sockets[i].src_port);
            if (id0 < 0) {
                Warn("ACK to closed listen socket");
                reset_socket(i);
            } else {
                sockets[i].state = Established;
                Log("establish");
                queue_listen(id0, i);
            }
            p->kill();
            break;
        case Fin_Wait1:
            sockets[i].state = Fin_Wait2;
            Log("ACK on fin_wait1");
            p->kill();
            break;
        case Closing:
            sockets[i].state = Closed;
            Log("ACK on closing");
            send_return(sockets[i].closeWait, false);
            sockets[i].closeWait = NULL;
            p->kill();
            break;
        default:
            Warn("Ack on invalid state");
            reset_socket(i);
            p->kill();
        }
    }
}

void TcpFrontend::push(int port, Packet *p) {
    if (!port) {
        if (p->anno_u8(RecvProto) != IpProtoTcp) {
            Warn("unknown packet");
            p->kill();
        } else {
            push_tcp(p);
        }
    } else {
        push_socket(p);
    }
}

void TcpFrontend::back_close(uint8_t i) {
    Packet *q = Packet::make(0);
    q->set_anno_u8(SocketMethod, Close);
    q->set_anno_u8(SocketId, i);
    output(2).push(q);
}

void TcpFrontend::back_establish(uint8_t i) {
    Packet *q = Packet::make(0);
    q->set_anno_u8(SocketMethod, Connect);
    q->set_anno_u8(SocketId, i);
    q->set_anno_u32(RecvIp, sockets[i].dst_ip);
    q->set_anno_u16(SrcPort, sockets[i].src_port);
    q->set_anno_u16(DstPort, sockets[i].dst_port);
    output(2).push(q);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(TcpFrontend)
