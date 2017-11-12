#ifndef __CLICK_INFRA_ANNO
#define __CLICK_INFRA_ANNO
#include <click/config.h>
CLICK_DECLS

enum AnnoType {
    FromInterface = 0,
    ToInterface = 2,
    SendIp = 4,
    RecvIp = 8,
    SendProto = 9,
    RecvProto = 10,
};

const int AnnoAnyPort = -1;

CLICK_ENDDECLS
#endif
