/*
 * elevator.cpp
 *
 * File to manage the elevator states and input
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */

#include "../include/elevator.h"
#include "../include/useCanBus.h"
#include "../../include/DB_DEFS.h"
#include "../../include/CAN_DEFS.h"
#include <iostream>

using namespace std;

#define 	errMsgStart	(string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: "


// CurState should really be a list so values cannot be thrown away (eg: go to floor 2 first, then floor 3)
    // For later...


/*
 * Method to execute the state machine for the elevator.
 *
 * Input:
 * 	int node: the (CAN) node making the request
 * 	int floor: the request made
 *
 * Returns int: the current floor number or 0 if the elevator is moving
 */
int Elevator::transition(int node, int floor)
{
    // Check where the elevator actually is
    int curFloor = getFloor();
    switch(curFloor)
    {
	case 1:
	    CurState = state::at_1;
	    break;
	case 2:
	    CurState = state::at_2;
	    break;
	case 3:
	    CurState = state::at_3;
	    break;
	default:
	    // Don't care
	    break;
    }

    cout << "State: " << to_string((int)CurState) << endl;

    // Parse the input
    event* newEvent = translateEvent(node, floor);
    if (newEvent == nullptr)
    {
        cout << errMsgStart << "New event is null..." << endl;
	if (state::at_1 == CurState)
	    return 1;
	
	else if (state::at_2 == CurState)
	    return 2;

	else if (state::at_3 == CurState)
	    return 3;

	else
	    return 0;
    }

    cout << "Event: " << to_string((int)*newEvent) << endl;

    state newState = NextState[(int)CurState][(int)(*newEvent)];

    cout << "newState: " << to_string((int)newState) << endl;

    if (newState == state::COND)
    {
        cout << "The elevator doesn't have the brains to deal with this yet. State will not change." << endl;
	cout << "Unfortunately, the position relative to floor 2 doesn't seem to be available unless you are already at a floor" << endl;
	// Should only be in this state if not sure to stop at floor 2 or not
    }
    else if (newState == CurState)
    {
        cout << "Already in this state so not doing anything." << endl;
    }
    else
    {
	CurState = newState;
	cout << "CurState: " << to_string((int)CurState) << endl;

	int destination = 0;
	if ((*newEvent == event::to_1) || (*newEvent == event::up_from_1))
	    destination = 1;

	else if ((*newEvent == event::to_2) || (*newEvent == event::dn_from_2) || (*newEvent == event::up_from_2))
	    destination = 2;

	else if ((*newEvent == event::to_3) || (*newEvent == event::dn_from_3))
	    destination = 3;

	if(!sendToFloor(destination))    // could be done better
	{
	    cout << "Failed sending the elevator" << endl;
    	    return 0;
	}
	cout << "Elevator sent to floor " << destination << endl;
    }

    int position = 0;    // Means the elevator is moving
    if (CurState == state::at_1)
	position = 1;

    else if (CurState == state::at_2)
	position = 2;

    else if (CurState == state::at_3)
	position = 3;

    return position;
}


/* 
 * Method to translate a pair of ints (floor and node) to an elevator event
 *
 * Inputs:
 *      int node: the CAN node making the request (elevator car, floor 1, etc)
 *      int floor: the request made by the node (see elevator.h defines)
 * 
 * Returns event*: pointer to the type of event (nullptr indicates error)
 */
Elevator::event* Elevator::translateEvent(int node, int floor)
{
    event* newEvent = nullptr;
    event input;
    try
    {
	switch(node)
	{
	    case NODE_CAR:    // From in the elevator car
		if (FLOOR_TO_1 == floor)    // Floor 1 button
		    input = event::to_1;

		else if (FLOOR_TO_2 == floor)
		    input = event::to_2;
	 
		else if (FLOOR_TO_3 == floor)
		     input = event::to_3;

		break;

	    case NODE_AT_1:
		if (FLOOR_TO_NODE == floor)
		    input = event::to_1;

		else if (FLOOR_GO_UP == floor)
		    input = event::up_from_1;
		
		break;
	    
	    case NODE_AT_2:
		if (FLOOR_TO_NODE == floor)
		    input = event::to_2;

		else if (FLOOR_GO_DOWN == floor)
		    input = event::dn_from_2;

		else if (FLOOR_GO_UP == floor)
		    input = event::up_from_2;

		break;
	
	    case NODE_AT_3:
		if (FLOOR_TO_NODE == floor)
		    input = event::to_3;

		else if (FLOOR_GO_DOWN == floor)
		    input = event::dn_from_3;

		break;
	
	    default:
		// don't care
		break;
	}

	newEvent = &input;
    }
    catch(const exception& ex)
    {
        cout << errMsgStart << ex.what() << endl;
        cout << "inputs: floor= " << floor << ", node= " << node << endl;
        newEvent = nullptr;
    }
    catch(...)
    {
        cout << errMsgStart << "General error" << endl;
        newEvent = nullptr;
    }
    return newEvent;
}

