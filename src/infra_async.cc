#include "infra_async.hh"
using namespace std;

CLICK_DECLS

void InfraAsync::push(int port, Packet *p) {
    if (!task.scheduled()) {
        task.reschedule();
    }
    que.push(make_pair(port, p));
}

bool InfraAsync::run_task(Task *) {
    if (que.empty()) return false;
    pair<int, Packet *> p = que.front();
    que.pop();
    do_work(p.first, p.second);
    if (!que.empty()) task.fast_reschedule();
    return true;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(InfraAsync)
