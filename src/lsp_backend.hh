// Link State Protocol Backend
// input(0) : incoming sequence from frontend
// output(0) : pulling port to query
// IP : self ip
#ifndef __CLICK_LSP_BACKEND
#define __CLICK_LSP_BACKEND
#include <click/config.h>
#include "infra_async.hh"
#include <vector>
CLICK_DECLS

class LspBackend : public InfraAsync {
    // connection table
    // id -> <src_ip, [dst_ip1, dst_ip2, ...]>
    std::vector<std::pair<uint32_t, std::vector<uint32_t> > > conn;
    // distance in dijkstra
    // id -> <distance, first hop>
    std::vector<std::pair<int, int> > dis;
    // visit array in dijkstra
    std::vector<bool> vis;
    // self ip
    uint32_t self;
protected:
    // find id from conn by ip
    int find_ip(uint32_t);
    // dijkstra
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
