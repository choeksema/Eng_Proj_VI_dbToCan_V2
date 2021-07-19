# Eng_Proj_VI_dbToCan_V2

Complete file reorganization compared to the last revision
Includes a program to update the database as if it were the website
Currently the database reading in db_can doesn't work on the Pi server. It fails in main.cpp at "newEntries->next()". The database code works in web_sub (although it doesn't perform a select query

Run db_can to move the elevator. Includes a state machine that isn't very smart (needs some work but not sure on steps forward)
Run web_sub to update the database as if you were the website. this allows for completely independent testing.
