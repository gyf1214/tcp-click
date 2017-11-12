#ifndef __CLICK_INFRA_ANNO
#define __CLICK_INFRA_ANNO
#include <click/config.h>
CLICK_DECLS

enum AnnoType {
    FromInterface = 0,
    ToInterface = 2,
    SendIp = 4,
    RecvIp = 8,
    SendProto = 12,
    RecvProto = 13,
    SocketMethod = 14,
    SocketId = 15,
    SocketSequence = 16,
    SrcPort = 20,
    DstPort = 22,
    SocketParentId = 24,
};

const int AnnoAnyPort = -1;

CLICK_ENDDECLS
#endif
