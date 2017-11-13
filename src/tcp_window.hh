#ifndef __CLICK_TCP_WINDOW
#define __CLICK_TCP_WINDOW
#include <click/config.h>
#include <click/deque.hh>
#include <click/packet.hh>
#include <click/timer.hh>
CLICK_DECLS

const size_t TcpBufferSize = 65536;
const size_t TcpSegmentSize = 1024;
const uint32_t TcpFixedCWnd = 20;

struct TcpSendWindow {
    Deque<Packet *> wait;
    char buf[TcpBufferSize];
    uint32_t seq_front, seq_back, buf_back;
    Deque<uint32_t> wnd;
    uint16_t rwnd;
    uint8_t cwnd, fails;
    Timer timer;
    void init(Element *, TimerCallback);
    uint32_t max_grow();
    uint32_t max_buffer();
};

struct TcpBlock {
    uint32_t ip;
    uint16_t sport;
    uint16_t dport;
    TcpSendWindow swnd;
};

inline void TcpSendWindow::init(Element *e, TimerCallback f) {
    wait.clear();
    wnd.clear();
    seq_front = seq_back = buf_back = 0;
    cwnd = TcpFixedCWnd;
    rwnd = TcpBufferSize;
    timer.assign(f, NULL);
    timer.initialize(e);
}

// max length to grow sending wnd
inline uint32_t TcpSendWindow::max_grow() {
    // check congestion window
    if (wnd.size() >= cwnd) {
        return 0;
    }
    // check recv window
    if (rwnd < seq_back - seq_front) {
        return 0;
    }
    uint32_t ret = buf_back - seq_back;
    uint32_t r0 = TcpSegmentSize;
    ret = ret > r0 ? r0 : ret;
    r0 = rwnd - (seq_back - seq_front);
    return ret > r0 ? r0 : ret;
}

// max length to push buffered data
inline uint32_t TcpSendWindow::max_buffer() {
    // -1 for round queue
    return TcpBufferSize - (buf_back - seq_front) - 1;
}

inline void tcp_from_wnd(char *dst, const char *src, uint32_t size, uint32_t l, uint32_t r) {
    l = l % size;
    r = r % size;
    if (r == l) return;
    if (r > l) {
        memcpy(dst, src + l, r - l);
    } else {
        memcpy(dst, src + l, size - l);
        memcpy(dst + size - l, src, r);
    }
}

inline void tcp_to_wnd(char *dst, const char *src, uint32_t size, uint32_t l, uint32_t r) {
    l = l % size;
    r = r % size;
    if (r == l) return;
    if (r > l) {
        memcpy(dst + l, src, r - l);
    } else {
        memcpy(dst + l, src, size - l);
        memcpy(dst, src + size - l, r);
    }
}

CLICK_ENDDECLS
#endif
