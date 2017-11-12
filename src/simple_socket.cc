#include "simple_socket.hh"
#include "infra_anno.hh"
#include "tcp_socket.hh"
#include <click/args.hh>
#include <click/error.hh>
#include <click/standard/scheduleinfo.hh>
CLICK_DECLS

SimpleSocket::SimpleSocket() : task(this), sequence(0) {}

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
    ScheduleInfo::initialize_task(this, &task, true, errh);
    return 0;
}

void SimpleSocket::socket(uint16_t port) {
    Packet *p = Packet::make(0);
    p->set_anno_u8(SocketMethod, New);
    p->set_anno_u16(SrcPort, port);
    p->set_anno_u32(SocketSequence, sequence);
    output(0).push(p);
}

bool SimpleSocket::run_task(Task *) {
    return true;
}

void SimpleSocket::push(int, Packet *p) {
    ++sequence;
    last_ret = p;
    task.reschedule();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SimpleSocket)