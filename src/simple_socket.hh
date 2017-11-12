#ifndef __CLICK_SIMPLE_SOCKET
#define __CLICK_SIMPLE_SOCKET
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class SimpleSocket : public Element {
public:
    const char *class_name() const { return "SimpleSocket"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return PUSH; }
};

CLICK_ENDDECLS
#endif
