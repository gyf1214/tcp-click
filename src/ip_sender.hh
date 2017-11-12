// Paint anno to send ip packet to frontend
#ifndef __CLICK_IP_SENDER
#define __CLICK_IP_SENDER
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class IpSender : public Element {
    uint32_t dst;
    uint8_t proto;
public:
    const char *class_name() const { return "IpSender"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return AGNOSTIC; }
    int configure(Vector<String> &, ErrorHandler *);
    Packet *simple_action(Packet *);
};

CLICK_ENDDECLS
#endif
