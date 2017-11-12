// Tcp Frontend
// input(0) : incoming packets
// input(1) : incoming socket requests
// output(0) : sending backend
// output(1) : socket response;
// output(2) : tcp backend
#ifndef __CLICK_TCP_FRONTEND
#define __CLICK_TCP_FRONTEND
#include <click/config.h>
#include <click/element.hh>
#include <vector>
#include <queue>
#include "tcp_socket.hh"
CLICK_DECLS

class TcpFrontend : public Element {
    std::vector<TcpSocket> sockets;
    uint32_t self;
public:
    const char *class_name() const { return "TcpFrontend"; }
    const char *port_count() const { return "2/3"; }
    const char *processing() const { return PUSH; }
    int configure(Vector<String> &, ErrorHandler *);
    // find socket based on dst_ip, src_port, dst_port
    int find_socket(uint32_t, uint16_t, uint16_t);
    // find listening socket based on port
    int find_bind_socket(uint16_t, bool = true);
    int find_empty_socket();
    // create conn socket based on listen socket
    void create_accept(TcpSocket &, uint32_t, uint16_t);
    // queue accept request
    void queue_accept(TcpSocket &, Packet *);
    // queue accept socket
    void queue_listen(uint8_t, uint8_t);
    // free wait queue
    void free_wait(TcpSocket &);
    // send tcp short packet
    void send_short(uint32_t, uint16_t, uint16_t, uint16_t);
    void send_short(TcpSocket &, uint16_t);
    // send back socket response
    void send_return(Packet *p, bool);
    // handle socket request
    void push_socket(Packet *);
    // handle tcp packet
    void push_tcp(Packet *);
    void push(int, Packet *);
    // inform back to close
    void back_close(uint8_t);
    // inform back to establish
    void back_establish(uint8_t);
    // reset socket
    void reset_socket(uint8_t, bool = true);
};

CLICK_ENDDECLS
#endif
