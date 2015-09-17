#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>

// File data restraints
#define  MAX_COLS_PER_LINE      	150
#define  MAX_FIELDS_PER_LINE   		10
#define  MAX_NAME_LEN     				50
#define  MAX_STOPS         				20000
#define  MAX_TRIPS        				40000
#define  MAX_ROUTE_LEN     				200
#define  MAX_ROUTE_ID      				4000
#define  MAX_STOP_ID       				100000
#define  MAX_TRIP_ID     	  			750000
//	Arithmetic constants and problem definitions
#define  PI                 		  (acos(-1.0))
#define  MINS_IN_DAY      			  (24 * 60)
#define  EARTH_RADIUS_M   				6372797
#define  WALK_SPEED       	     	60
#define  MAX_WAIT_MINS     				60
#define  MAX_WALKING_METRES 			1000
//	File locations
#define  STOPS 										"/stops.txt"
#define  STOPTIMES 								"/stop_times.txt"
#define  TRIPS 										"/trips.txt"
#define  ROUTES 									"/routes.txt"


//	Struct Definitions

/*
 * stop_t struct
 * Holds pertinent data for stop information
 */
typedef struct {
	int     stop_id;
	char    stop_name[MAX_NAME_LEN];
	double  stop_lat;
	double  stop_lon;
} stop_t;

/*
 * trip_t struct
 * Holds pertinent data for trip information and corresponding
 * route and stop data
 */
typedef struct {
	int   n_stops;                      // amount of stops in the trip
	char  name[MAX_NAME_LEN];           // name of corresonding route
	char  transport[5];                 // type of transport
	int   stops[MAX_ROUTE_LEN];         // list of stop ids
	int   arr_time[MAX_ROUTE_LEN];      // arrival times for each stop
	int   dep_time[MAX_ROUTE_LEN];      // departure times for each stop
} trip_t;

//	Global Variables
trip_t  trips[MAX_TRIPS];							// holds all the trips
stop_t  stops[MAX_STOPS];        			// holds all the stops


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
 * comma_idxs
 * int
 * retrieves the positions of all the comma delimeters in str, ingoring
 * delimeters within quote enclosed fields, and stores them in idxs
 * returns int representing the number of delimeter occurrences in str
 */
int comma_idxs(char str[], int idxs[]) {
	int n_del  = 0;
	int quotes = 0;
	int len    = strlen(str);
	
	for (int i = 0; i < len; i++) {
		if (str[i] == ',' && quotes == 0)
			idxs[n_del++] = i;
		if (str[i] == '"')
			quotes = 1 - quotes;
	}
	return n_del;
}


/*
 *	loadFile
 *	File *
 *	Takes NULL FILE * and char *
 *	To make a generic file loader with included error check
 */
FILE* loadFile(FILE *stream, char *folder, char*fileToLoad){
	char path[MAXPATHLEN];
	strcpy(path, folder);
	strcat(path, fileToLoad);
	stream = fopen(path, "r");
	if(stream == NULL){
		exit(EXIT_FAILURE);
	}
	return stream;
}


/*
 * load_routes
 * int 
 * loads the entire route data from the files, and stores the data
 * in their respective arrays
 * returns int representing the number of total routes
 */
