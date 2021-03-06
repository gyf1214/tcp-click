#ifndef __CLICK_SOCKET_SERVER
#define __CLICK_SOCKET_SERVER
#include <click/config.h>
#include <click/element.hh>
#include <click/timer.hh>
CLICK_DECLS

class SocketServer : public Element {
    uint32_t self;
    uint16_t port;
    Timestamp interval, wait;
    Timer timer;
    enum {
        Nothing, Start, Listened, Reading, Closing, AcceptClose, Err
    } state;
    uint32_t sequence;
    uint8_t id, id1;
    int buffer, total;
public:
    SocketServer();
    const char *class_name() const { return "SocketServer"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return "h/h"; }
    void run_timer(Timer *);
    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
