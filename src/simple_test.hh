// simple push element
#ifndef __TEST_HH
#define __TEST_HH
#include <click/config.h>
#include <click/element.hh>
CLICK_DECLS

class SimpleTestElement : public Element {
public:
    SimpleTestElement();
    ~SimpleTestElement();
    const char *class_name() const { return "SimpleTest"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return PUSH; }
    int configure(Vector<String> &, ErrorHandler *);
    void push(int, Packet *);
};

CLICK_ENDDECLS
#endif
