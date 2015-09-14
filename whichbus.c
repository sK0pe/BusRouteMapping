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
#define STOPTIMES	"/stop_times.txt"
//	Global Variables
int DAY;
int TIME;
char* FOLDER;

//	Struct Definitions

/*	
 *	Route struct
 *	Holds pertinent data for Route information
 */
typedef struct{
	int n_stops;
	char routeNumber[4];
	char routeName[50];
	char transport[50];
	Stop stops[];
	int arrivalTime[];
	int departureTime[];
} Trip;

/*
 * Stop struct
 * Holds pertinent data for Stop information
 */
typedef struct{
	int id;
	int name[100];
	double latitude;
	double longitude;
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
 * <unused due to context of project>
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
		printf("Cannot open path: %s\n", path);
		exit(EXIT_FAILURE);
	}
	return stream;
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
 * 	find_route()
 */
void find_route(double origin_Lat, double origin_Lon, double dest_Lat, double dest_Lon, int time){
	loadStops();
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
		//	Get Current time
		//	Addressing time in minutes allows for easy interpretation as a cost
		int minute_of_day = getTime(); // needs to be extracted from LEAVEHOME
		
		find_route(origin_Lat, origin_Lon, dest_Lat, dest_Lon, minute_of_day);	
	}
	return 0;
}
