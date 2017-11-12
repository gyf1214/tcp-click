#ifndef __CLICK_SIMPLE_SOCKET
#define __CLICK_SIMPLE_SOCKET
#include <click/config.h>
#include <click/element.hh>
#include <click/task.hh>
#include <click/notifier.hh>
CLICK_DECLS

class SimpleSocket : public Element {
protected:
    uint32_t self;
    uint32_t sequence;
    void send_info(const char *);
    void send_info(const String &);
    void exec(Packet *, uint8_t);
    void exec(Packet *, uint8_t, uint8_t);
    void socket(uint16_t);
    void connect(uint8_t, uint32_t, uint16_t);
public:
    SimpleSocket();
    const char *class_name() const { return "SimpleSocket"; }
    const char *port_count() const { return "2/2"; }
    const char *processing() const { return PUSH; }
    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void push_return(Packet *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
