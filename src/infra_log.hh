#ifndef __CLICK_INFRA_LOG
#define __CLICK_INFRA_LOG
#include <click/config.h>
#include <click/timestamp.hh>
CLICK_DECLS

#define Log(fmt, ...) click_chatter("%d.%03d\t%s@%08x\t[Info] " fmt, Timestamp::now().sec(),\
Timestamp::now().msec(), this->class_name(), this->self, ##__VA_ARGS__)
#define Warn(fmt, ...) click_chatter("%d.%03d\t%s@%08x\t[Warn] " fmt, Timestamp::now().sec(),\
Timestamp::now().msec(), this->class_name(), this->self, ##__VA_ARGS__)

CLICK_ENDDECLS
#endif
