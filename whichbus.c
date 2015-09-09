#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define  MAX_WALKING_DIST    1000
#define  EARTH_RADIUS_M      6372797

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
	lat1 = degrees_to_radians(lat1);
	lon1 = degrees_to_radians(lon1);
	lat2 = degrees_to_radians(lat2);
	lon2 = degrees_to_radians(lat2);
	
	return (2.0 * EARTH_RADIUS_M * 
			asin(
					sqrt(
						pow((sin((lat2 - lat1)/2.0)),2) 
							+ cos(lat1) * cos(lat2) * pow(sin((lon2 - lon1)/2.0),2))));
}

/*
 *	walk
 *	void
 *	Print function for walking
 *
 */
static void walk(int minutes, int distance, char[] destination){
	printf("%i:%i walk %im to %s", time/60, time%60, distance, destination);
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
		
		//	Do we need to open all the files all at once, or as we go?
		//	Depends on algorithm and data structure used.
		//	openFiles(argv[1]);
		
		//	Begin Journey	
		origin_to_stop(origin_Lat, origin_Lon, dest_Lat, dest_Lon);
	}
	return 0;
}
