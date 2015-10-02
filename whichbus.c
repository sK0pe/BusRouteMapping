/*
	CITS2002 Project 1 2015
	Name(s):            Pradyumn Vij, Elijah Fetzer
	Student number(s):  21469477, 21516694
	Date:               18-09-2015
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>

//  Arithmetic constants and problem definitions
#define PI acos(-1.0)
#define EARTH_RADIUS_M 6372797
#define WALK_SPEED 60.0
#define MAX_WAIT_MINS 60
#define MAX_WALK_METRES 1000
// File locations
#define STOPS "/stops.txt"
#define STOPTIMES "/stop_times.txt"
#define TRIPS "/trips.txt"
#define	ROUTES "/routes.txt"

//  Global Declarations
char*   FOLDER;

//  Struct Definitions
/*
 *	Stop struct
 *	Holds pertinent data for Stop information
 */
typedef struct {
	int id;
	char name[50];
	double distance;
} Stop;


/*
 *	valid_location
 *	boolean
 *	Sanity check function for coordinates
 *	returns true iff coordinates are correct
 *	else returns false
 */
static bool valid_location(double latitude, double longitude){
	return ((latitude  >= -90.0  && latitude  <= 90.0) &&
			(longitude >= -180.0 && longitude <= 180.0));
}


/*
 *	degrees_to_radians
 *	double
 * 	Helper function for haversine function
 * 	Converts degrees to radians in unit type
 */
static double degrees_to_radians(double degrees){
	return degrees * PI/180;
}


/*
 *	haversine
 *	double
 *	Uses Haversine formula,
 *	see https://en.wikipedia.org/wiki/Haversine_formula
 *	Calculates the distance in metres between
 *	2 coordinates
 */
static double haversine(double lat1, double lon1, double lat2, double lon2){
	double deltaLat = sin(degrees_to_radians(lat2 - lat1))/2.0;
	double deltaLon = sin(degrees_to_radians(lon2 - lon1))/2.0;
	lat1 = degrees_to_radians(lat1);
	lat2 = degrees_to_radians(lat2);
	
	return (2.0 * EARTH_RADIUS_M * 
			asin(
					sqrt(
						pow(deltaLat,2) + cos(lat1) * cos(lat2) * pow(deltaLon,2))));
}


/* 
 * 	get_time
 * 	int
 * 	returns int representing minute of day based on external parameter
 * 	indicating 24 hour time format, extracted from a string.
 */
static int get_time(char timeString[]){
	int hours = (timeString[0] - '0')*10 + (timeString[1] - '0');
	int minutes = (timeString[3] - '0')*10 + (timeString[4] - '0');
	return (hours * 60 + minutes) % 1440;
}


/*
 *	tokenizer
 *	char *
 *	Custom string tokenizer function that can manage empty spaces
 *	between delimiters.
 *	Also allows delimiters to be ignored if they are within double
 *	quotes.
 *	Outputs each successive token from string including
 *	empty positions when source is passed as NULL.
 */
static char *tokenizer(char *source, const char *delimiter){
	static char *cursor = NULL;
	char *tokenStart;
	//  First time, point to start of source
	if(source != NULL){
		cursor = source;
	}
	//  check if end of line
	if(cursor == NULL || *cursor == '\0'){
		return NULL;
	}
	//  save cursor pointer
	tokenStart = cursor;
	//  number of characters up to the next delimiter
	//  from cursor's position, not including those between
	//  speechmarks, while moving cursor forward the token
	//  length
	int n = 0;
	bool quote = false;
	while(*cursor != '\0'){
		if(*cursor == delimiter[0] && !quote){
			break;
		}
		if(*cursor == '"'){
			quote = !quote;
		}	
		n++;
		cursor++;
	}
	//  if cursor is not pointing to a nullbyte
	//  overwrite the delimiter with nullbyte and move forward
	if(*cursor != '\0'){
		*cursor++ = '\0';
	}
	//  return from cursor's position to new nullbyte
	return tokenStart;
}

 
/*
 *	load_file
 *	File *
 *	Takes NULL FILE * and char *
 *	To make a generic file loader with included error check.
 */
static FILE* load_file(FILE *stream, char*fileToLoad){
	char path[MAXPATHLEN];
	strcpy(path, FOLDER);
	strcat(path, fileToLoad);
	stream = fopen(path, "r");
	if(stream == NULL){
		fprintf(stderr, "Error opening file: %s\n", path);
		exit(EXIT_FAILURE);
	}
	return stream;
}


