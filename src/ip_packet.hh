#ifndef __CLICK_IP_PACKET
#define __CLICK_IP_PACKET
#include <click/config.h>
#include <click/integers.hh>
#include <clicknet/ip.h>
CLICK_DECLS

struct IpHeader {
    // version, ihl, tos is constant
    uint16_t magic;
    uint16_t length;
    // fragment is not implemented, set all to zero
    uint32_t zero;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src;
    uint32_t dst;
    char data[0];

    void init(uint16_t, uint8_t, uint32_t, uint32_t, uint8_t);
    void calc_checksum();
};

// version 4, ihl 5, tos 0
const uint16_t IpMagic = 0x0045;
// default ttl
const uint8_t IpTTL = 64;
// broadcast address
const uint32_t IpAny = -1;

const uint8_t IpProtoLsp = 61;

const size_t IpSize = sizeof(IpHeader);

inline void IpHeader::init(uint16_t len, uint8_t proto,
uint32_t s, uint32_t d, uint8_t t = IpTTL) {
    magic = IpMagic;
    length = htons(len);
    zero = 0;
    ttl = t;
    protocol = proto;
    src = s;
    dst = d;
    calc_checksum();
}

inline void IpHeader::calc_checksum() {
    checksum = 0;
    checksum = click_in_cksum((unsigned char *)this, IpSize);
}

CLICK_ENDDECLS
#endif
