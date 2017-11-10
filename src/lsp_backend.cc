#include "lsp_backend.hh"
#include "lsp_packet.hh"
#include "ip_packet.hh"
using namespace std;
CLICK_DECLS

const int infi = 100000000;

void LspBackend::do_work(int, Packet *p) {
    const IpHeader *ip = (const IpHeader *)p->data();
    const LspHeader *lsp = (const LspHeader *)ip->data;
    const LspSequenceData *seq = lsp->data;

    int n = conn.size();
    bool tf = false;
    for (int i = 0; i < n; ++i) {
        if (conn[i].first == ip->src) {
            updateConn(conn[i].second, seq->count, seq->entry)
            tf = true;
            break;
        }
    }
    if (!tf) {
        conn.push_back(make_pair(ip->src, vector<uint32_t>()));
    }
}

void LspBackend::updateConn(vector<uint32_t> &v, int n, uint32_t *ips) {
    v.resize(n);
    for (int i = 0; i < n; ++i) {
        v[i] = ips[i];
    }
}

void LspBackend::dijkstra() {
    
}

CLICK_ENDDECLS
