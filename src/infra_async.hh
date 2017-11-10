// InfraAsync is a async processor
#ifndef __CLICK_INFRA_ASYNC
#define __CLICK_INFRA_ASYNC
#include <click/config.h>
#include <click/element.hh>
#include <click/task.hh>
#include <queue>
CLICK_DECLS

class InfraAsync : public Element {
    std::queue<std::pair<int, Packet *> > que;
    Task task;
protected:
    virtual void do_work(int, Packet *) = 0;
public:
    InfraAsync();
    int initialize(ErrorHandler *);
    void push(int, Packet *);
    bool run_task(Task *);
};

CLICK_ENDDECLS
#endif
