/*
 * File managing database interactions on the server
 * 
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */

#include <iostream>
#include <sstream>
#include "../include/useDatabase.h"
// #include <cppconn/resultset.h>

static string errMsgStart = (string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: ";

using namespace std;


bool getFloorValues(int* curFloor, int* reqFloor)
{
	// Set to default output value
	*curFloor = 0;
	*reqFloor = 0;

	// Initialize the database
	DB database = DB(elevatorAddr, elevatorUser, elevatorPswd, dbElevator);

	if (!database.openConnection())
	{
		cout << errMsgStart << "Failed opening database. Please check credentials." << endl;
		return false;
	}

	/*if (!database.setSchema(dbElevator))
	{
		cout << errMsgStart << "Failed selecting schema. Please check if table exists." << endl;
		return false;
	}*/

	// Generate a query
	stringstream ss;
	ss << "Connected to DB";
	// ss << "SELECT " << elevatorNetwork[elevatorNetworkOffsets::currentFloor]/* << "," << COL_REQ_FLOOR << " FROM " << ELEVATOR_TABLE*/ << " WHERE " << elevatorNetwork[elevatorNetworkOffsets::nodeID] << "=" << ID_CAR_NODE;
	// database.setQuery(ss.str());

	cout << ss.str() << endl;
	/*sql::ResultSet* results = database.select();

	if (nullptr == results)
	{
		cout << errMsgStart << "Current floor not found." << endl;
		return false;
	}

	while(results->next())
	{
		try
		{
			*curFloor = results->getInt(COL_CUR_FLOOR);
			*reqFloor = results->getInt(COL_REQ_FLOOR);
		}
		catch(const exception& ex)
		{
			cout << errMsgStart << "Failed interpreting query results: " << ex.what() << endl;
			return false;
		}
	}

	if(!database.closeConnection())
	{
		cout << errMsgStart << "Failed closing database." << endl;
		return false;
	}*/

	return true;
}


/*bool setReqFloor(int curFloor)
{
	// Initialize the database
	DB database = DB(ADDRESS, USERNAME, PASSWORD);

	if (!database.openConnection())
	{
		cout << errMsgStart << "Failed opening database. Please check credentials." << endl;
		return false;
	}

	if (!database.setSchema(SCHEMA))
	{
		cout << errMsgStart << "Failed selecting schema. Please check if database exists." << endl;
		return false;
	}

	// Generate a query
	stringstream ss;
	ss << "UPDATE " << ELEVATOR_TABLE << " SET " << COL_CUR_FLOOR << "=" << curFloor << " WHERE " << COL_NODE_ID << "=" << ID_CAR_NODE;
	database.setQuery(ss.str());

	if(!database.update())
	{
		cout << errMsgStart << "Failed updating the database" << endl;
		return false;
	}

	if(!database.closeConnection())
	{
		cout << errMsgStart << "Failed closing the database" << endl;
		return false;
	}
	
	if(!updateElevatorStats(curFloor))
	{
		cout << errMsgStart << "Failed updating stats database" << endl;
	}

	return true;
}


bool addNewRows(int status, int curFloor, int reqFloor, string info)
{
	DB db = DB(ADDRESS, USERNAME, PASSWORD);
	
	cout << "DB open" << endl;

	if (!db.openConnection())
	{
		cout << errMsgStart << "Failed opening database. Please check credentials." << endl;
		return false;
	}
	
	if (!db.setSchema(SCHEMA))
	{
		cout << errMsgStart << "Failed selecting schema. Please check if database exists." << endl;
		return false;
	}

	cout << "Creating query: " << to_string((int)(sizeof(ELEVATOR_COLS)/sizeof(*ELEVATOR_COLS))) <<  endl;
	string value;
	cin >> value;

	stringstream ss;
	ss << "INSERT INTO " << ELEVATOR_TABLE << "(";
	for (int i=0; i < (int)(sizeof(ELEVATOR_COLS)/sizeof(*ELEVATOR_COLS)); i++)
	{
		ss << ELEVATOR_COLS[i];

		if (i < (int)(sizeof(ELEVATOR_COLS)/sizeof(*ELEVATOR_COLS)) - 1)
			ss << ",";
	}

	ss << ") VALUES (CURRENT_DATE, CURRENT_TIME, DEFAULT," << to_string(status) << "," << to_string(curFloor) << "," << to_string(reqFloor) << ",'" << info << "');";
	db.setQuery(ss.str());

	cout << ss.str() << endl;
	cin >> value;
	
	if(!db.insert())
	{
		cout << errMsgStart << "Failed updating the database" << endl;
		return false;
	}

	cout << ss.str() << endl;
	cin >> value;

	if(!db.closeConnection())
	{
		cout << errMsgStart << "Failed closing the database" << endl;
		return false;
	}
	
	return true;
}*/


/*bool deleteOldRows()
{

}*/


/*bool resetElevatorStats()
{
	DB db = DB(ADDRESS, USERNAME, PASSWORD);
	
	if (!db.openConnection())
	{
		cout << errMsgStart << "Failed opening database. Please check credentials." << endl;
		return false;
	}
	
	if (!db.setSchema(SCHEMA))
	{
		cout << errMsgStart << "Failed selecting schema. Please check if database exists." << endl;
		return false;
	}
	
	stringstream ss;
	ss << "UPDATE " << STATS_TABLE << " SET " 
	   << COL_STAT_FLOOR_1 << "=DEFAULT," 
	   << COL_STAT_FLOOR_2 << "=DEFAULT,"
	   << COL_STAT_FLOOR_3 << "=DEFAULT,"
	   << COL_STAT_DATETIME << "=NOW()";
	db.setQuery(ss.str());
	
	if(!db.update())
	{
		cout << errMsgStart << "Failed updating the database" << endl;
		return false;
	}

	if(!db.closeConnection())
	{
		cout << errMsgStart << "Failed closing the database" << endl;
		return false;
	}
	
	return true;
}


bool updateElevatorStats(int floor)
{
	string colName = "";
	int currentFloorCount = 0;
	switch (floor)
	{
		case 1:
			colName = COL_STAT_FLOOR_1;
			break;
		case 2:
			colName = COL_STAT_FLOOR_2;
			break;
		case 3:
			colName = COL_STAT_FLOOR_3;
			break;
		default:
			cout << "The requested floor doesn't exist" << endl;
			return false;
	}
	
	DB db = DB(ADDRESS, USERNAME, PASSWORD);
	
	if (!db.openConnection())
	{
		cout << errMsgStart << "Failed opening database. Please check credentials." << endl;
		return false;
	}
	
	if (!db.setSchema(SCHEMA))
	{
		cout << errMsgStart << "Failed selecting schema. Please check if database exists." << endl;
		return false;
	}
	
	// get current value
	stringstream ss;
	ss << "SELECT " << colName << " FROM " << STATS_TABLE;
	db.setQuery(ss.str());

	sql::ResultSet* results = db.select();

	if (nullptr == results)
	{
		cout << errMsgStart << "Current floor not found." << endl;
		return false;
	}
*/	/*else if(results->getRow() != 1)
	{
		cout << errMsgStart << "Unexpected number of records (" << results->getRow() << ")." << endl;
		cout << errMsgStart << colName << endl;
		return false;
	}*/

/*	while(results->next())
	{
		try
		{
			currentFloorCount = results->getInt(colName);
		}
		catch(const exception& ex)
		{
			cout << errMsgStart << "Failed interpreting query results: " << ex.what() << endl;
			return false;
		}
	}
	
	// update value
	ss.str("");
	ss.clear();
	ss << "UPDATE " << STATS_TABLE << " SET " << colName << "=" << (currentFloorCount + 1);
	db.setQuery(ss.str());
	
	if(!db.update())
	{
		cout << errMsgStart << "Failed updating the database" << endl;
		return false;
	}

	if(!db.closeConnection())
	{
		cout << errMsgStart << "Failed closing the database" << endl;
		return false;
	}
	
	return true;
}*/
