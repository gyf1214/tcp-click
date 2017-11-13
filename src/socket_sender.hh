#ifndef __CLICK_SOCKET_SENDER
#define __CLICK_SOCKET_SENDER
#include <click/config.h>
#include <click/element.hh>
#include <click/timer.hh>
CLICK_DECLS

class SocketSender : public Element {
    uint32_t self, ip;
    uint16_t sport, dport;
    Timestamp interval, wait;
    Timer timer;
    enum {
        Nothing, Start, Writing, Closing, Err
    } state;
    uint32_t sequence;
    uint8_t id;
public:
    SocketSender();
    const char *class_name() const { return "SocketSender"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return "h/h"; }
    void run_timer(Timer *);
    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
