#include "socket_sender.hh"
#include "infra_anno.hh"
#include "tcp_socket.hh"
#include "infra_log.hh"
#include <click/confparse.hh>
CLICK_DECLS

SocketSender::SocketSender() : timer(this), state(Nothing), sequence(0) {}

int SocketSender::configure(Vector<String> &args, ErrorHandler *errh) {
    if (cp_va_kparse(args, this, errh,
    "IP", cpkM + cpkP, cpIPAddress, &self,
    "DST", cpkM + cpkP, cpIPAddress, &ip,
    "SPORT", cpkM + cpkP, cpUnsignedShort, &sport,
    "DPORT", cpkM + cpkP, cpUnsignedShort, &dport,
    "INTERVAL", cpkM + cpkP, cpTimestamp, &interval,
    "TIMEOUT", cpkM + cpkP, cpTimestamp, &timeout, cpEnd) < 0) {
        return -1;
    }
    return 0;
}

int SocketSender::initialize(ErrorHandler *) {
    timer.initialize(this);
    timer.schedule_after(timeout);
    return 0;
}

void SocketSender::run_timer(Timer *) {
    WritablePacket *q = NULL;
    switch (state) {
    case Nothing:
        q = SocketPacket(New, 0, ++sequence);
        q->set_anno_u16(SrcPort, sport);
        Log("%d -> new %d", sequence, sport);
        break;
    case Start:
        q = SocketPacket(Connect, id, ++sequence);
        q->set_anno_u32(SendIp, ip);
        q->set_anno_u16(DstPort, dport);
        Log("%d -> connect %08x:%d", sequence, ip, dport);
        // timer.reschedule_after(timeout);
        break;
    case Waiting:
        q = SocketPacket(Close, id, ++sequence);
        Log("%d -> close %d", sequence, id);
        // timer.reschedule_after(timeout);
        break;
    default:
        Warn("unknown state");
    }
    if (q) {
        output(0).push(q);
    }
}

void SocketSender::push(int, Packet *p) {
    uint8_t method = p->anno_u8(SocketMethod);

    if (p->anno_u8(SocketSequence) != sequence) {
        Warn("old response");
    } else if (method == Error) {
        Warn("%d <- error", sequence);
        state = Error;
    } else if (state == Nothing && method == New) {
        state = Start;
        id = p->anno_u8(SocketId);
        Log("%d <- new %d", sequence, id);
        timer.schedule_now();
    } else if (state == Start && method == Connect) {
        state = Waiting;
        Log("%d <- connect", sequence);
        timer.reschedule_after(interval);
    } else if (state == Waiting && method == Close) {
        state = Start;
        Log("%d <- close", sequence);
        timer.reschedule_after(interval);
    } else {
        Warn("unknown response");
    }

    p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SocketSender);