int load_routes(char folder[]) {	
	FILE   *fp = NULL;
	
	char   line[MAX_COLS_PER_LINE];   // current line buffer

	int    idxs[MAX_FIELDS_PER_LINE]; // delimeter index positions in line
	int    stop_by_id[MAX_STOP_ID];   // for holding all the stop ids.
	int    n_stops  = 0;              // total number of stops
	int    n_routes = 0;              // total number of routes

	// temporary structure to hold route information
	struct {
		int  type;		            			// the type of route (bus/train/ferry/..)
		char name[MAX_NAME_LEN];  			// name of route
	} all_routes[MAX_ROUTE_ID];
	// temporary structure to hold trip information
	struct {
		int route_id; 	          		// route id corresponding to this trip
		int idx;                  		// index of current route in the output array
	} all_trips[MAX_TRIP_ID];
	

  // open the file containing the stops
	fp = loadFile(fp, folder, STOPS);

	while (fgets(line, MAX_COLS_PER_LINE, fp)) {
		int no_pos = comma_idxs(line, idxs);	
		for (int i = 0; i < no_pos; i++)
			line[idxs[i]] = '\0';
		
		// read data from current stop 	
		int id = atoi(line + idxs[1] + 1);  // get the stop id

		stop_by_id[id] = n_stops;	// update stop no 
		stops[n_stops].stop_id = id; // update the stop id
		strcpy(stops[n_stops].stop_name, line + idxs[3] + 1); // update the stop name
		stops[n_stops].stop_lat = strtod(line + idxs[5] + 1, NULL);   // update the stop latitude
		stops[n_stops].stop_lon = strtod(line + idxs[6] + 1, NULL);   // update the stop longitude
		
		n_stops++;
	}

  // open the file containing the routes
	fp = loadFile(fp, folder, ROUTES);
	
	while (fgets(line, MAX_COLS_PER_LINE, fp)) {
		int no_pos  = comma_idxs(line, idxs);
		for (int i = 0; i < no_pos; i++)
			line[idxs[i]] = '\0';
			
		// read data from current route
		int id = atoi(line); // get the route id
		
		all_routes[id].type = atoi(line + idxs[4] + 1);  // update the route transport type
		strcpy(all_routes[id].name, line + idxs[1] + 1); // update the route name
		if (strlen(line + idxs[2] + 1)) {
			strcat(all_routes[id].name, "\"");
			strcat(all_routes[id].name, line + idxs[2] + 1);
			strcat(all_routes[id].name, "\"");
		}
	}

	
  // open the file containing the trips
	fp = loadFile(fp, folder, TRIPS);

	while (fgets(line, MAX_COLS_PER_LINE, fp)) {
		int no_pos = comma_idxs(line, idxs);
		for (int i = 0; i < no_pos; i++)
			line[idxs[i]] = '\0';

		// read data from current trip
		int id = atoi(line + idxs[1] + 1);  // get the trip id
		
		all_trips[id].route_id = atoi(line); // update the route id of the route corresponding to the trip
		all_trips[id].idx = -1;             
	}

	memset(trips, '\0', sizeof(trip_t) * MAX_TRIPS);

  // open the file containing the stop times
	fp = loadFile(fp, folder, STOPTIMES);
	
  // add every stop to the trips array, first checking if the trip corres
	// first check if the trip corresponding to the stop is already in the array
	// if not, add trip to the array then add stop to the trip
	while (fgets(line, MAX_COLS_PER_LINE, fp)) {
		int no_pos = comma_idxs(line, idxs);
		for (int i = 0; i < no_pos; i++)
			line[idxs[i]] = '\0';
		
		// read data from current stop time
		int trip_id = atoi(line);	// get the trip id
		
		if (all_trips[trip_id].idx < 0) {
			all_trips[trip_id].idx = n_routes; 		// update the index of the trip corresponding to the route in the trips array
			strcpy(trips[n_routes].name, all_routes[all_trips[trip_id].route_id].name);   // update the route name
			
			// get the route transport method
			if (all_routes[all_trips[trip_id].route_id].type == 2)
				strcpy(trips[n_routes].transport, "rail");
			else if (all_routes[all_trips[trip_id].route_id].type == 3)
				strcpy(trips[n_routes].transport, "bus");
			else if (all_routes[all_trips[trip_id].route_id].type == 4)
				strcpy(trips[n_routes].transport, "ferry");
				
			n_routes++;
		}
		
		int stop_id  = atoi(line + idxs[2] + 1); // get the stop id
		int trip_idx = all_trips[trip_id].idx;   // get the current trip index

		trips[trip_idx].stops[trips[trip_idx].n_stops] = stop_by_id[stop_id];   // update the stop id
		trips[trip_idx].arr_time[trips[trip_idx].n_stops] = get_time(line + idxs[0] + 1);  // update the arrival time
		trips[trip_idx].dep_time[trips[trip_idx].n_stops] = get_time(line + idxs[1] + 1);  // update the departure time
		trips[trip_idx].n_stops++; 							   // update the number of associated stops
																				
	}
	//	close file stream access
	if (fp == NULL)
		fclose(fp);
	
	return n_routes;   // return total number of built routes
}


