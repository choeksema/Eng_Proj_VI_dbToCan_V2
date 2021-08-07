/*
 * DB_Conds_Class.h
 *
 * Contains DB conditions class prototype.
 * Currently supports only basic conditions
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 


#ifndef _DB_CONDS_CLASS_
#define _DB_CONDS_CLASS_

#include <string>
#include <list>
#include "DB_DEFS.h"

using namespace std;

class DBCond
{
public:
	enum class RELATION {    // Should add bitwise(?), logical operators
		LT,
		LTEQ,
		EQ,
		GTEQ,
		GT,
		NEQ,
		First,
		Last
	};

	enum class LOGICAL_RELATION_JOIN {    // Other logical relations apply to to whole list, not between elements
		AND,
		OR,
		First,
		Last
	};

	bool init(string, RELATION, string);
	bool init(string, RELATION, int);

	string getExpression();
	static string join(std::list<string>, LOGICAL_RELATION_JOIN);
	static string joinColumns(std::list<DBCond>, string);
	static string joinValues(std::list<DBCond>, string);

	// Iterator to allow for iterating over lists of this type
	struct iterator
	{
	public:
		// Tags
		using iterator_category = std::forward_iterator_tag;    // forward iteration only
		using difference_type 	= std::ptrdiff_t;				// step size is the size of the pointer
		using value_type 		= DBCond;
		using pointer 			= DBCond*;
		using reference 		= DBCond&;

		iterator(pointer ptr) : _Ptr(ptr) {}

		// Referencing
		reference operator*() const { return *_Ptr; }
		pointer operator->() { return _Ptr; }

		// Edit iterator position
		iterator operator++() { _Ptr++; return *this; }    // post-increment
		iterator operator++(int) { iterator temp = *this; ++(*this); return temp; }    // pre-increment

		// Comparisons
		friend bool operator==(const iterator& a, const iterator& b) { return (a._Ptr == b._Ptr); }
		friend bool operator!=(const iterator& a, const iterator& b) { return (a._Ptr != b._Ptr); }

	private:
		pointer _Ptr;
	};

private:
	// List of relations between columns and values in the columns (Eg: WHERE col1=value)
	const string Rels[(int)DBCond::RELATION::Last - 1] = {"<", "<=", "=", ">=", ">", "<>"};

	// List of relations between conditions (Eg: WHERE cond1 AND cond2)
	static const string LogRels[(int)DBCond::LOGICAL_RELATION_JOIN::Last - 1]; // = {" AND ", " OR "};
	const string SpecialMySqlStrings[5] = {"CURRENT_DATE", "CURRENT_TIME", "DEFAULT", "test", "test2"};

	// Private members
	string Column;
	string Relation;
	string Value;
	string Expression;

	// Private methods
	string parse();
};

#endif
