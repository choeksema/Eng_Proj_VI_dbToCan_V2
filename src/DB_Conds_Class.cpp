/*
 * DB_Conds_Class.cpp
 *
 * Contains DB conditions class definitions
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 


#include <string>
#include <iostream>
#include "../include/DB_Conds_Class.h"
#include "string_extend.h"

using namespace std;

// String doesn't quite work yet --> could fix with a logger and <sourcelocation>
static string errMsgStart = (string)"<" + (string)__FILE__ + (string)"@" + to_string(__LINE__) + (string)">: ";


/*
 * Method to initialize a DBCond element.
 * 
 * Inputs:
 * 		string col: the name of the column
 *		RELATION rel: the desired relation between the column and the value in it
 *		string val: the value to compare the column to
 *
 * Returns bool: indicates success of initialization (false indicates error)
 */
bool DBCond::init(string col, RELATION rel, string val)
{
	try
	{
		Column = col;
		Relation = Rels[(int)rel];

		// Don't add quotes if the string is a special SQL string
		for (int i=0; i<(int)(sizeof(SpecialMySqlStrings)/sizeof(*SpecialMySqlStrings)); i++)
		{
			if(val == SpecialMySqlStrings[i])
			{
				Value = val;
				break;
			}
		}

		// If not set above
		if (Value.empty() || Value != val)
			Value = "\"" + val + "\"";

		// Generate the expression
		Expression = parse();
	}
	catch (const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return false;	
	}
	return true;
}


/*
 * Method to initialize a DBCond element.
 * 
 * Inputs:
 * 		string col: the name of the column
 *		RELATION rel: the desired relation between the column and the value in it
 *		int val: the value to compare the column to
 *
 * Returns bool: indicates success of initialization (false indicates error)
 */
bool DBCond::init(string col, RELATION rel, int val)
{
	try
	{
		Column = col;
		Relation = Rels[(int)rel];
		Value = to_string(val);

		Expression = parse();
	}
	catch (const exception &ex)
	{
		cout << errMsgStart << ex.what() << endl;
		return false;	
	}

	return true;
}


/*
 * Method to generate the condition expression.
 * 
 * Inputs: N/A
 *
 * Returns string: generated condtion string
 */
string DBCond::parse()
{
	return (Column + Relation + Value);
}


/*
 * Public method to return the condition expression
 * 
 * Inputs: N/A
 *
 * Returns string: condition expression
 */
string DBCond::getExpression()
{
	return Expression;
}


/*
 * Method to join condition expressions.
 * Ideally would be a static function but it references the static private member LogRels
 * 		which seems to cause difficulties if the method is static.
 * 
 * Inputs:
 * 		std::list<string> listIn: list of condition expression
 *		DBCond::LOGICAL_RELATION_JOIN sep: condition to put between all expressions
 *
 * Returns string: joined condition list
 */
string DBCond::join(std::list<string> listIn, DBCond::LOGICAL_RELATION_JOIN sep)
{
	string tempSep = LogRels[(int)sep];

	return string_extend::join(listIn, tempSep);
}


/*
 * Method to join all columns by the supplied separator
 * 
 * Inputs:
 * 		std::list<DBCond> listIn: list of conditions (containing column names)
 *		string sep: char/string to put between all column names
 *
 * Returns string: joined column list
 */
string DBCond::joinColumns(std::list<DBCond> listIn, string sep)
{
	std::list<string> tempList;

    for(list<DBCond>::iterator it=listIn.begin(); it != listIn.end(); it++)
    	tempList.push_back((*it).Column);

    return string_extend::join(tempList, sep);
}


/*
 * Method to join all condition values by the supplied separator
 * 
 * Inputs:
 * 		std::list<DBCond> listIn: list of conditions (containing values)
 *		string sep: char/string to put between all values
 *
 * Returns string: joined value list
 */
string DBCond::joinValues(std::list<DBCond> listIn, string sep)
{
	std::list<string> tempList;

    for(list<DBCond>::iterator it=listIn.begin(); it != listIn.end(); it++)
    	tempList.push_back((*it).Value);

    return string_extend::join(tempList, sep);
}