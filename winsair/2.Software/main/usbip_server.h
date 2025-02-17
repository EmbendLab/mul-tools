#ifndef __USBIP_SERVER_H__
#define __USBIP_SERVER_H__
#include <stdint.h>
#include <stddef.h>


enum state_t
{
    ACCEPTING,
    ATTACHING,
    EMULATING,
    EL_DATA_PHASE
};
extern uint8_t kState;
extern int kSock;



#endif