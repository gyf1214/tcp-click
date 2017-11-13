#include "tcp_backend.hh"
#include "tcp_socket.hh"
#include "tcp_packet.hh"
#include "ip_packet.hh"
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
    tcb[i].swnd.timer.assign();
    tcb[i].swnd.timer.clear();
}

void TcpBackend::return_error(Packet *p) {
    Packet *q = SocketPacket(p->anno_u8(SocketMethod),
    p->anno_u8(SocketId), p->anno_u8(SocketSequence));
    p->kill();
    output(1).push(q);
}

bool TcpBackend::try_grow_send(uint8_t i) {
    TcpSendWindow &swnd = tcb[i].swnd;

    uint32_t grow = swnd.max_grow();
    if (!grow) {
        return false;
    }
    // grow window and send
    WritablePacket *q = Packet::make(TcpSize + grow);
    TcpHeader *tcp_q = (TcpHeader *)q->data();
    // make packet
    q->set_anno_u8(SendProto, IpProtoTcp);
    q->set_anno_u32(SendIp, IpProtoTcp);
    tcp_q->syn(tcb[i].sport, tcb[i].dport, swnd.seq_back);
    TcpFromWnd(tcp_q->data, swnd.buf, TcpBufferSize, swnd.seq_back, swnd.seq_back + grow);
    // update pointer
    swnd.seq_back += grow;
    // send
    output(0).push(q);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(TcpBackend);
