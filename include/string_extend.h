/*
 * string_extend.h
 *
 * Contains some useful function prototypes that aren't actually string class extensions
 * It would be nice to make it as a class extension at some point.
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 


#ifndef _STRING_EX_
#define _STRING_EX_

#include <string>
#include <list>

using namespace std;

class string_extend
{
public:
	static string join(std::list<string>, string);

private:
	string_extend() {};    // disallow instantiation
};

#endif
