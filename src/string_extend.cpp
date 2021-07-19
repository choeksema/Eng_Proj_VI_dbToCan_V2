/*
 * string_extend.cpp
 *
 * Contains string_extend class definitions
 *
 * Smoothbrains July 2021, Elevator Project
 * 		Caleb Hoeksema, Greg Huras, Andrew Sammut
 */ 

#include "string_extend.h"
#include <sstream>

using namespace std;


/*
 * Method to join all strings in the list by the supplied separator
 * 
 * Inputs:
 *      std::list<string> listIn: list of strings to join
 *      string sep: char/string to put between all strings
 *
 * Returns string: joined string list
 */
string string_extend::join(std::list<string> listIn, string sep)
{
    stringstream ss;
    for(list<string>::iterator it=listIn.begin(); it != listIn.end(); it++)
    {
        ss << *it;

        if (*it != listIn.back())
            ss << sep;
    }
    
    return ss.str();
}
