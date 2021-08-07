/*
 * DB_DEFS.h
 *
 * Contains general database definitions so programmers don't
 *		need to type in strings.
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 


#ifndef _DB_DEFS_H_
#define _DB_DEFS_H_

using namespace std;


#define		MOVE_STATUS      	0
#define	 	MOVING_NOW_STATUS	1
#define		CURRENT_POS_STATUS	2

#define 	ORDER_ASCEND		0
#define		ORDER_DESCEND		1


// General database definitions
static string dbElevator = "elevator";
static string elevatorUser = "ese";
static string elevatorPswd = "ese";
static string elevatorAddr = "tcp://localhost:3306";


// oldElevatorNetwork table definitions
static string tbOldElevatorNetwork = "oldElevatorNetwork";

// Using typedefs would be better
enum class oldElevatorNetworkOffsets {
	date,
	time,
	nodeID,
	status,
	currentFloor,
	requestedFloor,
	otherInfo,
	First = date,
	Last = otherInfo
};

static string oldElevatorNetwork[(int)oldElevatorNetworkOffsets::Last + 1] = {
	"date",
	"time",
	"nodeID",
	"status",
	"currentFloor",
	"requestedFloor",
	"otherInfo"
};

// Move elevator request inputs
typedef enum class button {
	one,
	two,
	three,
	upFrom1,
	dnFrom2,
	upFrom2,
	dnFrom3,
	First = one,
	Last = dnFrom3
} button;

static string btnNames[(int)button::Last + 1] = {"1","2","3","1up","2dn","2up","3dn"};


// elevatorNetwork table definitions
static string tbElevatorNetwork = "elevatorNetwork";

typedef enum class elevatorNetworkOffsets {
	timestamp,
	nodeID,
	status,
	floor,
	First = timestamp,
	Last = floor
} elevatorNetworkOffsets;

static string elevatorNetwork[(int)elevatorNetworkOffsets::Last + 1] = {
	"timestamp",
	"nodeID",
	"status",
	"floor"
};


// elevatorStats table definitions
static string tbElevatorStats = "elevatorStats";

enum class elevatorStatsOffsets {
	date_set,
	floor_one_count,
	floor_two_count,
	floor_three_count,
	First = date_set,
	Last = floor_three_count
};

static string elevatorStats[(int)elevatorStatsOffsets::Last + 1] = {
	"date_set",
	"floor_one_count",
	"floor_two_count",
	"floor_three_count"
};

#endif
