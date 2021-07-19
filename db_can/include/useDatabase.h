#ifndef _USE_DB_H_
#define _USE_DB_H_

#include "../../include/DB_Class.h"

// #define		SCHEMA				"elevator"
// #define		ELEVATOR_TABLE		"elevatorNetwork"
// #define		STATS_TABLE			"elevatorStats"
// #define		ADDRESS				"tcp://localhost:3306"
// #define		USERNAME			"choeksema"
// #define		PASSWORD			"eMach1ne"

/*#define 	COL_CUR_FLOOR		"currentFloor"
#define		COL_REQ_FLOOR		"requestedFloor"
#define 	COL_NODE_ID			"nodeID"

#define		COL_STAT_FLOOR_1	"floor_one_count"
#define		COL_STAT_FLOOR_2	"floor_two_count"
#define		COL_STAT_FLOOR_3	"floor_three_count"
#define 	COL_STAT_DATETIME	"date_set"*/

// #define		ID_CAR_NODE			1U


//static string ELEVATOR_COLS[7] = {"date", "time", "nodeID", "status", "currentFloor", "requestedFloor", "otherInfo"};


bool getFloorValues(int*, int*);
bool setReqFloor(int);
bool resetElevatorStats();
bool updateElevatorStats(int);
bool addNewRows(int, int, int, string info = "elevatorCar");

#endif
