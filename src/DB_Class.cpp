/*
 * DB_Class.cpp
 *
 * Contains DB class definitions
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 

#include "../include/DB_Class.h"
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <iostream>
#include <sstream>
#include "../include/string_extend.h"
#include <cppconn/resultset_metadata.h>

#define		errMsgStart		(string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: "

using namespace std;


/* 
 * Constructor method
 *
 * Inputs:
 * 		string addr: address of the DB connection
 * 		string user: username to use to connect
 * 		string pswd: password for the supplied username
 * 
 * Returns: N/A
 */
DB::DB(string addr, string user, string pswd, string schema)
{
	Address = addr;
	Username = user;
	Password = pswd;
	Schema = schema;
}


/*
 * Method to open a connection to the database
 * 
 * Inputs: N/A
 *
 * Returns bool: indicates success
 */
bool DB::openConnection()
{
	if (Address.empty() || Username.empty() || Password.empty() || Schema.empty())
	{
		cout << errMsgStart << "Not enough parameters to open database: address=" << Address 
		<< ", username=" << Username << ", password=" << Password << ", schema=" << Schema << endl;
		return false;
	}

	try
	{
		sql::Driver* driver = get_driver_instance();
		Con = driver->connect(Address, Username, Password);

		if (!setSchema(Schema))
			throw "Failed schema set";
		
		return true;
	}
	catch(const std::exception& ex)
	{
		cout << "Could not connect to '" << Address << "' as '" << Username 
			 << "' with password '" << Password << "' at database '" << Schema << "'" << endl;
		cout << errMsgStart << ex.what() << endl;
		return false;
	}

	return true;
}


/*
 * Method to open a connection to the database. Sets the 
 * database variables and calls the other DB::openConnection()
 * 
 * Inputs:
 * 		string addr: address of the database
 * 		string user: username to use to connect
 *		string pswd: password for the supplied username
 *
 * Returns bool: indicates success
 */
bool DB::openConnection(string addr, string user, string pswd, string schema)
{
	Address = addr;
	Username = user;
	Password = pswd;
	Schema = schema;
	return openConnection();
}


/*
 * Method to close a connection to the database
 * 
 * Inputs: N/A
 *
 * Returns bool: indicates success
 */
bool DB::closeConnection()
{
	try
	{
		delete Con;    // Not sure here..
	}
	catch(const exception& ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return false;
	}
	return true;
}


/*
 * Method to select a specific schema to use
 * 
 * Inputs: 
 * 		string reqSchema: the name of the schema to use
 *
 * Returns bool: indicates success
 */
bool DB::setSchema()
{
	if (Schema.empty())
		return false;

	try
	{
		Con->setSchema(Schema);
	}
	catch (const exception& ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return false;
	}
	return true;
}


/*
 * Method to select a specific schema to use
 * 
 * Inputs: 
 * 		string reqSchema: the name of the schema to use
 *
 * Returns bool: indicates success
 */
bool DB::setSchema(string reqSchema)
{
	Schema = reqSchema;
	return setSchema();
}


/*
 * Method to return the name of the schema in use
 * 
 * Inputs: N/A
 *
 * Returns string: name of the current schema
 */
string DB::getSchema()
{
	try
	{
		Schema = (string)Con->getSchema();
	}
	catch (const exception& ex)
	{
		cout << errMsgStart << "getSchema failed: " << ex.what() << endl;
		return "";
	}

	return Schema;
}


/*
 * Method to set the query member of the DB class
 * 
 * Inputs:
 *		string reqQuery: the requested query
 *
 * Returns: N/A
 */
void DB::setQuery(string reqQuery)
{
	Query = reqQuery;
}


/*
 * Method to return the query member of the DB class
 * 
 * Inputs: N/A
 *
 * Returns string: the query member
 */
string DB::getQuery()
{
	return Query;
}


/*
 * Method to execute read queries (eg: select) on the DB
 * 
 * Inputs: N/A
 *
 * Returns <vector<vector<string>>: vector of each vectors where each row is a selected row from the database.
 * 		The first row indicates the types from the database. (empty set indicates error)
 */
vector<vector<string>> DB::executeRead()
{
	if (Query.empty())
		return {};

	vector<vector<string>> results;

	try
	{
		if (!openConnection())
			throw "Failed open";

		// Perform query
		sql::Statement* stmt = Con->createStatement();
		sql::ResultSet* res = stmt->executeQuery(Query);

		results = resultsToVector(res);

		// Cleanup
		delete stmt;
		delete res;

		if(!closeConnection())
			throw "Failed close";
	}
	catch(const exception& ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return {};
	}
	catch(const char* msg)
	{
		cout << errMsgStart << msg << endl;
		return {};
	}

	return results;
}


