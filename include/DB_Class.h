/*
 * DB_Class.h
 *
 * Contains DB class prototype
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 


#ifndef _DB_CLASS_
#define _DB_CLASS_

#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <string>
#include "DB_DEFS.h"
#include "DB_Conds_Class.h"

using namespace std;

class DB
{
	string Query;
	string Address;
	string Username;
	string Password;
	string Schema;
	sql::Connection *Con;

	sql::ResultSet* executeRead();
	int executeWrite();
	bool setSchema(string);
	bool setSchema();

public:
	DB(string, string, string, string);

	bool openConnection();
	bool openConnection(string, string, string, string);
	bool closeConnection();

	string getSchema();

	void setQuery(string);    // Get rid of?
	string getQuery();

	sql::ResultSet* select(string, std::list<string>, std::list<DBCond>);
	int update(string, std::list<DBCond>, std::list<DBCond>, bool overwrite = false);
	int insert(string, std::list<DBCond>);
	int _delete(string, std::list<DBCond>, bool overwrite = false);
};

#endif
