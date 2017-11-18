// implement simple push element
#include "simple_test.hh"
CLICK_DECLS

SimpleTestElement::SimpleTestElement() {}
SimpleTestElement::~SimpleTestElement() {}

int SimpleTestElement::configure(Vector<String> &, ErrorHandler *) {
    return 0;
}

void SimpleTestElement::push(int, Packet *p) {
    click_chatter("got a packet of size %d", p->length());
    output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SimpleTestElement)