/*
 * find_route
 * void
 * Performs a shortest path search to find the optimal route, starting and end
 * stops for a single segment journey. Searches over entire trip data and finds
 * the shortest time to get to reach the destination using this trip.
 */
void find_best_route(int n_routes, int start_time, int min_trip[], double origLat, double origLon, double destLat, double destLon) {
	int     i_route = -1;	     // the current best route to take
	int     i_start = -1;      // current best starting stop
	int     i_end = -1;        // current best ending stop
	
	int     start;             // current starting stop
	int     finish;            // current finishing stop
	double  time;              // current elapsed journey time
	double  wait_time;         // current waiting time at a stop
	double  finish_time;       // current journey finishing time
	double  ans_time = 1e20;   // current shortest time taken for journey
	double  distance;            
		
	distance = haversine(origLat, origLon, destLat, destLon);
	
	if (distance < MAX_WALKING_METRES) {
		// total distance is short enough to walk straight to destination
		i_route = n_routes;
		ans_time = distance / WALK_SPEED;
	}
	for (int i = 0; i < n_routes; i++) {
		start = -1;
		finish = -1;
		finish_time = -1.0;
		// try to go using this route, getting into vehicle on the first available station 
		// find a station at which we can catch a bus/train/ferry
		for (int j = 0; j < trips[i].n_stops; j++) {
			distance = haversine(origLat, origLon, stops[trips[i].stops[j]].stop_lat, stops[trips[i].stops[j]].stop_lon);
			// check that we have to go no more than 1000m
			if (distance > MAX_WALKING_METRES)
				continue;
			wait_time = trips[i].dep_time[j] - (start_time + distance / WALK_SPEED);
			if (wait_time < 0.0)
				wait_time += MINS_IN_DAY;
			// check that the transport starts within 1 hr from leaving home
			if ((wait_time + distance / WALK_SPEED) <= MAX_WAIT_MINS) {
				start = j;
				break;
			}
		}
		if (start < 0)
			continue;
		// find a station at which we go out from transport
		for (int j = start + 1; j < trips[i].n_stops; j++) {
			distance = haversine(stops[trips[i].stops[j]].stop_lat, stops[trips[i].stops[j]].stop_lon, destLat, destLon);
			// check that we have to go no more than 1 km
			if (distance > MAX_WALKING_METRES)
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
			i_route = i;
			i_start = start;
			i_end   = finish; 
			ans_time = finish_time;
		}
	}
	// store optimal found route data
	min_trip[0] = i_route;
	min_trip[1] = i_start;
	min_trip[2] = i_end;
}


/*
 *	print_output
 *	void
 *  prints the shortest found single segment journey to standard output.
 */ 
