#include "lsp_backend.hh"
#include "lsp_packet.hh"
#include "ip_packet.hh"
#include "infra_log.hh"
#include <click/args.hh>
#include <click/error.hh>
using namespace std;
CLICK_DECLS

// infi distance == default ttl;
const int infi = IpTTL;

int LspBackend::configure(Vector<String> &args, ErrorHandler *errh) {
    String ip_str;
    if (Args(args, this, errh)
    .read_mp("IP", ip_str)
    .complete() < 0) {
        return -1;
    }

    if (!IPAddressArg().parse(ip_str, (struct in_addr &)self, this)) {
        return errh->error("IP should be ip address");
    }
    return 0;
}

void LspBackend::do_work(int, Packet *p) {
    const IpHeader *ip = (const IpHeader *)p->data();
    const LspHeader *lsp = (const LspHeader *)ip->data;
    const LspSequenceData *seq = lsp->data;

    // update connection table
    int n = conn.size();
    bool tf = false;
    for (int i = 0; i < n; ++i) {
        if (conn[i].first == ip->src) {
            conn[i].second.assign(seq->entry, seq->entry + seq->count);
            tf = true;
            break;
        }
    }

    // insert if unknown ip
    if (!tf) {
        conn.push_back(make_pair(ip->src,
        vector<uint32_t>(seq->entry, seq->entry + seq->count)));
    }

    p->kill();
    // dijkstra
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

    // find self id
    int k = find_ip(self);
    if (k < 0) {
        Warn("self not found in conn table");
        return;
    }

    // init self dis, first hop == -1
    dis[k] = make_pair(0, -1);
    vis[k] = true;

    // find direct neighbour k, first hop == port of k
    int m = conn[k].second.size();
    for (int i = 0; i < m; ++i) {
        int v = find_ip(conn[k].second[i]);
        if (v >= 0) {
            dis[v] = make_pair(1, i);
        }
    }

    for (int t = 1; t < n; ++t) {
        // find new node
        int best = infi;
        int u = -1;
        for (int i = 0; i < n; ++i) {
            if (!vis[i] && dis[i].first < best) {
                best = dis[i].first;
                u = i;
            }
        }
        // end on no new nodes connected
        if (u < 0) {
            break;
        }

        // relax & update first hop
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

    Log("finish dijkstra");
    for (int i = 0; i < n; ++i) {
        Log("%08x -> %d", conn[i].first, dis[i].second);
    }
}

Packet *LspBackend::pull(int) {
    // construct routing packet based on first hop
    int n = dis.size();
    WritablePacket *p = Packet::make(LspSizeRouting + n * LspSizeRoutingEntry);
    LspRouting *table = (LspRouting *)p->data();
    table->count = n;
    for (int i = 0; i < n; ++i) {
        table->entry[i].ip = conn[i].first;
        table->entry[i].port = dis[i].second;
    }

    return p;
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(InfraAsync)
EXPORT_ELEMENT(LspBackend)