/*
 * Method to execute write queries on the DB
 * Note: user should check that the query to be edited exists or else
 * 		the function will return on error.
 * DB Transactions may be implemented later if possible.
 * 
 * Inputs: N/A
 *
 * Returns int: the number of rows affected (0 indicates error)
 */
int DB::executeWrite()
{
	if (Query.empty())
		return 0;    // indicate no rows updated (fail)

	int rowsChanged = 0;

	try
	{
		if (!openConnection())
			throw "Failed open";

		sql::PreparedStatement* pstmt = Con->prepareStatement(Query);

		rowsChanged = pstmt->executeUpdate();

		if (!rowsChanged)
			throw "Failed update";    // May indicate that no rows matched: check using DB::select(...) first

		delete pstmt;

		if(!closeConnection())
			throw "Failed close";
	}
	catch(const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return 0;
	}
	catch(const char* msg)
	{
		cout << "Write Error: " << msg << endl;
		cout << "Warning: Table may have been edited. Should use DB transactions if possible." << endl;
		return 0;
	}

	return rowsChanged;
}


/*
 * Method to translate a ResultSet into a vector of vectors. Each inner row contains
 *		the results from a single row in the database. The first row indicates the types 
 * 		contained in the respective column.
 * This method exists since calls to sql::ResultSet::next() fail on Rasbian (ARM64) after
 *		the sql::Connection pointer is deleted (in DB::closeConnection(). It also allows 
 *		the sql::ResultSet pointer to be cleaned up in the same function where it was created.
 * 
 * Inputs: N/A
 *
 * Returns <vector<vector<string>>: the translated results (empty set indicates error)
 */
vector<vector<string>> DB::resultsToVector(sql::ResultSet* rawResults)
{
	// Variables
	vector<vector<string>> results;
	vector<string> row;

	try
	{
		// Get info about columns to save results
		sql::ResultSetMetaData* metaData = rawResults->getMetaData();
		int colCount = metaData->getColumnCount();

		// Save all datatypes for parsing later
		for (int i=1; i <= colCount; i++)    // DB result info is NOT OFFSET FROM 0!
		{
			row.push_back(metaData->getColumnTypeName(i));
		}
		results.push_back(row);

		// Save actual results
		while (rawResults->next())
		{
			row.clear();
			for (int i=1; i <= colCount; i++)
			{
				row.push_back(rawResults->getString(i));
			}
			results.push_back(row);
		}
	}
	catch (const exception& ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return {};
	}

	return results;
}


/*
 * Method to generate a select query. Also calls execution.
 * Should be able to specify the condition joiners!
 * Could be improved to warn users when they try select an entire table (might be a large table)
 * 
 * Inputs:
 * 		string table: the table to select from
 *		std::list<string> colNames: names of the columns to select
 * 		std::list<DBCond> conds: conditions for which rows to select
 *
 * Returns <vector<vector<string>>: 2D vector of query results (empty set indicates error)
 */
vector<vector<string>> DB::select(string table, std::list<string> colNames, std::list<DBCond> conds)
{
	vector<vector<string>> results;

	try
	{
		std::stringstream ss;
		ss << "SELECT ";

		if (colNames.empty())
			ss << "*";

		else
			ss << string_extend::join(colNames, ",");

		ss << " FROM " << table;

		if (!conds.empty())
		{
			ss << " WHERE ";

			list<string> tempConds;

			for(list<DBCond>::iterator it=conds.begin(); it != conds.end(); it++)
			{
				tempConds.push_back((*it).getExpression());
			}
			DBCond stupidInstatiantionThatShouldntBeNeeded;
			ss << stupidInstatiantionThatShouldntBeNeeded.join(tempConds, DBCond::LOGICAL_RELATION_JOIN::AND);
		}

		ss << ";";

		Query = ss.str();
		// cout << "Query: " << Query << endl;
		results = executeRead();

		if (results.size() == 0)
			throw "Bad results";    // Should probably create new exceptions and stuff

	}
	catch(const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return {};
	}
	catch(const char* msg)
	{
		cout << errMsgStart << "Select error: " << msg << endl;
		return {};
	}

	return results;
}


/*
 * Method to generate an update query. Also calls execution.
 * Should be able to specify the condition joiners!
 * Column list with values really should be a key-value pair or something and not a list of DBConds
 * 
 * Inputs:
 * 		string table: the table to update from
 *		std::list<string> colNames: names of the columns to update
 * 		std::list<DBCond> conds: conditions for which rows to update
 *		bool overwrite (default false): indicates if the user wishes to overwrite the entire table
 * 			This is likely an uncommon occurence. Only does anything if conds is empty.
 *
 * Returns int: number of entries updated (0 indicates error)
 */
