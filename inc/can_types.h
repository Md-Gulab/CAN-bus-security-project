#ifndef CAN_TYPES_H
#define CAN_TYPES_H

typedef enum STATUS{
    OK = 0 ,
    TIMEOUT,
    BUSY,
    NO_FREE_MAILBOX,
    FIFO_EMPTY,
    INVALID_PARAM,
    BUS_OFF
}status;

#endif