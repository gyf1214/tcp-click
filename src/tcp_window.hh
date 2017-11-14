#ifndef __CLICK_TCP_WINDOW
#define __CLICK_TCP_WINDOW
#include <click/config.h>
#include <click/deque.hh>
#include <click/packet.hh>
#include <click/timer.hh>
CLICK_DECLS

const size_t TcpBufferSize = 65536;
const size_t TcpSegmentSize = 1420;
const uint32_t TcpFixedCWnd = 10;

// look ahead for disorder packet
const size_t TcpRecvMaxAhead = TcpSegmentSize * 10;

struct TcpSendWindow {
    Deque<Packet *> wait;
    char buf[TcpBufferSize];
    uint32_t seq_front, seq_back, buf_back;
    Deque<uint32_t> wnd;
    uint16_t rwnd;
    uint8_t cwnd, fails;
    Timer *timer;
    bool fin;
    void init(Element *, TimerCallback, uint8_t);
    uint32_t max_grow();
    uint32_t max_buffer();
};

struct TcpRecvWindow {
    Deque<Packet *> wait;
    char buf[TcpBufferSize];
    // recving flag for disorder
    // TODO: lack of efficiency
    bool disorder[TcpRecvMaxAhead];
    uint32_t seq_front, seq_back, buf_back;
    void init();
    uint32_t max_grow();
    uint32_t max_recv();
    uint32_t max_tail();
    void forward();
    void mark_disorder(uint32_t, uint32_t);
    bool check_disorder(uint32_t, uint32_t);
};

struct TcpBlock {
    uint32_t ip;
    uint16_t sport;
    uint16_t dport;
    TcpSendWindow swnd;
    TcpRecvWindow rwnd;
};

inline void TcpSendWindow::init(Element *e, TimerCallback f, uint8_t i) {
    wait.clear();
    wnd.clear();
    seq_front = seq_back = buf_back = 0;
    cwnd = TcpFixedCWnd;
    rwnd = TcpFixedCWnd * TcpSegmentSize;
    fin = false;
    intptr_t id = i;
    timer = new Timer(f, (void *)id);
    timer->initialize(e);
}

inline void TcpRecvWindow::init() {
    wait.clear();
    seq_front = seq_back = buf_back = 0;
    memset(disorder, 0, sizeof(disorder));
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

// max length to recv data
inline uint32_t TcpRecvWindow::max_grow() {
    return TcpBufferSize - (seq_back - seq_front) - 1;
}

// max length to response to send, i.e. window size
inline uint32_t TcpRecvWindow::max_recv() {
    return seq_back - seq_front;
}

// max tail for disorder area
inline uint32_t TcpRecvWindow::max_tail() {
    uint32_t ret = max_grow() + seq_back;
    uint32_t r0 = TcpRecvMaxAhead + seq_back - 1;
    return ret > r0 ? r0 : ret;
}

// mark a packet to be recved
inline void TcpRecvWindow::mark_disorder(uint32_t l, uint32_t r) {
    if (r > buf_back) buf_back = r;
    l = l % TcpRecvMaxAhead;
    r = r % TcpRecvMaxAhead;
    if (r == l) return;
    if (r > l) {
        for (; l < r; ++l) disorder[l] = true;
    } else {
        for (; l < TcpRecvMaxAhead; ++l) disorder[l] = true;
        for (l = 0; l < r; ++l) disorder[l] = true;
    }
}

// check an area
inline bool TcpRecvWindow::check_disorder(uint32_t l, uint32_t r) {
    l = l % TcpRecvMaxAhead;
    r = r % TcpRecvMaxAhead;
    if (r == l) return false;
    if (r > l) {
        for (; l < r; ++l) if (!disorder[l]) return false;
    } else {
        for (; l < TcpRecvMaxAhead; ++l) if (!disorder[l]) return false;
        for (l = 0; l < r; ++l) if (!disorder[l]) return false;
    }
    return true;
}

// try resolve disorder and move forward;
inline void TcpRecvWindow::forward() {
    while (seq_back < buf_back && disorder[seq_back % TcpRecvMaxAhead]) {
        disorder[seq_back % TcpRecvMaxAhead] = false;
        ++seq_back;
    }
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
