/*
 * File to mimic the elevator web control by adding entries
 *		to the database for the elevator motion code use.
 * 
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */


#include "../../include/DB_Class.h"
#include <iostream>

int main()
{
	// Open the elevator and select the schema
	DB db(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

	while(1)
	{
		// Give the user some options (mimicking buttons)
		int floor = -1;
		int node = -1;
		string userIn = "";

		while((0 > floor) || (0 > node))
		{
			userIn = "";
			cout << "Pick an option:" << endl;
			cout << "Inside options: To (1), To (2), To (3)" << endl;
			cout << "Outside options: Up from 1 (1up), Down from 2 (2dn), Up from 2 (2up), Down from 3 (3dn)" << endl;

			cin.clear();
			cin >> userIn;

			// WOULD PROBABLY BE BETTER TO USE ENUMS FOR VALUE ASSIGNMENTS LATER
			if (userIn == btnNames[(int)button::one])
			{
				node = 0;
				floor = 1;
			}
			else if (userIn == btnNames[(int)button::two])
			{
				node = 0;
				floor = 2;
			}
			else if (userIn == btnNames[(int)button::three])
			{
				node = 0;
				floor = 3;
			}
			else if (userIn == btnNames[(int)button::upFrom1])
			{
				node = 1;
				floor = 5;
			}
			else if (userIn == btnNames[(int)button::dnFrom2])
			{
				node = 2;
				floor = 4;
			}
			else if (userIn == btnNames[(int)button::upFrom2])
			{
				node = 2;
				floor = 5;
			}
			else if (userIn == btnNames[(int)button::dnFrom3])
			{
				node = 3;
				floor = 4;
			}
			else
			{
				node = -1;
				floor = -1;
				if(cin.fail())
				{
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
				}
			}
		}

		// YUCKY initialization of input values
		DBCond tempCond1;
		tempCond1.init(elevatorNetwork[(int)elevatorNetworkOffsets::timestamp], DBCond::RELATION::EQ, "DEFAULT");
		DBCond tempCond2;
		tempCond2.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, node);
		DBCond tempCond3;
		tempCond3.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
		DBCond tempCond4;
		tempCond4.init(elevatorNetwork[(int)elevatorNetworkOffsets::floor], DBCond::RELATION::EQ, floor);

		std::list<DBCond> insertConds = {tempCond1, tempCond2, tempCond3, tempCond4};

		// Generate and perform the insert query
		int rowsChanged = db.insert(tbElevatorNetwork, insertConds);
		if (!rowsChanged)
		{
			cout << "An error occured. No rows should have been added." << endl;
			// return -1;
		}
		else 
		{
			cout << "Success! " << rowsChanged << " rows inserted." << endl;
		}
	}

	return 0;
}
