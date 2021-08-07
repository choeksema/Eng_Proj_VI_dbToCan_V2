# Eng_Proj_VI_dbToCan_V2

Complete file reorganization compared to the last revision.
Includes a program to update the database as if it were the website (run make under /web_substitute and run the resulting 'web_sub'). This exists to allow for independent testing.

To move the elevator, run make under /db_can and run the resulting process ('db_can').

Running db_can with arg "demo" activates Sabbath mode (aka auto/demo mode). Sabbath mode also will run between 6PM Friday to 6PM Saturday each week. To run without Sabbath mode during this timeframe, use command line arg "override". 
(Not for normal usage): To turn on debugging, compile using 'make DEBUG="-D DEBUG".
