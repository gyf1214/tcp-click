#include "simple_socket.hh"
#include "infra_anno.hh"
#include "infra_log.hh"
#include "tcp_socket.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/standard/scheduleinfo.hh>
#include <click/confparse.hh>
#include <click/straccum.hh>
#include <cstring>
CLICK_DECLS

SimpleSocket::SimpleSocket() : sequence(0) {}

int SimpleSocket::configure(Vector<String> &conf, ErrorHandler *errh) {
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

int SimpleSocket::initialize(ErrorHandler *errh) {
    return 0;
}

void SimpleSocket::exec(Packet *p, uint8_t method) {
    p->set_anno_u8(SocketMethod, method);
    p->set_anno_u32(SocketSequence, ++sequence);
    output(0).push(p);
}

void SimpleSocket::exec(Packet *p, uint8_t sock, uint8_t method) {
    p->set_anno_u8(SocketId, sock);
    exec(p, method);
}

void SimpleSocket::socket(uint16_t port) {
    Packet *p = Packet::make(0);
    p->set_anno_u16(SrcPort, port);
    exec(p, New);
}

void SimpleSocket::connect(uint8_t id, uint32_t ip, uint16_t port) {
    Packet *p = Packet::make(0);
    p->set_anno_u8(SocketId, id);
    p->set_anno_u32(SendIp, ip);
    p->set_anno_u16(SrcPort, port);
    exec(p, Connect);
}

void SimpleSocket::send_info(const char *str) {
    int n = strlen(str);
    WritablePacket *p = Packet::make(n + 1);
    memcpy(p->data(), str, n);
    p->data()[n] = '\n';
    output(1).push(p);
}

void SimpleSocket::send_info(const String &str) {
    int n = str.length();
    WritablePacket *p = Packet::make(n + 1);
    memcpy(p->data(), str.data(), n);
    p->data()[n] = '\n';
    output(1).push(p);
}

void SimpleSocket::push_return(Packet *p) {
    StringAccum sa;
    sa << p->anno_u32(SocketSequence) << " | ";
    switch (p->anno_u8(SocketMethod)) {
    case Error:
        sa << "error";
        break;
    case New:
        sa << "socket " << (int)p->anno_u8(SocketId) << "";
        break;
    default:
        sa << "returns";
    }
    p->kill();
    send_info(sa.take_string());
}

void SimpleSocket::push(int port, Packet *p) {
    if (!port) {
        push_return(p);
        return;
    }
    String str(p->data(), p->length());
    String cmd = cp_shift_spacevec(str);
    if (cmd == "socket") {
        int port;
        if (!cp_integer(cp_shift_spacevec(str), &port)) {
            send_info("port number error");
            p->kill();
            return;
        }
        socket(port);
    } else {
        int sock;
        if (!cp_integer(cp_shift_spacevec(str), &sock)) {
            send_info("sock id error");
            p->kill();
            return;
        }
        if (cmd == "connect") {
            uint32_t ip;
            int port;
            if (!cp_ip_address(cp_shift_spacevec(str), (unsigned char*)&ip)) {
                send_info("ip error");
                p->kill();
                return;
            }
            if (!cp_integer(cp_shift_spacevec(str), &port)) {
                send_info("port error");
                p->kill();
                return;
            }
            connect(sock, ip, port);
        }
    }
    p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SimpleSocket)
