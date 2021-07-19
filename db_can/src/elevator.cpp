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
#include <iostream>

using namespace std;

// Passed an event
// Move the elevator if needed and it will start going to floor x
// update current state (not necessarily a floor number)

// NEED TO UPDATE THE CURRENT STATE WHEN AN ELEVATOR REACHES ITS DESTINATION WHETHER A FUNCTION IS CALLED OR NOT...

// CurState should realy be a list so values cannot be thrown away (eg: go to floor 2 first, then floor 3)
    // For later...


/*int Elevator::loadEvent(int input)
{

}*/


int Elevator::transition(int input)
{
    // GET THE CURRENT POSITION FROM THE ACTUAL ELEVATOR HERE!
    int floor = getFloor();

    if ((floor > 0) && (floor < 4))
	CurState = (state)(floor - 1);

    cout << "Event: " << to_string(input) << endl;
    cout << "State: " << to_string((int)CurState) << endl;

    // Parse the input
    event* newEvent = translateEvent(input);
    if (newEvent == nullptr)
    {
        cout << "New event is null..." << endl;
        return (int)CurState;    // Nothing changed
    }

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

	int floor = 0;
	// The elevator needs to be moved
	if((input == (int)event::to_1) || (input == (int)event::up_from_1))
	    floor = 1;
	else if ((input == (int)event::to_2) || (input == (int)event::dn_from_2) || (input == (int)event::up_from_2))
	    floor = 2;
	else if ((input == (int)event::to_3) || (input == (int)event::dn_from_3))
 	    floor = 3;

	if(!sendToFloor(floor))    // Not the most generic... (not the best way to do this)
	{
	    cout << "Failed sending the elevator" << endl;
    	    return 0;
	}
	cout << "Elevator sent to floor " << floor << endl;
    }

    if((CurState == state::at_1) || (CurState == state::at_2) || (CurState == state::at_3))
    {
        return ((int)CurState + 1);   // Elevator is at a floor (enum starts at 0 - not the best way to do this...)
    }
    else
        return 0;               // Elevator is in transit
}


/* 
 * Method to translate an int to an Elevator event
 *
 * Inputs:
 *      int input: the int to translate to an event
 * 
 * Returns event*: pointer to the type of event (nullptr indicates error)
 */
Elevator::event* Elevator::translateEvent(int input)
{
    // Button codes map directly to event codes
    event* newEvent = nullptr;
    try
    {
	newEvent = (event*)(&input);
    }
    catch(const exception& ex)
    {
        cout << "Error: " << ex.what() << endl;
        cout << "Invalid button/cmd: " << input << endl;
        newEvent = nullptr;
    }
    catch(...)
    {
        cout << "Caught an error of some kind." << endl;
        newEvent = nullptr;
    }
    return newEvent;
}
