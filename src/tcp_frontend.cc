#include "tcp_frontend.hh"
#include "infra_anno.hh"
#include "infra_log.hh"
CLICK_DECLS

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

int TcpFrontend::find_empty_socket() {
    int n = sockets.size();
    for (int i = 0; i < n; ++i) {
        if (sockets[i].state == Nil) {
            return i;
        }
    }
    TcpSocket s;
    s.state = Nil;
    sockets.push_back(s);
    return n;
}

void TcpFrontend::push_socket(uint8_t method, Packet *p) {
    if (method == New) {
        int id = find_empty_socket();
        sockets[id].state = Closed;
        p->set_anno_u8(SocketId, id);
        p->set_anno_u8(SocketMethod, Return);
        Log("new socket %d", id);
        output(1).push(p);
        return;
    }

    int id = p->anno_u8(SocketId);
    if (id >= sockets.size() || sockets[id].state == Nil) {
        p->set_anno_u8(SocketMethod, Error);
        Warn("operate on invalid socket %d", id);
        output(1).push(p);
        return;
    }

    Log("socket %d", id);
    switch ((TcpSocketMethod)method) {
    case Bind:
        if (sockets[id].state != Closed) {
            p->set_anno_u8(SocketMethod, Error);
            Warn("bind called on open socket");
            output(1).push(p);
        } else {
            sockets[id].src_port = p->anno_u16(SocketPort);
            p->set_anno_u8(SocketMethod, Return);
            Log("bind to %d", sockets[id].src_port);
            output(1).push(p);
        }
        break;
    case Listen:
        sockets[id].state = Listening;
        p->set_anno_u8(SocketMethod, Return);
        Log("listen");
        output(1).push(p);
        break;
    case Accept:
        if (!sockets[id].listenWait.empty()) {

        }
        break;
    }
}

CLICK_ENDDECLS
// EXPORT_ELEMENT(TcpFrontend)
