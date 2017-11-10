// Link State Protocol Frontend
// input(0) : incoming packet
// output(0) : sending backend
// output(1) : lsp backend
#ifndef __CLICK_LSP_FRONTEND
#define __CLICK_LSP_FRONTEND
#include <click/config.h>
#include <click/element.hh>
#include <click/timer.hh>
#include <vector>
#include "lsp_packet.hh"
CLICK_DECLS

class LspFrontend : public Element {
    enum State {
        WaitAck,
        Sleep,
    } state;
    Timer timer;
    Timestamp interval, timeout;
    uint32_t sequence;
    uint32_t self;
    std::vector<uint32_t> portInfo;
public:
    LspFrontend();
    const char *class_name() const { return "LspFrontend"; }
    const char *port_count() const { return "1/2"; }
    const char *processing() const { return PUSH; }
    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void run_timer(Timer *);
    void push(int, Packet *);
    WritablePacket *build_packet(LspType, uint32_t, int);
};

CLICK_ENDDECLS
#endif
