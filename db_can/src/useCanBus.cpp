/*
 * File managing CAN bus interactions on the server
 * 
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */

#include <iostream>
#include "../include/useCanBus.h"

using namespace std;

/* NEED ERROR CHECKING, DOCUMENTATION, AND OTHER STUFF */

bool sendToFloor(int reqFloor)
{
	int floorCmd = 0;
	switch(reqFloor)
	{
		case 1:
			floorCmd = TO_FLOOR_1;
			break;
		case 2:
			floorCmd = TO_FLOOR_2;
			break;
		case 3:
			floorCmd = TO_FLOOR_3;
			break;
		default:
			cout << "Floor #" << reqFloor << " does not exist.";
			return false;
	}

	PCAN pecan;

	if(!pecan.openInit(PCAN_PATH, DEF_NFLAG, CAN_BAUD_125K))
	{
		cout << "Failed opening PCAN channel" << endl;
		return false;
	}

	pecan.setMsg(PI_TO_ELEV, floorCmd);

	if(!pecan.send())
	{
		cout << "Failed sending command to elevator" << endl;
		return false;
	}

	cout << "Command sent to elevator" << endl;
	
	if(!pecan.close())
	{
		cout << "Failed closing PCAN channel" << endl;
		return false;
	}

	return true;
}


int getFloor()
{
    PCAN pecan;

    if(!pecan.openInit(PCAN_PATH, DEF_NFLAG, CAN_BAUD_125K))
    {
	cout << "Failed opening PCAN channel" << endl;
	return -1;
    }

    int floor = pecan.receive();
    /*if(!pecan.receive(*//*ID_CAR_NODE*//*))
    {
	cout << "Failed getting current floor" << endl;
	return -1;
    }*/

    if(!pecan.close())
    {
	cout << "Failed closing PCAN channel" << endl;
	return -1;
    }

    return floor;
}

