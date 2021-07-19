#ifndef _USE_CANBUS_H_
#define _USE_CANBUS_H_

#include "../include/PCAN_Class.h"
#include <fcntl.h>

#define	 	TO_FLOOR_1		0x05
#define		TO_FLOOR_2		0x06
#define		TO_FLOOR_3		0x07

#define		PI_TO_ELEV		0x100

#define 	PCAN_PATH		"/dev/pcanusb32"
#define		DEF_NFLAG		O_RDWR


bool sendToFloor(int);
int getFloor();

#endif
