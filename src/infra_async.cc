#include "infra_async.hh"
#include <click/standard/scheduleinfo.hh>
CLICK_DECLS

InfraAsync::InfraAsync() : task(this) {}

int InfraAsync::initialize(ErrorHandler *errh) {
    ScheduleInfo::initialize_task(this, &task, false, errh);
    return 0;
}

void InfraAsync::push(int port, Packet *p) {
    if (!task.scheduled()) {
        task.reschedule();
    }
    que.push_back(make_pair(port, p));
}

bool InfraAsync::run_task(Task *) {
    if (que.empty()) return false;
    Pair<int, Packet *> p = que.front();
    que.pop_front();
    do_work(p.first, p.second);
    if (!que.empty()) task.fast_reschedule();
    return true;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(InfraAsync)
