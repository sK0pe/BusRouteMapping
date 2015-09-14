#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define MAX_WAIT 60
#define	MAX_WALK 1000
#define EARTH_RADIUS_M 6372797
#define PI (acos(-1.0))
//	Filenames
#define ROUTES		"/routes.txt"
#define STOPS			"/stops.txt"
#define STOPTIMES	"/stop_times.txt"
#define TRIPS 		"/trips.txt"
//	Global Variables
int DAY;
int TIME;
char* FOLDER;

//	Struct Definitions


/*
 * Stop struct
 * Holds pertinent data for Stop information
 */
typedef struct{
	int stop_id;
	char stop_name[100];
	double stop_lat;
	double stop_lon;
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
 * 	Converts degrees to radians in type
 *
 */
static double degrees_to_radians(double degrees){
	return degrees*PI/180;
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
 * get_time
 * int
 * returns int representing minute of day based on external parameter
 * indicating time 24 hour format, extracted from a string.
 */
int get_time(char timeString[]){
	int hours = (timeString[0] - '0')*10 + (timeString[1] - '0');
	int minutes = (timeString[3] - '0')*10 + (timeString[4] - '0'); 
	return hours * 60 + minutes;
}

/*
 * comma_indices
 * in []
 * finds the positions of all the comma delimeters in source
 * and stores them in an integer array provided
 * returns: the number of delimeters in the source
 * ignores delimeters within quote enclosed fields 
 */
int comma_indices(char source[], int idxs[]) {
	int numIndices = 0;
	int length = strlen(source);
	bool quote = false;
	for (int i = 0; i < length; i++) {
		if (source[i] == ',' && !quote)
			idxs[numIndices++] = i;
		if (source[i] == '"')
			quote = !quote;
	}
	return numIndices;
}

/*
 *	tokenizer
 *	*char
 *	Takes a char array pointer as the source string,
 *	and a string indicating the delimiter on which to
 *	split the source.
 *	Outputs each successive token from string including
 *	empty positions.
 */
char *tokenizer(char *source, const char *delimiter){
	static char *cursor = NULL;
	char *tokenStart;
	//	First time, point to start of source
	if(source != NULL){
		cursor = source;
	}
	//	check if end of line
	if(cursor == NULL || *cursor == '\0'){
		return NULL;
	}

	// save cursor pointer
	tokenStart = cursor;

	// number of characters up to the next delimiter
	// from cursor's position, not including those between
	// speechmarks, while moving cursor forward the token
	// length
	int n=0;
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

	// if cursor is not pointing to a nullbyte
	// overwrite the delimiter with nullbyte and move forward
	if(*cursor != '\0'){
		*cursor++ = '\0';
	}
	// return from cursor's position to new nullbyte
	return tokenStart;
}
 


/*
 *	getNumberOfLines()
 *	int
 *	Counts newlines in a file
 */
int getNumberOfLines(FILE * stream){
	int character;
	int lineCount;

	while((character = fgetc(stream)) != EOF){
		if( character == '\n'){
			lineCount++;
		}
	}
	return lineCount;
}

/*
 *	loadFile()
 *	File *
 *	Takes NULL FILE * and char *
 *	To make a generic file loader with included error check
 */
FILE* loadFile(FILE *stream, char*fileToLoad){
	char path[200];
	strcpy(path, FOLDER);
	strcat(path, fileToLoad);
	printf("%s\n", path);
	stream = fopen(path, "r");
	if(stream == NULL){
		printf("\nError: Cannot open path: %s\n", path);
		exit(EXIT_FAILURE);
	}
	return stream;
}

/*
 * load_routes
 * int
 * load route data from files
 * returns: n_routes the total number of routes
 */
int load_routes(char folder[], Trip trips[]) {	
	int   idxs[MAX_FIELDS];    // delimeter index positions in line

	// load all stops - thinking of placing this into a function later
	int stop_by_id[MAX_STOP_ID];   // for holding all the stop ids.
	int n_stops = 0;               // total number of stops
  char line[BUFSIZ];	// Buffer for reading lines
 
	// open the file containing the stops
	FILE* stopData = NULL;
	stopData = loadFile(stopData, STOPS);
		//	Stop array, size determined by finding number of lines in file
	Stop all_stops[MAX_STOP_ID];  // for holding all the stops

	while (fgets(line, sizeof line, stopData) != NULL) {
		int commaCount = comma_indices(line, idxs);	
		for (int i = 0; i < commaCount; i++){
			line[idxs[i]] = '\0';	// write null byte over commas
		}
		
		// read data from current stop	
		int id = atoi(line + idxs[1] + 1);  // get the stop id

		stop_by_id[id] = n_stops;					// update stop no 
		all_stops[n_stops].stop_id = id; // update the stop id
		strcpy(all_stops[n_stops].stop_name, line + idxs[3] + 1); // update the stop name
		all_stops[n_stops].stop_lat = atof(line + idxs[5] + 1);   // update the stop latitude
		all_stops[n_stops].stop_lon = atof(line + idxs[6] + 1);   // update the stop longitude
		
		n_stops++;
	}
	// close stopData file stream
	if(stopData != NULL){
		fclose(stopData);
	}

// load all routes  - thinking of placing this into a function later
	struct {
		int  type;								// the type of route (bus/train/ferry/..)
		char name[100];  // name of route
	} all_routes[MAX_ROUTE_ID];
	
	// open the file containing the routes
	FILE* routeData = NULL;
	routeData = loadFile(routeData, ROUTES);
	
	while (fgets(line, sizeof line, routeData) != NULL) {
		int commaCount  = comma_indices(line, idxs);
		for (int i = 0; i < commaCount; i++){
			line[idxs[i]] = '\0';
		}
			
		// read data from current route
		int id = atoi(line); // get the route id
		
		all_routes[id].type = atoi(line + idxs[4] + 1);  // update the route transport type
		strcpy(all_routes[id].name, line + idxs[1] + 1); // update the route name
		strcat(all_routes[id].name, line + idxs[2] + 1);
	}
	//	close routeData filestream
	if(routeData != NULL){
		fclose(routeData);
	}

// load all the trips  - thinking of placing this into a function later
	// this structure 
	struct {
		int route_id; 		// route id corresponding to this trip
		int idx;          // index of current route in the output array
	} all_trips[MAX_TRIP_ID];
	
	// open the file containing the trips
	FILE *tripData = NULL;
	tripData = loadFile(tripData, TRIPS);
	
	while (fgets(line, sizeof line, tripData)) {
		int commaCount = comma_indices(line, idxs);
		for (int i = 0; i < commaCount; i++)
			line[idxs[i]] = '\0';

		// read data from current trip
		int id = atoi(line + idxs[1] + 1);  // get the trip id
		
		all_trips[id].route_id = atoi(line); // update the route id of the route corresponding to the trip
		all_trips[id].idx = -1;             
	}
	// close tripData file stream
	if(tripData != NULL){
		fclose(tripData);
	}

	// now build the route connections  - thinking of placing this into a function later
	memset(trips, '\0', sizeof(Trip) * MAX_ROUTES);

	// open the file containing the stop times
	FILE *stopTimeData = NULL;
	stopTimeData = loadFile(stopTimeData, STOPTIMES);

	int n_routes = 0;
	
 /* add every stop to the trips array one by one
	* first check if the trip corresponding to the stop is already in the array
	* if not, add trip to the array
	* then add stop to the trip
	*/
	while (fgets(line, MAX_STR_LEN, stopTimeData)) {
		int commaCount = comma_indices(line, idxs);
		for (int i = 0; i < commaCount; i++){
			line[idxs[i]] = '\0';
		}
		
		// read data from current stop time
		int trip_id = atoi(line);	// get the trip id
		
		if (all_trips[trip_id].idx < 0) {
			all_trips[trip_id].idx = n_routes; // update the index of the trip corresponding to the route in the trips array
			strcpy(trips[n_routes].name, all_routes[all_trips[trip_id].route_id].name);  // update the route name
			
			// get the route transport method
			if (all_routes[all_trips[trip_id].route_id].type == 2){
				strcpy(trips[n_routes].transport, "train");
			}
			else if (all_routes[all_trips[trip_id].route_id].type == 3){
				strcpy(trips[n_routes].transport, "bus");
			}
			else if (all_routes[all_trips[trip_id].route_id].type == 4){
				strcpy(trips[n_routes].transport, "ferry");
			}
				
			n_routes++;
		}
		
		int stop_id  = atoi(line + idxs[2] + 1); // get the stop id
		int trip_idx = all_trips[trip_id].idx;   // get the current trip index

		trips[trip_idx].stops[trips[trip_idx].n_stops] = all_stops[stop_by_id[stop_id]];   // update the stop id
		trips[trip_idx].arr_time[trips[trip_idx].n_stops] = get_time(line + idxs[0] + 1);  // update the arrival time
		trips[trip_idx].dep_time[trips[trip_idx].n_stops] = get_time(line + idxs[1] + 1);  // update the departure time
		trips[trip_idx].n_stops++; 																 // update the number of associated stops
	}
	return n_routes; // return total number of built routes
}

/*
 *	loadStops()
 *
 */
void loadStops(){
	FILE* stopData = NULL;
	stopData = loadFile(stopData, STOPS);
	bool first = true;
	char line[BUFSIZ];
	while(fgets(line, sizeof line, stopData) != NULL){
		if(first){
			first = false;
			continue;
		}
	}	
}

/*
 *	loadStopTimes()
 *
 */
void loadStopTimes(){
	FILE* stopTimeData = NULL;
	stopTimeData = loadFile(stopTimeData, STOPTIMES);
	bool first = true;
	char line[BUFSIZ];
	while(fgets(line, sizeof line, stopTimeData) != NULL){
		if(first){
			first = false;
			continue;
		}
	}	
}

/*
 *	loadRoutes()
 *
 */
void loadRoutes(){
	FILE* routeData = NULL;
	routeData = loadFile(routeData, ROUTES);
	bool first = true;
	char line[BUFSIZ];
	while(fgets(line, sizeof line, routeData) != NULL){
		if(first){
			first = false;
			continue;
		}
	}
}

/*
 * find_route
 * 
 * Performs search to find the optimal route using the data stored from above function
 * since we are allowed to use only one mean of transport (current case), then look over all the trips
 * and try getting to the destination location using this trip. Firstly, find the station which is closest
 * to the starting loc, then find the station which is closest to the finish loc and choose the route which
 * takes the minimum time, constantly updating our findings as we go
 *
 */
void find_route(Trip *trips, int n_routes, int start_time, double start_lat, double start_lon, double finish_lat, double finish_lon, int *i_route, int *i_start, int *i_end) {
	*i_route = -1;	 	// the current best route
	*i_start = -1;    // current best starting stop
	*i_end   = -1;    // current best ending stop
	
	int     start;               // current starting stop
	int     finish;              // current finishing stop
	double  time;                // current elapsed journey time
	double  wait_time;           // current waiting time at a stop
	double  finish_time;         // current journey finishing time
	double  ans_time = 1e20;     // current shortest route time
	double  distance;            
	
	distance = haversine(start_lat, start_lon, finish_lat, finish_lon);
	// walk to destination if close enough: NOTE: THIS may not be the optimal solution, it's possible there are route segments within 1000m that can get us to our destination earlier
	if (distance < MAX_WALK_DIST) {
		*i_route = n_routes;
		ans_time = distance / WALK_SPEED;
	}
	
	for (int i = 0; i < n_routes; i++) {
		start = -1;
		finish = -1;
		finish_time = -1.0;
		// try to go using this route, getting into vehicle on the first available station 
		// find a station at which we can catch a bus/train/ferry
		for (int j = 0; j < trips[i].n_stops; j++) {
			distance = haversine(start_lat, start_lon, trips[i].stops[j].stop_lat, trips[i].stops[j].stop_lon);
			// check that we have to go no more than 1000m
			if (distance > MAX_WALK_DIST)
				continue;
			wait_time = trips[i].dep_time[j] - (start_time + distance / WALK_SPEED);
			if (wait_time < 0.0)
				wait_time += MINS_IN_DAY;
			// check that the transport starts within 1 hr from leaving home
			if (wait_time + distance / WALK_SPEED <= MAX_WAIT) {
				start = j;
				break;
			}
		}
		if (start < 0)
			continue;
		// find a station at which we go out from transport
		for (int j = start + 1; j < trips[i].n_stops; j++) {
			distance = haversine(finish_lat, finish_lon, trips[i].stops[j].stop_lat, trips[i].stops[j].stop_lon);
			// check that we have to go no more than 1 km
			if (distance > MAX_WALK_DIST)
				continue;
			time = trips[i].arr_time[j] - start_time;
			if (time < 0)
				time += MINS_IN_DAY;
			time += distance / WALK_SPEED;
			if (finish < 0 || time < finish_time) {
				finish_time = time;
				finish = j;
			}
		}
		if (finish < 0)
			continue;
		// if we have better answer using this route, update it
		if (finish_time < ans_time) {
			*i_route = i;            // update best route
			*i_start = start;        // update starting stop
			*i_end   = finish;       // update ending stop
			ans_time = finish_time;  // update total time taken for journey (which must be under 3600)
		}
	}
}

/* 
 *
 * prints the result for a single segment journey
 *
 *
 * needs to be broken down into functions
 *
 */
void print_output(Trip * trips, int n_routes, int start_time, double start_lat, double start_lon, double finish_lat, double finish_lon, int i_route, int i_start, int i_end) {
	double dist;
	double time;
	
	if (i_route < 0) {
		printf("No appropriate route is found!\n");
	}
	else if (i_route == n_routes) // our journey was reachable in walking distance.
																// It is POSSIBLE that we could get to our destination within 1000m by still catching a transport segment. Can change for this later.
	{ 
		dist = haversine(start_lat, start_lon, finish_lat, finish_lon);
		printf("%02d:%02d  walk  %dm  to  destination\n", start_time / 60, start_time % 60, (int)dist);
		time = start_time + dist / WALK_SPEED;
		if (time > MINS_IN_DAY)
			time -= MINS_IN_DAY;
		printf("%02d:%02d  arrive\n", (int)time / 60, (int)time % 60);
	} 
	else {
		dist = haversine(start_lat, start_lon, trips[i_route].stops[i_start].stop_lat, trips[i_route].stops[i_start].stop_lon);
		printf("%02d:%02d  walk  %dm  to  stop  %d  %s\n",
				start_time / 60,
				start_time % 60,
				(int)dist,
				trips[i_route].stops[i_start].stop_id,
				trips[i_route].stops[i_start].stop_name);
		printf("%02d:%02d  catch  %s  %s  to  stop  %d  %s\n",
				trips[i_route].dep_time[i_start] / 60,
				trips[i_route].dep_time[i_start] % 60,
				trips[i_route].transport,
				trips[i_route].name,
				trips[i_route].stops[i_end].stop_id,
				trips[i_route].stops[i_end].stop_name);
		dist = haversine(finish_lat, finish_lon, trips[i_route].stops[i_end].stop_lat, trips[i_route].stops[i_end].stop_lon);
		printf("%02d:%02d  walk  %dm  to  destination\n",
				trips[i_route].arr_time[i_end] / 60,
				trips[i_route].arr_time[i_end] % 60,
				(int)dist);
		time = trips[i_route].dep_time[i_end] + dist / WALK_SPEED;
		if (time > MINS_IN_DAY)
			time -= MINS_IN_DAY;
		printf("%02d:%02d  arrive\n", (int)time / 60, (int)time % 60);
	}
}



/*
 *	Main Function
 *	Deals with input argument validity and passing variables as required
 */
int main(int argc, char *argv[]){

	if (argc != 6){
		fprintf(stderr,"%s Usage: <directory>\n <Origin Latitude> <Origin Longitude>\n\
				<Destination Latitude> <Destination Longitude>\n Coordinates \
				must be entered as positive or negative numbers.\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	else{
		// Using local variables but could place these directly into 
		// valid_location checks
		// Also may want to change to scanf instead of atof
		double origin_Lat = atof(argv[2]);
		double origin_Lon = atof(argv[3]);
		double dest_Lat = atof(argv[4]);
		double dest_Lon = atof(argv[5]);
		//	Check if Origin coordinates correct
		if (!valid_location(origin_Lat,origin_Lon)){
			fprintf(stderr,"%s Error: Origin Coordinates invalid!\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		//	Check if Destination coordinates correct
		if (!valid_location(dest_Lat, dest_Lon)){
			fprintf(stderr,"%s Error: Destination Coordinates invalid!\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		//	Define global variable for FOLDER
		FOLDER = argv[1];
		
		int n_routes;
		int start_time;
		int i_route;
		int i_start;
		int i_end;
		
		char cur_time[6];	
		start_time = get_time(strncpy(cur_time, getenv("LEAVEHOME")+4, 5)); // get LEAVEHOME time in minutes
		
		n_routes = load_routes(FOLDER, trips);
		find_route(trips, n_routes, start_time, origin_Lat, origin_Lon, dest_Lat, dest_Lon, &i_route, &i_start, &i_end);
		print_output(trips, n_routes, start_time, origin_Lat, origin_Lon, dest_Lat, dest_Lon, i_route, i_start, i_end);
	}
	return 0;
}
