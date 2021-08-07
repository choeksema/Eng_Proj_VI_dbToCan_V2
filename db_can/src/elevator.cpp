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
#include "../../include/DB_Class.h"
#include "../../include/DB_Conds_Class.h"
#include <iostream>

using namespace std;

#define 	errMsgStart	(string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: "


/*
 * Method to execute the state machine for the elevator.
 *
 * Input:
 * 	int node: the (CAN) node making the request
 * 	int floor: the request made
 *
 * Returns int: the current floor number or 0 if the elevator is moving
 */
int Elevator::transition(/*list<tuple<int,int>> allRequests*/)
{
    // Check where the elevator actually is
    int curFloor = getFloor();
#ifdef DEBUG
	cout << "Elevator is currently at " << curFloor << endl;
#endif

    switch(curFloor)
    {
		case 1:
			CurState = state::at_1;
			LastFloor = 1;
			break;
		case 2:
			CurState = state::at_2;
			LastFloor = 2;
			break;
		case 3:
			CurState = state::at_3;
			LastFloor = 3;
			break;
		default:
			// Moving up/down state is preserved
			break;
    }

	tuple<int, int> newState_raw = getNextState();
#ifdef DEBUG
    cout << errMsgStart << "State: " << to_string((int)CurState) << endl;
	cout << errMsgStart << "Output: nodeID: " << get<REQ_NODE_ID>(newState_raw) << ", floor: " << get<REQ_FLOOR>(newState_raw) << endl;
#endif

	event* newEvent = translateEvent(get<REQ_NODE_ID>(newState_raw), get<REQ_FLOOR>(newState_raw));
    if (newEvent == nullptr)
    {
#ifdef DEBUG
        cout << errMsgStart << "New event is null..." << endl;
#endif
		
		if (state::at_1 == CurState) { return 1; }
		else if (state::at_2 == CurState) { return 2; }
		else if (state::at_3 == CurState) { return 3; }
		else { return 0; }
    }
    state newState = NextState[(int)CurState][(int)(*newEvent)];

#ifdef DEBUG
    cout << "Event: " << to_string((int)*newEvent) << endl;
    cout << "newState: " << to_string((int)newState) << endl;
#endif

    if (newState == state::COND) {}
    else if (newState == CurState)
        cout << "Already in this state so not doing anything." << endl;

    else
    {
		CurState = newState;
#ifdef DEBUG
		cout << "CurState: " << to_string((int)CurState) << endl;
#endif

		int destination = 0;
		if ((*newEvent == event::to_1) || (*newEvent == event::up_from_1)) { destination = 1; }

		else if ((*newEvent == event::to_2) || (*newEvent == event::dn_from_2) || (*newEvent == event::up_from_2)) { destination = 2; }

		else if ((*newEvent == event::to_3) || (*newEvent == event::dn_from_3)) { destination = 3; }

		if(!sendToFloor(destination))    // could be done better
		{
			cout << "Failed sending the elevator" << endl;
			return 0;
		}
		cout << "Elevator sent to floor " << destination << endl;
    }

    int position = 0;    // Means the elevator is moving
    if (CurState == state::at_1) { position = 1; }
    else if (CurState == state::at_2) { position = 2; }
    else if (CurState == state::at_3) { position = 3; }	

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
	bool badInput = false;
    try
    {
		switch(node)
		{
			case NODE_CAR:    // From in the elevator car
				if (FLOOR_TO_1 == floor) { input = event::to_1; }    // Floor 1 button
				else if (FLOOR_TO_2 == floor) { input = event::to_2; }
				else if (FLOOR_TO_3 == floor) { input = event::to_3; }
				break;

			case NODE_AT_1:
				if (FLOOR_TO_NODE == floor) { input = event::to_1; }
				else if (FLOOR_GO_UP == floor) { input = event::up_from_1; }
				break;
			
			case NODE_AT_2:
				if (FLOOR_TO_NODE == floor) { input = event::to_2; }
				else if (FLOOR_GO_DOWN == floor) { input = event::dn_from_2; }
				else if (FLOOR_GO_UP == floor) { input = event::up_from_2; }
				break;
		
			case NODE_AT_3:
				if (FLOOR_TO_NODE == floor) { input = event::to_3; }
				else if (FLOOR_GO_DOWN == floor) { input = event::dn_from_3; }
				break;
		
			default:
				badInput = true;
				break;
		}

		newEvent = (badInput) ? nullptr : &input;
    }
    catch(const exception& ex)
    {
        cout << errMsgStart << ex.what() << endl;
        cout << errMsgStart << "inputs: floor= " << floor << ", node= " << node << endl;
        newEvent = nullptr;
    }
    catch(...)
    {
        cout << errMsgStart << "General error" << endl;
        newEvent = nullptr;
    }
    return newEvent;
}


