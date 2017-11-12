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

void TcpFrontend::send_short(TcpSocket &sock, uint16_t flags) {
    send_short(sock.dst_ip, sock.src_port, sock.dst_port, flags);
}

void TcpFrontend::create_accept(TcpSocket &sock, uint32_t ip, uint16_t port) {
    Log("%d", sock.src_port);
    int id = find_empty_socket();
    Log("%d", sock.src_port);
    TcpSocket &sock2 = sockets[id];
    sock2.dst_ip = ip;
    sock2.dst_port = port;
    Log("%d", sock.src_port);
    sock2.src_port = sock.src_port;
    Log("%d", sock.src_port);
    sock2.state = Syn_Rcvd;
    Log("%d", sock.src_port);
}

void TcpFrontend::queue_accept(TcpSocket &sock, Packet *p) {
    if (sock.listenWait.empty()) {
        sock.acceptWait.push_back(p);
    } else {
        Packet *q = sock.listenWait.front();
        sock.listenWait.pop_front();
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
    TcpSocket &sock = sockets[id0];

    if (sock.acceptWait.empty()) {
        sock.listenWait.push_back(q);
    } else {
        Packet *p = sock.acceptWait.front();
        sock.acceptWait.pop_front();
        q->set_anno_u32(SocketSequence, p->anno_u32(SocketSequence));
        output(1).push(q);
        p->kill();
    }
}

void TcpFrontend::free_wait(TcpSocket &sock) {
    if (sock.connectWait) {
        send_return(sock.connectWait, true);
        sock.connectWait = NULL;
    }
    if (sock.closeWait) {
        send_return(sock.closeWait, true);
        sock.closeWait = NULL;
    }
    while (!sock.acceptWait.empty()) {
        // free waiting accept requests
        // respond error
        send_return(sock.acceptWait.front(), true);
        sock.acceptWait.pop_front();
    }
    while (!sock.listenWait.empty()) {
        // free sockets for accept
        Packet *p = sock.listenWait.front();
        int id = p->anno_u8(SocketId);
        TcpSocket &sock2 = sockets[id];
        sock2.state = Nil;
        p->kill();
        sock.listenWait.pop_front();
    }
}

void TcpFrontend::reset_socket(uint8_t id, bool rst) {
    TcpSocket &sock = sockets[id];
    if (rst) {
        send_short(sock, Rst);
    }
    if (sock.state == Listening || sock.state == Closed) return;
    sock.state = Closed;
    free_wait(sock);
    back_close(id);
}

void TcpFrontend::push_socket(Packet *p) {
    uint8_t method = p->anno_u8(SocketMethod);
    if (method == New) {
        uint16_t port = p->anno_u16(SrcPort);
        if (find_bind_socket(port, false) >= 0) {
            Warn(":%d in use", port);
            send_return(p, true);
        } else {
            int id = find_empty_socket();
            sockets[id].state = Closed;
            sockets[id].src_port = port;
            p->set_anno_u8(SocketId, id);
            Log("new socket %d:%d", id, port);
            send_return(p, false);
        }
        return;
    }

    int id = p->anno_u8(SocketId);
    int n = sockets.size();
    if (id >= n || sockets[id].state == Nil) {
        p->set_anno_u8(SocketMethod, Error);
        Warn("operate on invalid socket %d", id);
        output(1).push(p);
        return;
    }

    Log("operate socket %d", id);
    TcpSocket &sock = sockets[id];
    // FSM switch
    switch (method) {
    case Listen:
        if (sock.state != Closed) {
            Warn("listen called on open socket");
            send_return(p, true);
        } else {
            sock.state = Listening;
            Log("listen");
            send_return(p, false);
        }
        break;
    case Accept:
        if (sock.state != Listening) {
            Warn("accept called on non-listening socket");
            send_return(p, true);
        } else {
            Log("accept");
            queue_accept(sock, p);
        }
        break;
    case Connect:
        if (sock.state != Closed) {
            Warn("listen called on open socket");
            send_return(p, true);
        } else {
            sock.state = Syn_Sent;
            sock.dst_ip = p->anno_u32(SendIp);
            sock.dst_port = p->anno_u16(DstPort);
            sock.connectWait = p;
            Log("connect");
            send_short(sock, Syn);
        }
        break;
    case Send:
        if (sock.state != Established) {
            Warn("send called on non-open socket");
            send_return(p, true);
        } else {
            Log("send %d", p->length());
            output(2).push(p);
        }
        break;
    case Recv:
        if (sock.state != Established) {
            Warn("send called on non-open socket");
            send_return(p, true);
        } else {
            Log("recv %d", p->length());
            output(2).push(p);
        }
        break;
    case Close:
        switch (sock.state) {
        case Syn_Sent:
        case Listening:
            free_wait(sock);
            sock.state = Closed;
            Log("close");
            send_return(p, false);
            break;
        case Established:
            sock.state = Fin_Wait1;
            sock.closeWait = p;
            Log("close fin");
            back_close(id);
            send_short(sock, Fin);
            break;
        case Close_Wait:
            sock.state = Last_Ack;
            sock.closeWait = p;
            Log("close wait");
            back_close(id);
            send_short(sock, Fin);
            break;
        case Closed:
            Warn("duplicate close");
            send_return(p, false);
            break;
        default:
            Warn("close called on invalid state, reset");
            reset_socket(id);
            send_return(p, true);
        }
        break;
    case Free:
        if (sock.state != Closed) {
            Warn("free called on open socket");
            send_return(p, true);
        } else {
            sock.state = Nil;
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

    int id = find_socket(ip, sport, dport);
    if (id < 0) {
        id = find_bind_socket(sport);
    }
    if (id < 0) {
        Warn("no socket found");
        send_short(ip, sport, dport, Rst);
        p->kill();
        return;
    }

    Log("received socket %d", id);
    TcpSocket &sock = sockets[id];
    Log("%d", sock.src_port);

    if (sock.state == Closed) {
        Warn("received on closed");
        send_short(sock, Rst);
        p->kill();
        return;
    }

    // FSM
    if (flag & Rst) {
        // RST
        Log("reset");
        reset_socket(id, false);
        p->kill();
    } else if ((flag & Syn) && (flag & Ack)) {
        // SYN ACK
        if (sock.state == Syn_Sent) {
            sock.state = Established;
            Log("establish");
            // inform backend
            back_establish(id);
            // respond previous connect call with success
            send_return(sock.connectWait, false);
            // send back ack
            send_short(sock, Ack);
            p->kill();
        } else {
            Warn("SYN ACK on invalid state");
            reset_socket(id);
            p->kill();
        }
    } else if (flag & Syn) {
        // SYN
        switch (sock.state) {
        case Established:
            // data packet
            Log("data");
            output(2).push(p);
            break;
        case Listening:
            Log("syn to accept");
            Log("%d", sock.src_port);
            create_accept(sock, ip, dport);
            Log("%d", sock.src_port);
            send_short(ip, sock.src_port, dport, Syn | Ack);
            p->kill();
            break;
        default:
            Warn("SYN on invalid state");
            reset_socket(id);
            p->kill();
        }
    } else if ((flag & Fin) && (flag & Ack)) {
        // FIN ACK
        if (sock.state == Fin_Wait1) {
            sock.state = Closed;
            Log("FIN ACK on fin_wait1");
            send_return(sock.closeWait, false);
            sock.closeWait = NULL;
        } else {
            Warn("FIN ACK on invalid state");
            reset_socket(id);
        }
        p->kill();
    } else if (flag & Fin) {
        // FIN
        switch (sock.state) {
        case Established:
            sock.state = Close_Wait;
            Log("fin close");
            send_short(sock, Ack);
            break;
        case Fin_Wait1:
            sock.state = Closing;
            Log("FIN on fin_wait1");
            send_short(sock, Ack);
            break;
        case Fin_Wait2:
            sock.state = Closed;
            Log("FIN on fin_wait2");
            send_return(sock.closeWait, false);
            sock.closeWait = NULL;
            send_short(sock, Ack);
            break;
        default:
            Warn("FIN on invalid state");
            reset_socket(id);
        }
        p->kill();
    } else if (flag & Ack) {
        // ACK
        int id0;
        switch (sock.state) {
        case Established:
            Log("data ack");
            output(2).push(p);
            break;
        case Syn_Rcvd:
            id0 = find_bind_socket(sock.src_port);
            if (id0 < 0) {
                Warn("ACK to closed listen socket");
                reset_socket(id);
            } else {
                sock.state = Established;
                Log("establish");
                queue_listen(id0, id);
            }
            p->kill();
            break;
        case Fin_Wait1:
            sock.state = Fin_Wait2;
            Log("ACK on fin_wait1");
            p->kill();
            break;
        case Closing:
            sock.state = Closed;
            Log("ACK on closing");
            send_return(sock.closeWait, false);
            sock.closeWait = NULL;
            p->kill();
            break;
        default:
            Warn("Ack on invalid state");
            reset_socket(id);
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

void TcpFrontend::back_close(uint8_t id) {
    Packet *q = Packet::make(0);
    q->set_anno_u8(SocketMethod, Close);
    q->set_anno_u8(SocketId, id);
    output(2).push(q);
}

void TcpFrontend::back_establish(uint8_t id) {
    TcpSocket &sock = sockets[id];
    Packet *q = Packet::make(0);
    q->set_anno_u8(SocketMethod, Connect);
    q->set_anno_u8(SocketId, id);
    q->set_anno_u32(RecvIp, sock.dst_ip);
    q->set_anno_u16(SrcPort, sock.src_port);
    q->set_anno_u16(DstPort, sock.dst_port);
    output(2).push(q);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(TcpFrontend)
