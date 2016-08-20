# Public Transport route finder
Full Instructions at http://teaching.csse.uwa.edu.au/units/CITS2002/projects/project1.php
Perth's Public Transport Authority (PTA) provides public access to its scheduled times, stop locations, and route information from its webpage www.transperth.wa.gov.au/About/Spatial-Data-Access. The data is released as a collection of inter-related textfiles following the Google Transit Feed Specification (GTFS), which is also used by many other public transport companies, worldwide.
You will need to download your own copy of the data (about 90MB) by clicking on the first link "By downloading the data you are agreeing to the terms of the License..." 
The currently available data is valid until November 25th (after the project's due date).
The task

The project requires you to develop a C99 program, named whichbus, which uses GTFS-format data to find the shortest travel time between two provided locations. Your program should check for and accept 5 command-line arguments:
the name of a directory containing a set of GTFS files, and
a starting and an ending location (as pairs of latitude and longitude coordinates, typically home and work),
and produce plain-text output describing the "best" public transport route that should be taken to travel between the two locations. The starting time of the journey should be taken from the environment variable named LEAVEHOME. This enables you to develop and test your program by holding the day and time constant. Note that the day is required, as not all bus services run every day. The example, below, shows how you can set this environment variable to the current day, hour and minute, and then export it to your program.
The definition of the "best" route is the one requiring the minimum total journey time, consisting of segments:
walking from the starting point to the first bus, train, or ferry stop;
travel time on a bus, train, or ferry;
(if required) walking time to transfer to a connection;
(if required) waiting time for a connection;
(if required) travel time on a second bus, train, or ferry; and
time walking from the final stop to the required destination.
Your program should allow the traveller sufficient time to walk from their starting location (typically their home) to, say, the first bus-stop. Amazingly, people can walk in straight lines at a constant speed of one metre-per-second, between any two points (through buildings!), and buses are always on time! No walking segment should be longer than 1000m.
The journey will include at-most one transfer between two forms of transport (any combination of bus, train, or ferry).
Program requirements

Projects will be marked using an automatic marking program (for correctness) and by visual inspection (for good programming practices). It is thus important that your program produces its output in the correct format. Only lines commencing with digits (for times) will be considered valid output during the testing - thus, you can leave your "last minute" debugging output there if you wish.
Each line of output consists of a number of fields, which may be separated by one-or-more space or tab characters. The names of bus, train, and ferry stops should be enclosed within double-quotation characters, as the names include spaces. Times should be specified as HH:MM using a 24-hour clock, and no journey (in testing) will span midnight. Distances should be reported as an integer number of metres (truncated, no decimal component), such as 340m.