/*
 *	get_stop_arraysize
 *	int *
 *	Returns a pointer to integer array indicating the number of 
 *	valid stops within walking distance for origin and destination.
 */
static int *get_stop_arraysize(double origLat, double origLon, double destLat, double destLon){
	FILE *stopData = NULL;
	stopData = load_file(stopData,STOPS);
	char line[BUFSIZ];        // current line buffer
	bool first = true;        // flag to skip first line
	static int arraySizes[2];
	int originStops = 0;      // number of stops close enough to origin
	int destinationStops = 0; // number of stops close enough to destination
	double stop_lat;
	double stop_lon;

	//  Read through text file looking for valid stops
	while(fgets(line, sizeof line, stopData) != NULL){
		//  Skip first line
		if(first){
			first = false;
			continue;
		}
		
		int fieldNum = 0;
		//  find stop coordinates
		char *field = tokenizer(line, ",");
		while(field != NULL){
			//  field 6 has latitude
			if(fieldNum == 6 ){
				stop_lat = atof(field);
				//  next entry is guaranteed to be longitude
				stop_lon = atof(tokenizer(NULL, ","));
				break;
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
		//  If close enough to origin, increment number of stops
		if(haversine(origLat, origLon, stop_lat, stop_lon) <= MAX_WALK_METRES){
			originStops++;
		}
		//  If close enough to destination, increment number of stops
		if(haversine(destLat, destLon, stop_lat, stop_lon) <= MAX_WALK_METRES){
			destinationStops++;
		}
	}
	arraySizes[0] = originStops;
	arraySizes[1] = destinationStops;
	//  Close file stream access	
	if(stopData != NULL){
		fclose(stopData);
	}
	return arraySizes;
}


/*
 * 	populate_stop_arrays
 * 	void
 * 	Helper function for find_valid_stops
 * 	Populates the Origin and Destination Stop
 * 	Arrays with information from STOPS file
 */
static void populate_stop_arrays(Stop *originStopsArr, Stop *destStopsArr, 
		double origLat, double origLon, double destLat, double destLon){
	
	FILE *stopData = NULL;
	stopData = load_file(stopData,STOPS);
	char line[BUFSIZ];  // current line buffer
	bool first = true;  // flag to skip first line
	int stop_id;
	char *stop_name;
	double origin_stop_cost;
	double dest_stop_cost;
	double stop_lat;
	double stop_lon;
	int origin_stop_counter = 0;
	int dest_stop_counter = 0;
	int fieldNum;      // field or token counter
	char *field;       // pointer to recieve tokens
	
	//  Read through text file looking for valid stops
	while(fgets(line, sizeof line, stopData) != NULL){
		//  Skip first line
		if(first){
			first = false;
			continue;
		}
		fieldNum = 0;
		//  find stop coordinates
		field = tokenizer(line, ",");
		while(field != NULL){
			//  field 2 has ID
			if(fieldNum == 2){
					stop_id = atoi(field);
			}
			//  field 4 has stop name
			if(fieldNum == 4){
				stop_name = field;
			}
			//  field 6 has latitude
			if(fieldNum == 6){
				stop_lat = atof(field);
				//  If field 6 is known then 7 is also known (longitude)
				stop_lon = atof(tokenizer(NULL, ","));
				origin_stop_cost = haversine(origLat, origLon, stop_lat, stop_lon);
				dest_stop_cost = haversine(stop_lat, stop_lon, destLat, destLon);
				if((origin_stop_cost <= MAX_WALK_METRES) && (origin_stop_cost < dest_stop_cost)){
					//  Enter id
					originStopsArr[origin_stop_counter].id = stop_id;
					//  Enter name of stop
					strcpy(originStopsArr[origin_stop_counter].name, stop_name);
					//  Enter time cost
					originStopsArr[origin_stop_counter].distance = origin_stop_cost;
					origin_stop_counter++;
					break;
				}
				if((dest_stop_cost <= MAX_WALK_METRES) && (dest_stop_cost < origin_stop_cost)){
					//  Enter id
					destStopsArr[dest_stop_counter].id = stop_id;
					//  Enter name of stop
					strcpy(destStopsArr[dest_stop_counter].name, stop_name);
					//  Enter time cost
					destStopsArr[dest_stop_counter].distance = dest_stop_cost;
					dest_stop_counter++;
					break;
				}
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
	}
	if(stopData != NULL){
		fclose(stopData);
	}
}


/*
 *	find_optimal_trip
 *	void
 *	Helper function for find_valid_stops
 *	Opens and cycles through STOPTIMES looking for trips
 *	that will connect stops within originStopsArr and 
 *	destStopsArr, then find the minimum arrival time at
 *	the destination.
 *	Returns the array indicies and values regarding the optimal
 *	trip.
 */
static void find_optimal_trip(Stop *originStopsArr, int originStopNumber, 
		Stop *destStopsArr, int destStopNumber, int currentTime, int *optimal_stops){
	
	FILE *stopTimeData = NULL;
	stopTimeData = load_file(stopTimeData, STOPTIMES);
	char line[BUFSIZ];	// current line buffer
	bool first = true;	// flag to skip first line
	//  current value variables
	int trip_id;
	int arrivalTime;
	int departureTime;
	int stop_id;
	//  optimal value variables
	int min_timeCost = -1;
	//  Checking for arrival or departure, start with looking for departure
	bool validDepartureFound = false;
	int validDepartureTime;
	int validTrip_id;
	int validIndex;
	//  Boolean marker to check whether to skip processing of line due to
	//  logically invalid possibilities
	bool skipLine = false;
	//  Loop Variables
	int fieldNum;		// field or token counter
	char *field;		// pointer to recieve tokens

	while(fgets(line, sizeof line, stopTimeData) != NULL){
		//  Skip first line
		if(first){
			first = false;
			continue;
		}
		fieldNum = 0;
		//  find valid legs
		field = tokenizer(line,",");
		while(field != NULL){
			//  field 0 has trip_id
			if(fieldNum == 0){
				trip_id = atoi(field);
				if(validDepartureFound && trip_id != validTrip_id){
					validDepartureFound = false;
				}
			}
			//  field 1 has arrival time
			if(fieldNum == 1){
				arrivalTime = get_time(field);
				// Can't arrive before current time
				if(arrivalTime < currentTime){
					skipLine = true;
					break;
				}
			}
			//  field 2 has departure time
			if(fieldNum == 2){
				departureTime = get_time(field);
				// departures can't be more than an hour after leaving home
				if(!validDepartureFound && (departureTime < currentTime || departureTime > currentTime + MAX_WAIT_MINS)){
					skipLine = true;
					break;
				}
			}
			//  field 3 has stop_id
			if(fieldNum == 3){
				stop_id = atoi(field);
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
		//  Skip processing line as a field was invalid
		if(skipLine){
			skipLine = false;
			continue;
		}
		
		//  Checking for valid link between stops
		//  validTrip_id may be a double check from field 0 check above
		if(validDepartureFound && arrivalTime > validDepartureTime){
			for(int j = 0; j < destStopNumber; j++){
			//  if valid origin to destination link found 
				if(stop_id == destStopsArr[j].id){
					int validTime = arrivalTime + (int)ceil(destStopsArr[j].distance / WALK_SPEED);
					//  Check for less than 0 min_timeCost meaning no links are yet found
					if(min_timeCost > validTime || min_timeCost < 0){
							min_timeCost = validTime;
							//  returning array of valid indices to construct answer from
							optimal_stops[0] = validIndex;
							optimal_stops[1] = j;
							optimal_stops[2] = validDepartureTime;
							optimal_stops[3] = arrivalTime;
							optimal_stops[4] = validTrip_id;
					}
				}
			}
		}

		//  Check for valid origin
		for(int i = 0; i < originStopNumber; i++){
			//  Check if stop is a valid origin
			if(stop_id == originStopsArr[i].id){
				//  Check if valid departure (i.e. walk to stop in time)
				int origin_timecost = (int)(originStopsArr[i].distance/WALK_SPEED);
				if(currentTime + origin_timecost < departureTime){
					validDepartureFound = true;
					//  If true, record departure details
					validDepartureTime = departureTime;
					validTrip_id = trip_id;
					validIndex = i;
				}
			}
		}
	}
	if(stopTimeData != NULL){
		fclose(stopTimeData);
	}
}


/*
 *	get_route_id
 *	int
 *	Takes trip_id as input, matches it in 
 *	TRIPS file to its corresponding route_id and
 *	returns it as an integer.
 */
static int get_route_id(int trip){
	//  Open and search TRIPS file
	FILE *tripData = NULL;
	tripData = load_file(tripData, TRIPS);
	char line[BUFSIZ];	// current line buffer
	bool first = true;	// flag to skip first line
	int routeID;
	bool tripFound = false;
	//  Loop through text file
	int fieldNum; 		// field or token counter
	char *field;		// pointer to recieve tokens

	while(fgets(line, sizeof line, tripData) != NULL){
		//  Skip First Line
		if(first){
			first = false;
			continue;
		}
		fieldNum = 0;
		field = tokenizer(line, ",");
		//  Loop through tokens produced
		while(field != NULL){
			//  field 0 has route_id
			if(fieldNum == 0){
				routeID = atoi(field);
			}
			//  field 2 has trip_id
			if(fieldNum == 2){
				if(atoi(field) == trip){
					tripFound = true;
					break;
				}
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
		if(tripFound){
			break;
		}
	}
	if(tripData != NULL){
		fclose(tripData);
	}
	return routeID;
}


/*
 *	get_route_name
 *	void
 *	Populates a character array with the route name of
 *	the route_id provided, reading the ROUTES file.
 */
static void get_route_name(int routeID, char* returnName){
	// Open and search ROUTES file
	FILE *routeData = NULL;
	routeData = load_file(routeData, ROUTES);
	char line[BUFSIZ];	// current line buffer
	bool first = true;	// flag to skip first line
	int transportType;
	char shortName[4];
	char longName[40];
	bool skipLine = false;
	bool found = false;
	int fieldNum;		// field or token counter
	char *field;		// pointer to recieve tokens
	
	while(fgets(line, sizeof line, routeData) != NULL){
		//  Skip First Line
		if(first){
			first = false;
			continue;
		}
		fieldNum = 0;
		field = tokenizer(line, ",");
		//  Loop through tokens produced
		while(field != NULL){
			//  field 0 has route_id
			if(fieldNum == 0){
				//  Only process routeID this way
				if(routeID != atoi(field)){
					skipLine = true;
					break;
				}
				else{
					found = true;
				}
			}
			//  field 2 has route_short_name
			if(fieldNum == 2){
				strcpy(shortName, field);
			}
			//  field 3 has route_long_name
			if(fieldNum == 3){
				if (strlen(field) > 0){
					strcpy(longName, "\"");
					strcat(longName, field);
					strcat(longName, "\"");
				}
				else{
					strcpy(longName, field);
				}
			}
			//  field 5 has transport type
			if(fieldNum == 5){
				transportType = atoi(field);
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
		//  Skip line processing if indicated
		if(skipLine){
			skipLine = false;
			continue;
		}
		//  Found route ID, don't need to process any more lines
		if(found){
			break;
		}
	}
	if(routeData != NULL){
		fclose(routeData);
	}
	//  Construct route name description
	switch(transportType){
		case 2 :
			strcpy(returnName, "rail  ");
			strcat(returnName, longName);
			break;
		case 3 :
			strcpy(returnName, "bus  ");
			strcat(returnName, shortName);
			strcat(returnName, longName);
			break;
		case 4 :
			strcpy(returnName, "ferry  ");
			strcat(returnName, longName);
			break;
	}
}


/*
 *	print_results
 *	void
 *	Prints to standard output the shortest found single segment journey
 */
void print_results(Stop *originStopsArr, Stop *destStopsArr, int *min_trip, double origLat,
              double origLon, double destLat, double destLon, int currentTime){

	int min_OriginIndex = min_trip[0];
	int min_DestIndex = min_trip[1];
	//  Get route ID
	int route_id = get_route_id(min_trip[4]);
	//  Get route name
	char route_name[30];
	get_route_name(route_id, route_name);

	//  Arrival Time
	int arrival = min_trip[3] + (int)ceil(destStopsArr[min_DestIndex].distance/WALK_SPEED);
	//  Calculate walking time and distance to see if walking is faster than taking a trip
	double walking_distance = haversine(origLat, origLon, destLat, destLon);
	int walking_time = (int)ceil(walking_distance/WALK_SPEED);
	int walk_arrival = currentTime + walking_time;
	
	if(min_trip[0] == -1){
		//  If no appropriate journey found
		printf("%02d:%02d  not  possible\n", currentTime/60, currentTime%60);
	}
	else if(walking_distance < MAX_WALK_METRES && walk_arrival < arrival){
		//  If walking is faster than taking public transport
		printf("%02d:%02d  walk  %dm  to  destination\n%02d:%02d  arrive\n",
		    currentTime/60, currentTime%60, (int)walking_distance,
				walk_arrival/60, walk_arrival%60);
	}
	else{
		//  If taking public transport
		printf("%02d:%02d  walk  %dm  to  stop  %d  %s\n%02d:%02d  catch  %s  to  stop  %d  %s\n%02d:%02d  walk  %dm  to  destination\n%02d:%02d  arrive\n",
                currentTime/60, currentTime%60,
					(int)originStopsArr[min_OriginIndex].distance,
						originStopsArr[min_OriginIndex].id,
							originStopsArr[min_OriginIndex].name,
								min_trip[2]/60, min_trip[2]%60,
									route_name,
										destStopsArr[min_DestIndex].id,
											destStopsArr[min_DestIndex].name,
												min_trip[3]/60, min_trip[3]%60,
													(int)destStopsArr[min_DestIndex].distance,
														arrival/60, arrival%60);
	}
}


/*
 *	find_valid_stops
 *	void
 *	Fills origin and destination arrays of type Stop with STOP file
 *	information. Uses this information to find optimal trip and then
 *	prints the result.
 */
static void find_valid_stops(double origLat, double origLon, double destLat, double destLon, int currentTime){
	//  Find number of valid stops at origin and destination
	int *size = get_stop_arraysize(origLat, origLon, destLat, destLon);
	int originStopNumber = size[0];
	int destStopNumber = size[1];
	//  Build array of stops of appropriate size
	Stop originStopsArr[ originStopNumber];
	Stop destStopsArr[ destStopNumber ];
	//  Populate Stop Arrays
	populate_stop_arrays(originStopsArr, destStopsArr, origLat, origLon, destLat, destLon);
	//  Find Optimal Trip
	int min_trip[5];
	//  Initialize array with -1 values so impossible solutions can be found
	for(int i = 0; i < 5; ++i){
		min_trip[i] = -1;
	}
	find_optimal_trip(originStopsArr, originStopNumber, destStopsArr, destStopNumber, currentTime, min_trip);

	print_results(originStopsArr, destStopsArr, min_trip, origLat, origLon, destLat, destLon, currentTime);
}


/*
 *	Main Function
 *	Input argument validity and passing variables as required
 */
int main(int argc, char *argv[]){
	if (argc != 6){
		fprintf(stderr,"%s Usage: <directory> <Origin Latitude> <Origin Longitude>\n\
		<Destination Latitude> <Destination Longitude>\n\
		Coordinates must be entered as positive or negative numbers.\n",argv[0]);	
		exit(EXIT_FAILURE);
	}
	else{
		double origin_Lat = atof(argv[2]);
		double origin_Lon = atof(argv[3]);
		double dest_Lat = atof(argv[4]);
		double dest_Lon = atof(argv[5]);
		//  Check if Origin coordinates correct
		if (!valid_location(origin_Lat,origin_Lon)){
			fprintf(stderr,"%s Error: Origin Coordinates invalid!\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		//  Check if Destination coordinates correct
		if (!valid_location(dest_Lat, dest_Lon)){
			fprintf(stderr,"%s Error: Destination Coordinates invalid!\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		char *env = getenv("LEAVEHOME");
		//  Check if LEAVEHOME environment variable is set
		if (env == NULL){
			fprintf(stderr,"%s Error: LEAVEHOME environment variable not set!\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		//  Source time from LEAVEHOME environment variable
		char cur_time[6];
		int start_time = get_time(strncpy(cur_time, env+4, 5));
		//  Check if LEAVEHOME time is valid
		if (start_time > 1439 || start_time < 0){
			fprintf(stderr,"%s Error: LEAVEHOME environment variable time invalid!\n", argv[0]);
			exit(EXIT_FAILURE);			
		} 
		//  Define global variable for FOLDER
		FOLDER = argv[1];
	
		find_valid_stops(origin_Lat, origin_Lon, dest_Lat, dest_Lon, start_time);
		
		exit(EXIT_SUCCESS);
	}
	return 0;
}
