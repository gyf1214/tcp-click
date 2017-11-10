// Link State Protocol Backend
// input(0) : incoming sequence from frontend
// output(0) : pulling port to query
#ifndef __CLICK_LSP_BACKEND
#define __CLICK_LSP_BACKEND
#include <click/config.h>
#include "infra_async.hh"
#include <vector>
CLICK_DECLS

class LspBackend : public InfraAsync {
    std::vector<std::pair<uint32_t, std::vector<uint32_t> > > conn;
    std::vector<std::pair<int, int> > dis;
    std::vector<bool> vis;
    uint32_t self;
protected:
    int find_ip(uint32_t);
    void dijkstra();
    void do_work(int, Packet *);
public:
    const char *class_name() const { return "LspBackend"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return PUSH_TO_PULL; }
    int configure(Vector<String> &, ErrorHandler *);
    Packet *pull(int);
};

CLICK_ENDDECLS
#endif
