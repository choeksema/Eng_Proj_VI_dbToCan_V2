#ifndef _PCAN_CLASS_H_
#define _PCAN_CLASS_H_

#include <libpcan.h>
#include <string>

#define		RX_QUEUE_EMPTY		0x00020U
#define		PCAN_NO_ERROR		0x00000U
#define 	STATUS_ID			0x01
#define		STATUS_LEN			0x04

using namespace std;

class PCAN
{
	DWORD status;
	TPCANMsg message;
	HANDLE handle;
	
public:
	void setMsg(int id, int data);
	TPCANMsg getMsg();
	
	bool openInit(const char* path, int nFlag, int rate);
	bool close();

	bool send();
	bool send(int id, int data);
	
	int receive();
};

#endif
