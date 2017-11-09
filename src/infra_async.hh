// InfraAsync is a async processor
#ifndef __CLICK_INFRA_ASYNC
#define __CLICK_INFRA_ASYNC
#include <queue>
#include <click/config.h>
#include <click/element.hh>
#include <click/task.hh>
CLICK_DECLS

class InfraAsync : public Element {
    std::queue<std::pair<int, Packet *> > que;
    Task task;
protected:
    virtual void do_work(int, Packet *) = 0;
public:
    void push(int, Packet *);
    bool run_task(Task *);
    virtual Packet *pull(int port) = 0;
};

CLICK_ENDDECLS
#endif
