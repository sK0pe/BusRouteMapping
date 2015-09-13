#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define	MAX_WALKING_DIST 1000
#define EARTH_RADIUS_M 6372797
#define PI (acos(-1.0))
//	Filenames
#define ROUTES		"/routes.txt"
#define STOPS			"/stops.txt"
#define TRIPS			"/trips.txt"
#define STOPTIMES	"/stop_times.txt"
//	Global Variables
int DAY;

//	Struct Definitions

/*	
 *	Route struct
 *	Holds pertinent data for Route information
 */
typedef struct{
	int id;
	char[4] routeNumber;
	char[50] routeName;
	int type;
} Route;

/*
 *	
 */
typedef struct{
	int id;
} Trip;




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
 * getTime
 * int
 * returns int representing minute of day based on system time
 * and assigns the global variable DAY an integer
 */
int getTime(){
	char hour[3], min[3], day[4];
	time_t currentTime;
	//	call current time in seconds from local system
	time(&currentTime);
	//	format to a tm struct details for local system
	struct tm *detail = localtime(&currentTime);
	//	extract hour, minute and day in string form
	//	could also have extracted date and day 
	strftime(hour, 3, "%H", detail);
	strftime(min, 3, "%M", detail);
	strftime(day, 4, "%a", detail);
	//	assign global DAY the appropiate day
	if(strcmp(day,"Mon")==0) DAY = 1;
	else if(strcmp(day,"Tue")==0) DAY = 2;
	else if(strcmp(day,"Wed")==0) DAY = 3;
	else if(strcmp(day,"Thu")==0) DAY = 4;
	else if(strcmp(day,"Fri")==0) DAY = 5;
	else if(strcmp(day,"Sat")==0) DAY = 6;
	else if(strcmp(day,"Sun")==0) DAY = 7;
	else printf("getTime : Invalid day sourced from System");
	

	//	convert time to int 
	int h = atoi(hour);
	int m = atoi(min);	
	return h*60 + m;
}

/*
 *	tokenizer
 *	*char
 *	Takes a char array pointer as the source string,
 *	and a string indicating the delimiter on which to
 *	split the string.
 *	Outputs each successive token from string including
 *	empty positions.
 */
char *tokenizer(char *source, const char *delimiter){
	static char *cursor = NULL;
	char *tokenStart;
	int n;
	//	First time, point to start of source
	if(source != NULL){
		cursor = source;
	}
	//	check if end of line
	if(cursor == NULL || *cursor == '\0'){
		return NULL;
	}

	// number of characters up to the next delimiter
	// from cursor's position
	n = strcspn(cursor, delimiter);
	// save cursor pointer
	tokenStart = cursor;
	// move cursor forward the token length
	cursor += n;
	// if cursor is not pointing to a nullbyte
	// overwrite the delimiter with nullbyte and move forward
	if(*cursor != '\0'){
		*cursor++ = '\0';
	}
	// return from cursor's position to new nullbyte
	return tokenStart;
}
 
/*
 *	getRouteName
 *	char*
 *	Takes integer representation of route name as input
 *	returns char* with route name,
 *	if no route name returns "bus"
 */
char *getRouteName(int route){
	FILE *routeData = fopen(ROUTES, "r");
	char line[BUFSIZ];
	char routeName[50];
	char id[4];
	bool first = true;

	while(fgets(line, sizeof line, routeData) != NULL){
		if(first){
			first = false;
			continue;
		}
		sscanf( line, "%[^','],%*[^','],%*[^','],%[^','],%*[^','],%*[^','],%*[^','],%*[^','],%*s", id, routeNumber,routeName);
		if(route == atoi(id)){
			break;
		}
	}
	if(strcmp(routeName, "")==0) return "bus";
	return routeName;
}

/*
 *	walk
 *	void
 *	Print function for walking
 *
 */
static void walk(int minutes, int distance, char destination[]){
	printf("%i:%i walk %im to %s", minutes/60, minutes%60, distance, destination);
}

/*
 * 	stop_to_destination
 */
void stop_to_destination(int minutes, double final_Walk){
	int distance = (int) ceil(final_Walk);
	//	print walk line
	walk(minutes, distance, "destination");
	//	print arrive line
	minutes += distance;
	printf("%i:%i arrive", minutes/60, minutes%60);
}

/*
 *	origin_to_stop
 *	Determine distance to destination and finds optimal public transport
 *	stop to walk to (if available).
 *	
 */
static void origin_to_stop(double lat1, double lon1, double lat2, double lon2){
	double distance = haversine(lat1, lon1, lat2, lon2);
	if(distance < MAX_WALKING_DIST){
		stop_to_destination(0, distance);
	}
}

/*
 *	stop_to_stop
 */


/*
 */



/*
 *	Main Function
 *	Deals with input argument validity and passing variables as required
 */
int main(int argc, char *argv[]){

	if (argc != 6){
		fprintf(stderr,"%s Usage: <directory>\n <Origin Latitude> <Origin Longitude>\n \
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
		
		//	Get Current time
		//	Addressing time in minutes allows for easy interpretation as a cost
		int minute_of_day = getTime();

		//	Do we need to open all the files all at once, or as we go?
		//	Depends on algorithm and data structure used.
		//	openFiles(argv[1]);
		



		//	Find optimal closest stop?	
		origin_to_stop(origin_Lat, origin_Lon, dest_Lat, dest_Lon);
	}
	return 0;
}