void print_output(int n_routes, int start_time, int min_trip[], double origLat, double origLon, double destLat, double destLon) {
	
	int route     = min_trip[0];
	int startStop = min_trip[1];
	int endStop   = min_trip[2];
	
	double dist;
	double time;
	
	if (route == -1) {
		//	If no appropriate journey found
		printf("%02d:%02d  not  possible\n", start_time / 60, start_time  % 60);
	}
	else if (route == n_routes) { 
		// If walking is faster than taking public transport
		dist = haversine(origLat, origLon, destLat, destLon);
		printf("%02d:%02d  walk  %dm  to  destination\n", start_time / 60, start_time % 60, (int)dist);
		time = ceil(MINS_IN_DAY - start_time + dist / WALK_SPEED);
		if (time > MINS_IN_DAY)
			time -= MINS_IN_DAY;
		printf("%02d:%02d  arrive\n", (int) time / 60, (int) time % 60);
	} 
	else {
		// If taking public transport		
		dist = haversine(origLat, origLon, stops[trips[route].stops[startStop]].stop_lat, stops[trips[route].stops[startStop]].stop_lon);
		printf("%02d:%02d  walk  %dm  to  stop  %d  %s\n",
				start_time / 60,
				start_time % 60,
				(int)dist,
				stops[trips[route].stops[startStop]].stop_id,
				stops[trips[route].stops[startStop]].stop_name);
		printf("%02d:%02d  catch  %s  %s  to  stop  %d  %s\n",
				trips[route].dep_time[startStop] / 60,
				trips[route].dep_time[startStop] % 60,
				trips[route].transport,
				trips[route].name,
				stops[trips[route].stops[endStop]].stop_id,
				stops[trips[route].stops[endStop]].stop_name);
		dist = haversine(destLat, destLon, stops[trips[route].stops[endStop]].stop_lat, stops[trips[route].stops[endStop]].stop_lon);
		printf("%02d:%02d  walk  %dm  to  destination\n",
				trips[route].arr_time[endStop] / 60,
				trips[route].arr_time[endStop] % 60,
				(int)dist);
		time = ceil(trips[route].dep_time[endStop] + dist / WALK_SPEED);
		if (time > MINS_IN_DAY)
			time -= MINS_IN_DAY;
		printf("%02d:%02d  arrive\n", (int) time / 60, (int) time % 60);
	}
}


/*
 * find_valid_stops
 * void
 * loads entire relevant data from files, finds the optimal single-segment
 * journey, and prints the results to standard output.
 */
void find_valid_stops(char *argv[], double origLat, double origLon, double destLat, double destLon, int currentTime) {
	int n_routes;		  	// number of routes
	int min_trip[3];  	// for holding optimal route data
												
	n_routes = load_routes(argv[1]);
	find_best_route(n_routes, currentTime, min_trip, origLat, origLon, destLat, destLon);
	print_output(n_routes, currentTime, min_trip, origLat, origLon, destLat, destLon);	
}


/*
 *	Main Function
 *	Deals with input argument validity and passing variables as required
 */
int main(int argc, char *argv[]) {
	if (argc != 6){
		fprintf(stderr,"%s Usage: <directory> <Origin Latitude> <Origin Longitude>"
							" <Destination Latitude> <Destination Longitude>\nCoordinates "
							"must be entered as positive or negative numbers.\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	else{
		double origin_Lat  = strtod(argv[2], NULL);
		double origin_Lon  = strtod(argv[3], NULL);
		double dest_Lat = strtod(argv[4], NULL);
		double dest_Lon = strtod(argv[5], NULL);

		//	Check if Origin coordinates correct
		if (!valid_location(origin_Lat, origin_Lon)){
			fprintf(stderr,"%s Error: Origin Coordinates invalid!\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		//	Check if Destination coordinates correct
		if (!valid_location(dest_Lat, dest_Lon)){
			fprintf(stderr,"%s Error: Destination Coordinates invalid!\n", argv[0]);
			exit(EXIT_FAILURE);
		}	
		
		// Source time from environment variable LEAVEHOME
		char *env = getenv("LEAVEHOME");
		char cur_time[6];	
		int  start_time = get_time(strncpy(cur_time, env+4, 5));
		if(start_time > 1439 || start_time < 0){
			fprintf(stderr,"%s Error: Time: %s is invalid!\n", argv[0], cur_time);
		}
		
		// find the best route
		find_valid_stops(argv, origin_Lat, origin_Lon, dest_Lat, dest_Lon, start_time);		

		exit(EXIT_SUCCESS);
	}
	return 0;
}
