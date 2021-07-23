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


int main(int argc, char** argv)
{
    cout << "Starting the elevator moving process" << endl;
    cout << "C++ Version: " << __cplusplus << endl;

    /* Set up */
    Elevator elevator;
    DB db(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

    cout << "WARNING: Currently there are no database interactions" << endl;

    // Check if stats should be reset?
    
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
    

    // Begin running loop
    while(1)
    {
	// Check for Sabbath mode
	time_t now = time(0);
	tm* dateTime = localtime(&now);

	int floor = 1;
	
	if (!overrideSabbathMode)
	{
	    while (demoMode || (((dateTime->tm_hour >= 18) && (dateTime->tm_wday == 5)) 
		&& ((dateTime->tm_hour <= 18) && (dateTime->tm_wday == 6))))    // Maybe use enums instead of values
	    {
		int newState = elevator.transition(NODE_CAR, floor);
		if (newState == 0)
		    cout << "Elevator is moving" << endl;
		else
		    cout << "Elevator is at " << newState << endl;

		sleep(30);    // Really should start the sleep from arrival on at a floor instead of departure

		// UPDATE STATS HERE

		floor++;
		if (floor > 3)
		    floor = 1;

		now = time(0);
		dateTime = localtime(&now);
	    } // while (Sabbath time check)
	} // if(!overrideSabbathMode)

        /* Check for entries in the database */

        // Select columns
        list<string> columns = {
            elevatorNetwork[(int)elevatorNetworkOffsets::timestamp], // to ID columns later
            elevatorNetwork[(int)elevatorNetworkOffsets::nodeID],
            elevatorNetwork[(int)elevatorNetworkOffsets::floor]    // REQ is split between nodID and floor
        };

        // Set conditions (kind of gross)
        DBCond getStatus;
        getStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
        list<DBCond> conditions = {getStatus};

        // Execute query
        vector<vector<string>> newEntries = db.select(tbElevatorNetwork, columns, conditions);
        if(newEntries.size() == 0)
        {
            cout << errMsgStart << "No new entries found." << endl;
            cout << "Query: " << db.getQuery() << endl;
            continue;
        }

	for (int row=1; row < (int)newEntries.size(); row++)    // First row is types
     	{
	    try
            {
                /* Now actually move the elevator */
                // Gather values
                string dateTime = newEntries[row][0];    // Use "columns" offsets and not ints
                int nodeID = stoi(newEntries[row][1]);
                int floor = stoi(newEntries[row][2]);

		cout << "timestamp: " << dateTime << endl;
		cout << "nodeID: " << nodeID << endl;
		cout << "floor: " << floor << endl;

		int requestedFloor = 0;

                /* Activate elevator brains to see what to do next */
                int newState = elevator.transition(nodeID, floor);
                if (0 == newState)
                {
                    cout << "Elevator is moving" << endl;
                    requestedFloor = newState;
                }
                else
                {
                    cout << "Elevator is at floor " << newState << "." << endl;
                    requestedFloor = newState;
                }

		/* Update status if elevator is moving */
		/*if (0 == requestedFloor)
		{
		    DBCond updateWhereTime;
		    updateWhereTime.init(elevatorNetwork[(int)elevatorNetworkOffsets::timestamp], DBCond::RELATION::EQ, dateTime);

		    list<DBCond> updateConds = {updateWhereTime};

		    DBCond updateStatus;
		    updateStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVING_NOW_STATUS);

		    list<DBCond> updateCols = {updateStatus};

		    int rowsUpdated = db.update(tbElevatorNetwork, updateCols, updateConds);
		    if(0 == rowsUpdated)
		    {
			cout << errMsgStart << "Rows not found to update status" << endl;
			cout << "Query: " << db.getQuery();
		    }
		}*/
		/* Delete entry if elevator is at the requested floor */
		// else
		// {
		    DBCond deleteWhereTime;
                    deleteWhereTime.init(elevatorNetwork[(int)elevatorNetworkOffsets::timestamp], DBCond::RELATION::EQ, dateTime);

                    list<DBCond> deleteConds = {deleteWhereTime};

		    int deleteResults = db._delete(tbElevatorNetwork, deleteConds);
                    if(0 == deleteResults)    // These rows should exist (were just 'SELECT'ed)
                    {
                	cout << errMsgStart << "Failed deleting entry." << endl;
                	cout << "Query: " << db.getQuery() << endl;
                	continue;
                    }
		// }


                /* Update database with new floor */
                DBCond updateWhereStatus;
                updateWhereStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, CURRENT_POS_STATUS);
                list<DBCond> updateCols = {updateWhereStatus};

                DBCond updateFloor;
                updateFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, requestedFloor);    // Later only update this if at a floor. Otherwise change status of request to 1
                list<DBCond> updateConds = {updateFloor};

		int updateRes = db.update(tbElevatorNetwork, updateConds, updateCols, true);
                if(0 == updateRes)
                {
                    cout << errMsgStart << "Failed updating current position" << endl;
		    cout << "Query: " << db.getQuery() << endl;
		    cout << "Later set up to try adding a new row" << endl;
                }

                // If the elevator is moving, don't update stats
                if (requestedFloor == 0)
                    continue;


                /* Update the stats table */
                /*try
                {
                    // Find enum for the floor value
                    elevatorStatsOffsets floorEnum;
                    list<string> floorCol;
                    switch (requestedFloor)
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
                            continue;
                    }

                    list<DBCond> allRows = {};
                    // Select value to get current value
                    sql::ResultSet* statsResults = db.select(tbElevatorStats, floorCol, allRows);
                    if(nullptr == statsResults)
                    {
                        cout << "Found no stats." << endl;
                        continue;
                    }
                    
                    int floorCount = 0;
                    while(statsResults->next())
                        floorCount = statsResults->getInt(elevatorStats[(int)floorEnum]);

                    DBCond setStat;
                    setStat.init(elevatorStats[(int)floorEnum], DBCond::RELATION::EQ, ++floorCount);
                    list<DBCond> statsUpdate = {setStat};

                    if(!db.update(tbElevatorStats, statsUpdate, allRows, true))
                    {
                        cout << "Failed updating stats" << endl;
                        continue;
                    }
                }
                catch(const exception& ex)
                {
                    cout << "Failed updating stats: " << ex.what() << endl;
                    continue;
                }*/
            }
            catch (const exception& ex)
            {
                cout << errMsgStart << ex.what() << endl;
                continue;
            }
        } // End for(rows)
	// sleep(10);
    } // End while(1)

    cout << "Ending elevator moving program" << endl;

    return 0;
}