int DB::update(string table, list<DBCond> columns, list<DBCond> conds, bool overwrite)
{
	int updatedRows = 0;
	try
	{
		if (columns.empty())
		{
			cout << "No updates specified.";
			return 0;    // Indicates no rows changed (error)
		}
		
		DBCond stupidInstatiantionThatShouldntBeNeeded;
		
		list<string> tempCols;
		
		for(list<DBCond>::iterator it=columns.begin(); it != columns.end(); it++)
		{
			tempCols.push_back((*it).getExpression());
		}

		std::stringstream ss;
		ss << "UPDATE " << table << " SET " << string_extend::join(tempCols, ",");

		if (!overwrite && conds.empty())
		{
			cout << "Query is set to overwrite entire table. If you want to do that, set the 'overwrite' flag." << endl;
			return 0;
		}

		else if (!conds.empty())
		{
			ss << " WHERE ";

			list<string> tempConds;

			for(list<DBCond>::iterator it=conds.begin(); it != conds.end(); it++)
			{
				tempConds.push_back((*it).getExpression());
			}
			ss << stupidInstatiantionThatShouldntBeNeeded.join(tempConds, DBCond::LOGICAL_RELATION_JOIN::AND);
		}

		ss << ";";

		Query = ss.str();
		// cout << "Query: " << Query << endl;
		updatedRows = executeWrite();
		if(!updatedRows)
			throw "Failed update.";
	}
	catch(const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return 0;
	}
	catch(const char* msg)
	{
		cout << errMsgStart << "Update error: " << msg << endl;
		return 0;
	}

	return updatedRows;
}


/*
 * Method to generate an insert query. Also calls execution.
 * 
 * Inputs:
 * 		string table: the table to insert into
 *		std::list<DBCond> newValues: the columns to set and their values. 
 *			RELATION element is ignored. Should really be a different type.
 *
 * Returns int: number of inserted entries (0 indicates error)
 */
int DB::insert(string table, std::list<DBCond> newValues)
{
	int addedRows = 0;
	try
	{
		if(newValues.empty())
		{
			cout << "Invalid input" << endl;
			return 0;
		}

		stringstream ss;
		ss << "INSERT INTO " << table << "(" << DBCond::joinColumns(newValues, ",") 
			<< ") VALUES (" << DBCond::joinValues(newValues, ",") << ");";

		Query = ss.str();
		cout << Query << endl;
		addedRows = executeWrite();
		if(!addedRows)
			throw "Failed insert.";
	}
	catch(const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return 0;
	}
	catch(const char* msg)
	{
		cout << errMsgStart << "Insert error: " << msg << endl;
		return 0;
	}
	return addedRows;
}


/*
 * Method to generate a delete query. Also calls execution.
 * Should be able to specify the condition joiners!
 * 
 * Inputs:
 * 		string table: the table to delete from
 * 		std::list<DBCond> conds: conditions for which rows to delete
 *		bool overwrite (default false): indicates if the user wishes to overwrite the entire table
 * 			This is likely an uncommon occurence. Only does anything if conds is empty.
 *
 * Returns int: number of entries updated (0 indicates error)
 */
int DB::_delete(string table, std::list<DBCond> conds, bool overwrite)
{
	int deletedRows = 0;
	try
	{
		std::stringstream ss;
		ss << "DELETE FROM " << table;

		if (!overwrite && conds.empty())
		{
			cout << "Query is set to delete all records. If you want to do that, set the 'overwrite' flag." << endl;
			return 0;
		}

		else if (!conds.empty())
		{
			ss << " WHERE ";

			list<string> tempConds;

			for(list<DBCond>::iterator it=conds.begin(); it != conds.end(); it++)
			{
				tempConds.push_back((*it).getExpression());
			}
			DBCond stupidInstatiantionThatShouldntBeNeeded;
			ss << stupidInstatiantionThatShouldntBeNeeded.join(tempConds, DBCond::LOGICAL_RELATION_JOIN::AND);
		}

		ss << ";";

		Query = ss.str();
		// cout << "Query: " << Query << endl;
		deletedRows = executeWrite();
		if(!deletedRows)
			throw "Failed delete.";
	}
	catch(const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return 0;
	}
	catch(const char* msg)
	{
		cout << errMsgStart << "Update error: " << msg << endl;
		return 0;
	}

	return deletedRows;
}
