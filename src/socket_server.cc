#include "socket_server.hh"
#include "infra_anno.hh"
#include "tcp_socket.hh"
#include "infra_log.hh"
#include <click/confparse.hh>
CLICK_DECLS

SocketServer::SocketServer() : timer(this), state(Nothing), sequence(0) {}

int SocketServer::configure(Vector<String> &args, ErrorHandler *errh) {
    if (cp_va_kparse(args, this, errh,
    "IP", cpkM + cpkP, cpIPAddress, &self,
    "PORT", cpkM + cpkP, cpUnsignedShort, &port,
    "INTERVAL", cpkM + cpkP, cpTimestamp, &interval,
    "WAIT", cpkM + cpkP, cpTimestamp, &wait,
    "BUFFER", cpkM + cpkP, cpInteger, &buffer, cpEnd) < 0) {
        return -1;
    }
    return 0;
}

int SocketServer::initialize(ErrorHandler *) {
    timer.initialize(this);
    timer.schedule_after(wait);
    return 0;
}

void SocketServer::run_timer(Timer *) {
    WritablePacket *q = NULL;
    switch (state) {
    case Nothing:
        q = SocketPacket(New, 0, ++sequence);
        q->set_anno_u16(SrcPort, port);
        Log("%d -> new :%d", sequence, port);
        break;
    case Start:
        q = SocketPacket(Listen, id, ++sequence);
        Log("%d -> listen", sequence);
        break;
    case Listened:
        q = SocketPacket(Accept, id, ++sequence);
        Log("%d -> accept %d", sequence, id);
        break;
    case Reading:
        q = SocketPacket(Recv, id1, ++sequence, buffer);
        Log("%d -> recv %d", sequence, buffer);
        break;
    case Closing:
        q = SocketPacket(Close, id1, ++sequence);
        Log("%d -> close %d", sequence, id1);
        break;
    case AcceptClose:
        q = SocketPacket(Free, id1, ++sequence);
        Log("%d -> free %d", sequence, id1);
        break;
    default:
        Warn("unknown state");
    }
    if (q) {
        output(0).push(q);
    }
}

void SocketServer::push(int, Packet *p) {
    uint8_t method = p->anno_u8(SocketMethod);

    if (p->anno_u8(SocketSequence) != sequence) {
        Warn("old response");
    } else if (method == Error) {
        if (state == Reading || state == Closing) {
            state = state == Closing ? AcceptClose : Closing;
            Log("%d <- error (connection close)", sequence);
            timer.schedule_now();
        } else {
            Warn("%d <- error (unknown)", sequence);
            state = Err;
        }
    } else if (state == Nothing && method == New) {
        state = Start;
        id = p->anno_u8(SocketId);
        Log("%d <- new %d", sequence, id);
        timer.schedule_now();
    } else if (state == Start && method == Listen) {
        state = Listened;
        Log("%d <- listen", sequence);
        timer.schedule_now();
    } else if (state == Listened && method == Accept) {
        state = Reading;
        total = 0;
        id1 = p->anno_u8(SocketId);
        Log("%d <- accept %d", sequence, id1);
        timer.schedule_now();
    } else if (state == Reading && method == Recv) {
        total += p->length();
        Log("%d <- recv %d, total %d", sequence, p->length(), total);
        timer.schedule_after(interval);
    } else if (state == Closing && method == Close) {
        state = AcceptClose;
        Log("%d <- close", sequence);
        timer.schedule_now();
    } else if (state == AcceptClose && method == Free) {
        state = Listened;
        Log("%d <- free", sequence);
        timer.schedule_now();
    } else {
        Warn("unknown response");
    }

    p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SocketServer);
