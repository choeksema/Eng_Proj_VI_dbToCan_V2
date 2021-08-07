/*
 * main.cpp
 *
 * File to manage moving the elevator
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */

#include "../../include/DB_Class.h"
#include "../include/elevator.h"

using namespace std;

#define 	errMsgStart	(string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: "

int secondsPerFloor = 30;

bool updateStats(int);
bool updateCurrentPosition(int);
bool addSabbathModeEntry(int,int);


int main(int argc, char** argv)
{
    cout << "Starting the elevator moving process" << endl;
#if (DEBUG)
    cout << errMsgStart << "C++ Version: " << __cplusplus << endl;
#endif 

    /* Set up */
    Elevator elevator;
    
    // Check if demo/auto mode was requested
    string demo = "demo";
    bool demoMode = false;
    if ((2 == argc) && (demo.compare(argv[1]) == 0))
	demoMode = true;

    // Check if Sabbath mode should be overridden
    string strOverride = "override";
    bool overrideSabbathMode = false;
    if ((2 == argc) && (strOverride.compare(argv[1]) == 0))
	overrideSabbathMode = true;

    int lastFloor = 3;

    // Begin running loop
    while(1)
    {
        // Check for Sabbath mode
        time_t now = time(0);
        tm* dateTime = localtime(&now);
#ifdef DEBUG
        cout << errMsgStart << "Now: " << dateTime->tm_hour << endl;
#endif
        
        if (!overrideSabbathMode)
        {
            int floor = 1;
            while (demoMode || (((dateTime->tm_hour >= 18) && (dateTime->tm_wday == 5)) 
                    || ((dateTime->tm_hour <= 18) && (dateTime->tm_wday == 6))))
            {
                cout << "Running in Sabbath mode." << endl;

                if (!addSabbathModeEntry(NODE_CAR, floor))
                    continue;

                int newState = elevator.transition();
                if (newState == 0)
                    cout << "Elevator is moving" << endl;
                else
                {
                    cout << "Elevator is at " << newState << endl;
                    
                    updateCurrentPosition(floor);
                    updateStats(floor);

                    floor++;
                    if (floor > ELEVATOR_FLOOR_TOP)
                        floor = ELEVATOR_FLOOR_BOTTOM;
                }

                sleep(secondsPerFloor);    // Really should start the sleep from arrival on at a floor instead of departure

                now = time(0);
                dateTime = localtime(&now);
            } // while (Sabbath time check)
        } // if(!overrideSabbathMode)


        /* Regular operation */
        try
        {
            int floor = elevator.transition();
#ifdef DEBUG
            cout << errMsgStart << "Elevator::transition() => floor = " << floor << endl;
#endif

            if ((ELEVATOR_FLOOR_BOTTOM <= floor) && (ELEVATOR_FLOOR_TOP >= floor))
            {
                cout << "Elevator is at floor " << floor << endl;
                
                if (floor != lastFloor)
                {
                    lastFloor = floor;
                    if (!updateCurrentPosition(floor))
                        throw ("Failed updating current position");
                    if (!updateStats(floor))
                        throw ("Failed updating stats");
                }
                sleep(secondsPerFloor);
            }
            else if (0 == floor)
            {
                cout << "Elevator is moving" << endl;
                sleep(secondsPerFloor);
            }   
            else
                throw ("Undefined elevator state");
        }
        catch (const exception& ex)
        {
            cout << errMsgStart << ex.what() << endl;
            continue;
        }
        catch (const char* msg)
        {
            cout << errMsgStart << msg << endl;
            continue;
        }
    } // End while(1)

    cout << "Ending elevator moving program" << endl;
    return 0;
} // End main



bool updateStats(int floor)
{
    try
    {
        DB db(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

        // Find enum for the floor value
        elevatorStatsOffsets floorEnum;
        list<string> floorCol;
        switch (floor)
        {
            case 1:
                floorEnum = elevatorStatsOffsets::floor_one_count;
                floorCol.push_back(elevatorStats[(int)elevatorStatsOffsets::floor_one_count]);
                break;
            case 2:
                floorEnum = elevatorStatsOffsets::floor_two_count;
                floorCol.push_back(elevatorStats[(int)elevatorStatsOffsets::floor_two_count]);
                break;
            case 3:
                floorEnum = elevatorStatsOffsets::floor_three_count;
                floorCol.push_back(elevatorStats[(int)elevatorStatsOffsets::floor_three_count]);
                break;
            default:
                throw ("Invalid floor number");
        }

        // Select value to get current value
    	vector<vector<string>> statsResults = db.select(tbElevatorStats, floorCol, {}, DBCond::LOGICAL_RELATION_JOIN::AND);
        if(statsResults.size() <= 1)
            throw ("Found no stats.");
        
        int floorCount = stoi(statsResults[1][0]); // Row zero is types, only 1 column

        DBCond setStat;
        setStat.init(elevatorStats[(int)floorEnum], DBCond::RELATION::EQ, ++floorCount);
        list<DBCond> statsUpdate = {setStat};

        if(!db.update(tbElevatorStats, statsUpdate, {}, DBCond::LOGICAL_RELATION_JOIN::AND, true))
            throw ("Failed updating stats");

#ifdef DEBUG
        cout << errMsgStart << floor << " count is " << floorCount << endl;
#endif

    }
    catch(const exception& ex)
    {
        cout << errMsgStart << "Failed updating stats: " << ex.what() << endl;
        return false;
    }    
    catch(const char* msg)
    {
        cout << errMsgStart << "Failed updating stats: " << msg << endl;
        return false;
    }
    return true;
}


bool updateCurrentPosition(int floor)
{
    try
    {
        if ((floor > ELEVATOR_FLOOR_TOP) || (floor < ELEVATOR_FLOOR_BOTTOM))
            if ((FLOOR_GO_UP != floor) || (FLOOR_GO_DOWN != floor))
                throw ("Invalid floor number");

        DB db(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

        DBCond setFloor;
        setFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, floor);

        DBCond whereStatus;
        whereStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, CURRENT_POS_STATUS);

        db.update(tbElevatorNetwork, {setFloor}, {whereStatus}, DBCond::LOGICAL_RELATION_JOIN::AND);
    }
    catch(const exception& ex)
    {
        cout << errMsgStart << "Failed updating current position: " << ex.what() << endl;
        cout << "\tInput floor no.: " << floor << endl;
        return false;
    }    
    catch(const char* msg)
    {
        cout << errMsgStart << "Failed updating current position: " << msg << endl;
        cout << "\tInput floor no.: " << floor << endl;
        return false;
    }
    return true;
}


bool addSabbathModeEntry(int node, int floor)
{
    DB db(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

    DBCond setNode;
    setNode.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, node);
    DBCond setStatus;
    setStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
    DBCond setFloor;
    setFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, floor);

    // Generate and perform the insert query
    return db.insert(tbElevatorNetwork, {setNode, setStatus, setFloor});
}
