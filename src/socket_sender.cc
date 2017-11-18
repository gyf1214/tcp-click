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
    "DATA", cpkM + cpkP, cpString, &data,
    "INTERVAL", cpkM + cpkP, cpTimestamp, &interval,
    "WAIT", cpkM + cpkP, cpTimestamp, &wait,
    "LIMIT", cpkM + cpkP, cpInteger, &limit,
    "BUFFER", cpkM + cpkP, cpInteger, &buffer, cpEnd) < 0) {
        return -1;
    }
    limit *= data.length();
    return 0;
}

int SocketSender::initialize(ErrorHandler *) {
    timer.initialize(this);
    timer.schedule_after(wait);
    return 0;
}

WritablePacket *SocketSender::send_next() {
    int len = buffer;
    len = limit - offset > len ? len : limit - offset;

    WritablePacket *q = SocketPacket(Send, id, ++sequence, len);
    int n = data.length();

    int i = offset % n;
    if (i > len) {
        memcpy(q->data(), data.c_str() + n - i, len);
        len = 0;
        i = len;
    }

    if (i) {
        memcpy(q->data(), data.c_str() + n - i, i);
        len -= i;
    }

    for (; len >= n; len -= n) {
        memcpy(q->data() + i, data.c_str(), n);
        i += n;
    }

    if (len) {
        memcpy(q->data() + i, data.c_str(), len);
        i += len;
    }
    offset += i;

    return q;
}

void SocketSender::run_timer(Timer *) {
    WritablePacket *q = NULL;

    switch (state) {
    case Nothing:
        q = SocketPacket(New, 0, ++sequence);
        q->set_anno_u16(SrcPort, sport);
        Log("%d -> new :%d", sequence, sport);
        break;
    case Start:
        q = SocketPacket(Connect, id, ++sequence);
        q->set_anno_u32(SendIp, ip);
        q->set_anno_u16(DstPort, dport);
        Log("%d -> connect %08x:%d", sequence, ip, dport);
        // timer.reschedule_after(timeout);
        break;
    case Writing:
        if (offset >= limit) {
            state = Closing;
            q = SocketPacket(Close, id, sequence);
            Log("%d -> close %d", sequence, id);
        } else {
            q = send_next();
            Log("%d -> send %d", sequence, q->length());
        }
        break;
    case Closing:
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

    if (p->anno_u32(SocketSequence) != sequence) {
        Warn("old response");
    } else if (method == Error) {
        if (state == Writing || state == Closing || state == Start) {
            state = state == Closing ? Start : Closing;
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
    } else if (state == Start && method == Connect) {
        state = Writing;
        offset = 0;
        Log("%d <- connect", sequence);
        timer.schedule_now();
    } else if (state == Writing && method == Send) {
        Log("%d <- send, total %d", sequence, offset);
        timer.schedule_after(interval);
    } else if (state == Closing && method == Close) {
        state = Start;
        Log("%d <- close", sequence);
    } else {
        Warn("unknown response");
    }

    p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SocketSender);