// Should act on events not finished yet
tuple<int,int> Elevator::getNextState()
{
    DB db(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

	// Select columns
	list<string> columns = {
		elevatorNetwork[(int)elevatorNetworkOffsets::timestamp],
		elevatorNetwork[(int)elevatorNetworkOffsets::nodeID],
		elevatorNetwork[(int)elevatorNetworkOffsets::floor]    // REQ is split between nodeID and floor
	};
	int Offset_timeStamp = 0, Offset_nodeID = 1, Offset_floor = 2;
	
	DBCond getStatus, getFloor, getNode, getLimit;
	list<DBCond> conditions;
	vector<vector<string>> newEntries;


	// If elevator isn't moving, direction doesn't matter
	if ((state::at_1 == CurState) || (state::at_2 == CurState) || (state::at_3 == CurState))
	{
		// At a floor - delete associated requests that are now satisfied
		getStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, LastFloor);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getFloor});

		newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
		if (newEntries.size() > 1)
			db._delete(tbElevatorNetwork, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
		
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, LastFloor);
		conditions.clear();
		conditions.splice(conditions.begin(), {getNode, getStatus});

		newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
		if (newEntries.size() > 1)
			db._delete(tbElevatorNetwork, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);


		// Check current floor first
		getStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, LastFloor);
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, NODE_CAR);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});

		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // returns column types in the first row, even with no data
		{
			// No motion needed so all entries are dealt with - delete them
			db._delete(tbElevatorNetwork, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
#ifdef DEBUG
			cout << errMsgStart << "Delete all entries matching current floor: " << db.getQuery() << endl;
#endif
			return make_tuple(NODE_CAR, LastFloor);
		}

		// Unfortunately there are 2 ways of encoding the current floor and DB::select() is not good enough to handle both at once
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, LastFloor);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode});

		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if(newEntries.size() > 1)
		{
			// No motion needed - entries are dealt with - delete them
			db._delete(tbElevatorNetwork, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);	
#ifdef DEBUG
			cout << errMsgStart << "Delete all entries matching current floor: " << db.getQuery() << endl;
#endif
			return make_tuple(NODE_CAR, LastFloor);
		}


		// If here, there are no requests for the current floor so check other floors
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::NEQ, LastFloor);
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::NEQ, LastFloor);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor});

		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor with the greatest number of requests
		{
			int floorToReach = 0, maxCount = 0, nearestFloor = 0;
			
			for (int f = ELEVATOR_FLOOR_BOTTOM; f <= ELEVATOR_FLOOR_TOP; f++)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if ((f == stoi(newEntries[x][Offset_nodeID])) || (f == stoi(newEntries[x][Offset_floor])))
						count++;
#ifdef DEBUG
				cout << errMsgStart << count << " entries for floor " << f << endl;
#endif
				if ((1 == abs(f - LastFloor)) && (0 != count))
				{
					nearestFloor = f;
				}

				if (count > maxCount)
				{
					maxCount = count;
					floorToReach = f;
				}
			}
			if (nearestFloor > 0)
				floorToReach = nearestFloor;
#ifdef DEBUG
			cout << errMsgStart << "Elevator needs to go to " << floorToReach << endl;
#endif

			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, floorToReach);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor});
			DBCond updateStatus;
			updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
			
			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif

			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, floorToReach);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getNode});
			
			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(NODE_CAR, floorToReach);    // Do NOT update Elevator::LastFloor - only do that once it gets to a floor
		}
	}
	

	else if (state::go_up == CurState)
	{
		// Going to floor 2 or floor 3 if nodeID is 0
		getStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::GT, LastFloor);  // Deals with pesky issue of floor 2 :)
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, NODE_CAR);
		getLimit.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::LTEQ, ELEVATOR_FLOOR_TOP);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor, getLimit});
		
		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND, "floor");  // <-- sort
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor
		{
			int floorToReach = 0;
			for (int f = LastFloor; f <= ELEVATOR_FLOOR_TOP; f++)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if (f == stoi(newEntries[x][Offset_floor]))
						count++;

				if (count > 0)
				{
					floorToReach = f;    // Go to this nearest floor
					break;
				}
			}
#ifdef DEBUG
			cout << "Nearest floor is #" << floorToReach << endl;
#endif
	
			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, floorToReach);
			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, NODE_CAR);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});

			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
			{
				DBCond updateStatus;
				updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			}
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(NODE_CAR, floorToReach);
		}

		// Going to floor 2 or floor 3 if floor is 0
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_TO_NODE);
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::GT, LastFloor);    // Deals with pesky issue of floor 2 :)
		getLimit.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::LTEQ, ELEVATOR_FLOOR_TOP);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor, getLimit});
		
		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND, "floor");  // <-- sort
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor
		{
			int floorToReach = 0;
			for (int f = LastFloor; f <= ELEVATOR_FLOOR_TOP; f++)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if (f == stoi(newEntries[x][Offset_nodeID]))
						count++;

				if (count > 0)
				{
					floorToReach = f;    // Go to this nearest floor
					break;
				}
			}
#ifdef DEBUG
			cout << "Nearest floor is #" << floorToReach << endl;
