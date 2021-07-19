

#include "../include/PCAN_Class.h"
#include <iostream>
#include <unistd.h>

// Assuming standard mode (extended is not supported at this time)

using namespace std;

void PCAN::setMsg(int id, int data)
{
	message.ID = id;
	message.MSGTYPE = MSGTYPE_STANDARD;
	message.LEN = 1;
	message.DATA[0] = data;
}

// Not sure if this works well
TPCANMsg PCAN::getMsg()
{
	return message;
}

bool PCAN::openInit(const char* path, int nFlag, int rate)
{	
	try
	{
		handle = LINUX_CAN_Open(path, nFlag);
		status = CAN_Init(handle, rate, CAN_INIT_TYPE_ST);
	}
	catch(const exception& ex)
	{
		cout << "Failed open: " << ex.what() << endl;
		close();    // Try to close the channel again
		return false;
	}
	return true;
}

bool PCAN::close()
{
	try
	{
		CAN_Close(handle);
	}
	catch(const exception& ex)
	{
		cout << "Failed close: " << ex.what() << endl;
		return false;
	}
	return true;
}

bool PCAN::send()
{
	try
	{
		status = CAN_Status(handle);
		// Check the message here?
		sleep(1);
		status = CAN_Write(handle, &message);
	}
	catch(const exception& ex)
	{
		cout << "Failed send: " << ex.what() << endl;
		return false;
	}
	return true;
}

bool PCAN::send(int id, int data)
{
	setMsg(id, data);
	return send();
}

int PCAN::receive()
{
    while(1)
    {

	while(RX_QUEUE_EMPTY == (status = CAN_Read(handle, &message)))
		sleep(1);
	
	if (PCAN_NO_ERROR != status)
	{
		cout << "Error No. " << status << ". Returning." << endl;
		return false;
	}
	
	/*if ((STATUS_ID == message.ID) && (STATUS_LEN == message.LEN))
	{
		cout << "Status message so don't care" << endl;
		continue;
	}*/

	/*cout << "Message: ID = " << (int)message.ID 
		 << ", LEN = " << (int)message.LEN
		 << ", DATA = " << (int)message.DATA[0]
		 << "." << endl;*/

	if(message.ID != 257)    // 257 is 0x101
	    continue;

	switch((int)message.DATA[0])
	{
	    case (5):
		return 1;    // floor 1
	    case 6:
		return 2;    // floor 2
	    case 7:
		return 3;    // floor 3
	    default:
		return 0;   // 4 means moving, otherwise just assume moving
	
	// return true;
	}
    }
}
