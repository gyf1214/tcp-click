#ifndef __CLICK_INFRA_LOG
#define __CLICK_INFRA_LOG
#include <click/config.h>
CLICK_DECLS

#define Log(fmt, ...) click_chatter("%s@%x\t[Info] " fmt, this->class_name(), this->self, ##__VA_ARGS__)
#define Warn(fmt, ...) click_chatter("%s@%x\t[Warn] " fmt, this->class_name(), this->self, ##__VA_ARGS__)

CLICK_ENDDECLS
#endif
