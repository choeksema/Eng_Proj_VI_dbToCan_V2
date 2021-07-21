/*
 * main.cpp
 *
 * File to manage moving the elevator
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */


/* 
 * Process:
 *   Website writes entries with status 0
 *   C++ finds entries with status 0 and moves the elevator accordingly
 *   C++ deletes the entries it found
 *   C++ updates the entry with status 2 with the new position of the elevator
 *   Website uses the entry with status 2 to get current position
 */

#include "../../include/DB_Class.h"
#include "../include/elevator.h"

using namespace std;

static string errMsgStart = (string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: ";


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
		int newState = elevator.transition(floor);
		if (newState == 0)
		    cout << "Elevator is moving" << endl;
		else
		    cout << "Elevator is at " << newState << endl;

		sleep(30);    // Really should start the sleep from arrival on at a floor instead of departure
		floor++;
		if (floor > 3)
		    floor = 1;

		now = time(0);
		dateTime = localtime(&now);
	    } // while (Sabbath time check)
	}


	int selection = -1;
	string userIn = "";

	while ((0 > selection) || (selection > 6))
	{
	    userIn = "";
	    cout << "Pick an option:" << endl;
	    cout << "Inside options: To (1), To (2), To (3)" << endl;
	    cout << "Outside options: Up from 1 (1up), Down from 2 (2dn), Up from 2 (2up), Down from 3 (3dn)" << endl;

	    cin.clear();
	    cin >> userIn;

	    if (userIn == btnNames[(int)button::one])
		selection = (int)button::one;
	    else if (userIn == btnNames[(int)button::two])
		selection = (int)button::two;
	    else if (userIn == btnNames[(int)button::three])
		selection = (int) button::three;
	    else if (userIn == btnNames[(int)button::upFrom1])
		selection = (int)button::upFrom1;
	    else if (userIn == btnNames[(int)button::dnFrom2])
		selection = (int)button::dnFrom2;
	    else if (userIn == btnNames[(int)button::upFrom2])
		selection = (int)button::upFrom2;
	    else if (userIn == btnNames[(int)button::dnFrom3])
		selection = (int)button::dnFrom3;
	    else
	    {
		selection = -1;
		if(cin.fail())
		{
		    cin.clear();
		    cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
	    }
	}
	int requestedFloor = -1;

	/* Check elevator brains for next action */
	int newState = elevator.transition(selection);
	if (newState == 0)
	{
	    cout << "Elevator is moving" << endl;
	    requestedFloor = newState;
	}
	else
	{
	    cout << "Elevator is at floor " << newState << "." << endl;
	    requestedFloor = newState;
	}


        /* Check for entries in the database */

        // Select columns
        /*list<string> columns = {
            elevatorNetwork[(int)elevatorNetworkOffsets::date], // Get date and time to identify the column again later
            elevatorNetwork[(int)elevatorNetworkOffsets::time],
            elevatorNetwork[(int)elevatorNetworkOffsets::requestedFloor]    // Get the request
        };

        // Set conditions (kind of gross)
        DBCond getStatus;
        getStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
        list<DBCond> conditions = {getStatus};

        // Execute query
        sql::ResultSet* newEntries = db.select(tbElevatorNetwork, columns, conditions);
        if(newEntries == nullptr)
        {
            cout << "No new entries found." << endl;
            cout << "Query: " << db.getQuery() << endl;
            continue;
        }
	// cout << "Performed query: " << db.getQuery() << endl;
	// cout << "Number of rows found: " << to_string(newEntries->getRow()) << endl;
	newEntries->beforeFirst();
        
	cout << "We have " << to_string(newEntries->rowsCount()) << ". Trying to read results now" << endl;*/

	//while (newEntries->next())
        //{
	// newEntries->first();
            // cout << "in the while loop" << endl;
	    /*try
            {*/
		// cout << "going through entries" << endl;
                /* Now actually move the elevator */
                // Gather values
               /* string date = newEntries->getString(elevatorNetwork[(int)elevatorNetworkOffsets::date]);
                string time = newEntries->getString(elevatorNetwork[(int)elevatorNetworkOffsets::time]);
                int requestedFloor = newEntries->getInt(elevatorNetwork[(int)elevatorNetworkOffsets::requestedFloor]);

                cout << "Found a request to go to floor " << requestedFloor << "." << endl;*/


                /* Activate elevator brains to see what to do next */
                /*int newState = elevator.transition(requestedFloor);
		cout << "Got a state" << endl;
                if (newState == 0)
                {
                    cout << "Elevator is moving" << endl;
                    requestedFloor = newState;
                }
                else
                {
                    cout << "Elevator is at floor " << newState << "." << endl;
                    requestedFloor = newState;
                }*/
                // Need to update the floor once the elevator reaches the new floor!

                /* Delete entry now that it is dealt with */
                /*DBCond dateCond;
                dateCond.init(elevatorNetwork[(int)elevatorNetworkOffsets::date], DBCond::RELATION::EQ, date);
                DBCond timeCond;
                timeCond.init(elevatorNetwork[(int)elevatorNetworkOffsets::time], DBCond::RELATION::EQ, time);

                list<DBCond> deleteConds = {dateCond, timeCond};
                if(!db._delete(tbElevatorNetwork, deleteConds))    // We know these rows should exist since we just 'SELECT'ed them
                {
                    cout << "Failed deleting entry." << endl;
                    cout << "Query: " << db.getQuery() << endl;
                    continue;
                }*/


                /* Update database with new floor */
                /*DBCond updateWhereStatus;
                updateWhereStatus.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, CURRENT_POS_STATUS);
                list<DBCond> updateCols = {updateWhereStatus};

                DBCond updateFloor;
                updateFloor.init(elevatorNetwork[(int)elevatorNetworkOffsets::currentFloor], DBCond::RELATION::EQ, requestedFloor);
                list<DBCond> updateConds = {updateFloor};

                if(!db.update(tbElevatorNetwork, updateConds, updateCols))
                {
                    cout << "Failed updating current position. Query: " << db.getQuery() << endl;
                }

                // If the elevator is moving, don't update or stats
                if (requestedFloor == 0)
                    continue;*/


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
                }
            }
            catch (const exception& ex)
            {
                cout << errMsgStart << ex.what() << endl;
                continue;
            }
        //}*/
    }

    cout << "Ending elevator moving program" << endl;

    return 0;
}
