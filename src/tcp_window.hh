#ifndef __CLICK_TCP_WINDOW
#define __CLICK_TCP_WINDOW
#include <click/config.h>
#include <click/deque.hh>
#include <click/packet.hh>
#include <click/timer.hh>
CLICK_DECLS

const size_t TcpBufferSize = 65536;
const size_t TcpSegmentSize = 1024;

const uint32_t TcpFixedCWnd = 4096;

struct TcpSendWindow {
    Deque<Packet *> wait;
    char buf[TcpBufferSize];
    uint32_t seq_front, seq_last, seq_back, buf_back;
    uint32_t cwnd;
    Timer timer;
    void init(Element *, TimerCallback);
    uint32_t max_grow();
};

struct TcpBlock {
    uint32_t ip;
    uint16_t sport;
    uint16_t dport;
    TcpSendWindow swnd;
};

inline void TcpSendWindow::init(Element *e, TimerCallback f) {
    wait.clear();
    seq_front = seq_last = seq_back = buf_back = 0;
    cwnd = TcpFixedCWnd;
    timer.assign(f, NULL);
    timer.initialize(e);
}

inline uint32_t TcpSendWindow::max_grow() {
    uint32_t ret = buf_back - seq_back;
    uint32_t r0 = cwnd - (seq_back - seq_front);
    ret = ret > r0 ? r0 : ret;
    r0 = TcpSegmentSize;
    return ret > r0 ? r0 : ret;
}

inline void TcpFromWnd(char *dst, const char *src, uint32_t size, uint32_t l, uint32_t r) {
    l = l % size;
    r = r % size;
    if (r == l) return;
    if (r > l) {
        memcpy(dst, src + l, r - l);
    } else {
        memcpy(dst, src + l, size - l);
        memcpy(dst, src, r);
    }
}

CLICK_ENDDECLS
#endif
