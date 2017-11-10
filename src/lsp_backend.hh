#ifndef __CLICK_LSP_BACKEND
#define __CLICK_LSP_BACKEND
#include <click/config.h>
#include "infra_async.hh"
#include <vector>
CLICK_DECLS

class LspBackend : public InfraAsync {
    std::vector<std::pair<uint32_t, std::vector<uint32_t> > > conn;
    std::vector<int> dis;
protected:
    void updateConn(std::vector<uint32_t> &, int, uint32_t *);
    void dijkstra();
    void do_work(int, Packet *);
};

CLICK_ENDDECLS
#endif
