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
		int selection = -1;
		string userIn = "";

		while((0 > selection) || (selection > 6))
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
				selection = (int)button::three;
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

		// YUCKY initialization of input values
		DBCond tempCond1;
		tempCond1.init(elevatorNetwork[(int)elevatorNetworkOffsets::date], DBCond::RELATION::EQ, "CURRENT_DATE");	
		DBCond tempCond2;
		tempCond2.init(elevatorNetwork[(int)elevatorNetworkOffsets::time], DBCond::RELATION::EQ, "CURRENT_TIME");
		DBCond tempCond3;
		tempCond3.init(elevatorNetwork[(int)elevatorNetworkOffsets::nodeID], DBCond::RELATION::EQ, "DEFAULT");
		DBCond tempCond4;
		tempCond4.init(elevatorNetwork[(int)elevatorNetworkOffsets::status], DBCond::RELATION::EQ, MOVE_STATUS);
		DBCond tempCond5;
		tempCond5.init(elevatorNetwork[(int)elevatorNetworkOffsets::currentFloor], DBCond::RELATION::EQ, 0);
		DBCond tempCond6;
		tempCond6.init(elevatorNetwork[(int)elevatorNetworkOffsets::requestedFloor], DBCond::RELATION::EQ, selection);
		DBCond tempCond7;
		tempCond7.init(elevatorNetwork[(int)elevatorNetworkOffsets::otherInfo], DBCond::RELATION::EQ, "elevatorCar");

		std::list<DBCond> insertConds = {tempCond1, tempCond2, tempCond3, tempCond4, tempCond5, tempCond6, tempCond7};

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