#endif
	
			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_TO_NODE);
			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, floorToReach);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});
			
			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
			{
				DBCond updateStatus;
				updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			}
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(floorToReach, FLOOR_TO_NODE);
		}

		// Going to floor 2 or floor 3 if request is from outside ("I want to go up") (floor is 5)
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_GO_UP);
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::GT, LastFloor);    // Deals with pesky issue of floor 2 :)
		getLimit.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::LTEQ, ELEVATOR_FLOOR_TOP);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor, getLimit});
		
		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND, "floor");  // <-- sort
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor
		{
			int floorToReach = 0;
			for (int f = LastFloor; f <= ELEVATOR_FLOOR_TOP; f++)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if (f == stoi(newEntries[x][Offset_nodeID]))
						count++;

				if (count > 0)
				{
					floorToReach = f;    // Go to this nearest floor
					break;
				}
			}
#ifdef DEBUG
			cout << "Nearest floor is #" << floorToReach << endl;
#endif
	
			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_GO_UP);
			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, floorToReach);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});

			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
			{
				DBCond updateStatus;
				updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			}
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(floorToReach, FLOOR_GO_UP);
		}
	}


	else if (state::go_dn == CurState)
	{
		// Going to floor 2 or floor 1 if nodeID is 0
		getStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::LT, LastFloor);  // Deals with pesky issue of floor 2 :)
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, NODE_CAR);
		getLimit.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::GTEQ, ELEVATOR_FLOOR_BOTTOM);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor, getLimit});
		
		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND, "floor", ORDER_DESCEND);  // <-- sort
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor
		{
			int floorToReach = 0;
			for (int f = LastFloor; f >= ELEVATOR_FLOOR_TOP; f--)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if (f == stoi(newEntries[x][Offset_floor]))
						count++;

				if (count > 0)
				{
					floorToReach = f;    // Go to this nearest floor
					break;
				}
			}
#ifdef DEBUG
			cout << "Nearest floor is #" << floorToReach << endl;
#endif
	
			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, floorToReach);
			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, NODE_CAR);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});

			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
			{
				DBCond updateStatus;
				updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			}
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(NODE_CAR, floorToReach);
		}


		// Going to floor 2 or floor 1 if floor is 0
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_TO_NODE);
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::LT, LastFloor);    // Deals with pesky issue of floor 2 :)
		getLimit.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::GTEQ, ELEVATOR_FLOOR_BOTTOM);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor, getLimit});
		
		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND, "floor", ORDER_DESCEND);  // <-- sort
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor
		{
			int floorToReach = 0;
			for (int f = LastFloor; f >= ELEVATOR_FLOOR_TOP; f--)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if (f == stoi(newEntries[x][Offset_floor]))
						count++;

				if (count > 0)
				{
					floorToReach = f;    // Go to this nearest floor
					break;
				}
			}
#ifdef DEBUG
			cout << "Nearest floor is #" << floorToReach << endl;
#endif
	
			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_TO_NODE);
			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, floorToReach);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});
			
			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
			{
				DBCond updateStatus;
				updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			}
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(floorToReach, FLOOR_TO_NODE);
		}


		// Going to floor 2 or floor 1 if request is from outside ("I want to go down") (floor is 4)
		getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_GO_DOWN);
		getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::LT, LastFloor);    // Deals with pesky issue of floor 2 :)
		getLimit.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::GTEQ, ELEVATOR_FLOOR_BOTTOM);
		conditions.clear();
		conditions.splice(conditions.begin(), {getStatus, getNode, getFloor, getLimit});
		
		newEntries = db.select(tbElevatorNetwork, columns, conditions, DBCond::LOGICAL_RELATION_JOIN::AND, "floor", ORDER_DESCEND);  // <-- sort
#ifdef DEBUG
		cout << errMsgStart << db.getQuery() << endl;
#endif
		if (newEntries.size() > 1)    // Move to the nearest floor
		{
			int floorToReach = 0;
			for (int f = LastFloor; f >= ELEVATOR_FLOOR_TOP; f--)
			{
				int count = 0;
				for (uint x = 1; x < newEntries.size(); x++)
					if (f == stoi(newEntries[x][Offset_nodeID]))
						count++;

				if (count > 0)
				{
					floorToReach = f;    // Go to this nearest floor
					break;
				}
			}
#ifdef DEBUG
			cout << "Nearest floor is #" << floorToReach << endl;
#endif
	
			// Update any entries matching these conditions as being dealt with
			getFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, FLOOR_GO_DOWN);
			getNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, floorToReach);
			conditions.clear();
			conditions.splice(conditions.begin(), {getStatus, getFloor, getNode});

			newEntries = db.select(tbElevatorNetwork, {}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			if (newEntries.size() > 1)
			{
				DBCond updateStatus;
				updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);
				db.update(tbElevatorNetwork, {updateStatus}, conditions, DBCond::LOGICAL_RELATION_JOIN::AND);
			}
#ifdef DEBUG
			cout << errMsgStart << db.getQuery() << endl;
#endif
			
			return make_tuple(FLOOR_GO_DOWN, floorToReach);
		}
	}	
#ifdef DEBUG
	cout << errMsgStart << "Didn't find any requests at all" << endl;
#endif
	
	// If it gets here, just say it didn't move. It will be adjusted on the next FSM query
	return make_tuple(NODE_CAR, LastFloor);
}
