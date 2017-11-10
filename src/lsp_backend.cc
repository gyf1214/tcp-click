#include "lsp_backend.hh"
#include "lsp_packet.hh"
#include "ip_packet.hh"
#include "infra_log.hh"
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
            conn[i].second.assign(seq->entry, seq->entry + seq->count);
            tf = true;
            break;
        }
    }
    if (!tf) {
        conn.push_back(make_pair(ip->src,
        vector<uint32_t>(seq->entry, seq->entry + seq->count)));
    }

    p->kill();
    dijkstra();
}

int LspBackend::find_ip(uint32_t ip) {
    int n = conn.size();
    for (int i = 0; i < n; ++i) {
        if (conn[i].first == ip) {
            return i;
        }
    }
    return -1;
}

void LspBackend::dijkstra() {
    int n = conn.size();
    dis.assign(n, make_pair(infi, -1));
    vis.assign(n, false);

    int k = find_ip(self);
    if (k < 0) {
        Warn("self not found in conn table");
        return;
    }

    dis[k] = make_pair(0, -1);
    vis[k] = true;

    int m = conn[k].second.size();
    for (int i = 0; i < m; ++i) {
        int v = find_ip(conn[k].second[i]);
        if (v >= 0) {
            dis[v] = make_pair(1, v);
        }
    }

    for (int t = 1; t < n; ++t) {
        int best = infi;
        int u = -1;
        for (int i = 0; i < n; ++i) {
            if (!vis[i] && dis[i].first < best) {
                best = dis[i].first;
                u = i;
            }
        }
        if (u < 0) {
            break;
        }
        vis[u] = true;
        m = conn[u].second.size();
        for (int i = 0; i < m; ++i) {
            int v = find_ip(conn[u].second[i]);
            if (v >= 0 && dis[v].first > dis[u].first + 1) {
                dis[v].first = dis[u].first + 1;
                dis[v].second = dis[u].second;
            }
        }
    }
}

CLICK_ENDDECLS
