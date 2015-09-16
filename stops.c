#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define PI acos(-1.0)
#define EARTH_RADIUS_M 6372797
#define WALK_SPEED 60.0
#define MAX_WAIT 60
#define MAX_WALK 1000
#define FOLDER "transperth"
#define STOPS "/stops.txt"
#define STOPTIMES "/stop_times.txt"

typedef struct {
	int id;
	char name[50];
	double distance;
} Stop;

static double degrees_to_radians(double degrees){
	return degrees*PI/180;
}


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
	//int n = strcspn(cursor, delimiter);
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
	//cursor += n;

	// if cursor is not pointing to a nullbyte
	// overwrite the delimiter with nullbyte and move forward
	if(*cursor != '\0'){
		*cursor++ = '\0';
	}
	// return from cursor's position to new nullbyte
	return tokenStart;
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


FILE* loadFile(FILE *stream, char*fileToLoad){
	char path[200];
	strcpy(path, FOLDER);
	strcat(path, fileToLoad);
	stream = fopen(path, "r");
	if(stream == NULL){
		printf("\nError: Cannot open path: %s\n", path);
		exit(EXIT_FAILURE);
	}
	return stream;
}



/*
 *	get_stop_arraysize
 *	int *
 *	Returns a pointer to integer array indicating the number of 
 *	valid stops for origin and destination.
 *
 */
int *get_stop_arraysize(double origLat, double origLon, double destLat, double destLon){
	FILE *stopData = NULL;
	stopData = loadFile(stopData,STOPS);
	bool first = true;
	static int arraySizes[2];
	int originStops = 0;	//	Number of stops close enough to origin
	int destinationStops = 0;	// Number of stops close enough to destination
	char line[BUFSIZ];	//	Line buffer
	double stop_lat;
	double stop_lon;
	//	Read through text file looking for valid stops
	while(fgets(line, sizeof line, stopData) != NULL){
		//	Skip first line
		if(first){
			first = false;
			continue;
		}
		
		int fieldNum = 0;
		//	find stop coordinates
		char *field = tokenizer(line,",");
		while(field != NULL){
			//	field 6 has latitude
			if(fieldNum == 6 ){
				stop_lat = atof(field);
				// next entry is guaranteed to be longitude
				stop_lon = atof(tokenizer(NULL, ","));
				break;
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
		// If close enough to origin, add 1
		if(haversine(origLat, origLon, stop_lat, stop_lon) <= MAX_WALK){
			originStops++;
		}
		if(haversine(destLat, destLon, stop_lat, stop_lon) <= MAX_WALK){
			destinationStops++;
		}
	}
	arraySizes[0] = originStops;
	arraySizes[1] = destinationStops;
	//	Close file
	if(stopData != NULL){
		fclose(stopData);
	}
	return arraySizes;
}


/*
 * populate_stop_arrays
 * void
 * Helper function for find_valid_stops
 * Populates the Origin and Destination Stop
 * Arrays with information from STOPS file
 *
 */
void populate_stop_arrays(Stop *originStopsArr, Stop *destStopsArr, 
		double origLat, double origLon, double destLat, double destLon){
	
	FILE *stopData = NULL;
	stopData = loadFile(stopData,STOPS);
	bool first = true;	//	Boolean to skip first line
	char line[BUFSIZ];	//	Line buffer
	int stop_id;
	char *stop_name;
	double origin_stop_cost;
	double dest_stop_cost;
	double stop_lat;
	double stop_lon;
	int origin_stop_counter = 0;
	int dest_stop_counter = 0;
	int fieldNum;	//	field or token counter
	char *field;	//	pointer to recieve tokens
	//	Read through text file looking for valid stops
	while(fgets(line, sizeof line, stopData) != NULL){
		//	Skip first line
		if(first){
			first = false;
			continue;
		}
		fieldNum = 0;
		//	find stop coordinates
		field = tokenizer(line,",");
		while(field != NULL){
			//	field 2 has latitude
			if(fieldNum == 2){
					stop_id = atoi(field);
			}
			//	field 4 has stop name
			if(fieldNum == 4){
				stop_name = field;
			}
			//	field 6 has latitude
			if(fieldNum == 6){
				stop_lat = atof(field);
				//	If ield 6 is known than 7 is also known (longitude)
				stop_lon = atof(tokenizer(NULL, ","));
				origin_stop_cost = haversine(origLat, origLon, stop_lat, stop_lon);
				dest_stop_cost = haversine(destLat, destLon, stop_lat, stop_lon);
				if((origin_stop_cost <= MAX_WALK) && (origin_stop_cost < dest_stop_cost)){
					//	Enter id
					originStopsArr[origin_stop_counter].id = stop_id;
					//	Enter name of stop
					strcpy(originStopsArr[origin_stop_counter].name, stop_name);
					//	Enter time cost
					originStopsArr[origin_stop_counter].distance = origin_stop_cost;
					++origin_stop_counter;
					break;
				}
				if((dest_stop_cost <= MAX_WALK) && (dest_stop_cost < origin_stop_cost)){
					//	Enter id
					destStopsArr[dest_stop_counter].id = stop_id;
					//	Enter / copy over name
					strcpy(destStopsArr[dest_stop_counter].name, stop_name);
					//	Enter time cost
					destStopsArr[dest_stop_counter].distance = dest_stop_cost;
					++dest_stop_counter;
					break;
				}
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
	}
	//	close file stream access
	if(stopData != NULL){
		fclose(stopData);
	}
}


/*
 *	find_optimal_trip
 *	int *
 *	Helper function for find_valid_stops
 *	Opens and cycles through STOPTIMES looking for trips
 *	that will connect stops within originStopsArr and 
 *	destStopsArr, then find the minimum arrival time at
 *	the destination.
 *	Returns the array indicies and values regarding the optimal
 *	trip.
 *
 */
int *find_optimal_trip(Stop *originStopsArr, int originStopNumber, 
		Stop *destStopsArr, int destStopNumber, int currentTime, int *optimal_stops){
	
	FILE *stopTimeData = NULL;
	//	Open STOPTIMES file
	stopTimeData = loadFile(stopTimeData, STOPTIMES);
	bool first = true;	//	boolean to skip first line
	char line[BUFSIZ];	//	buffer for line
	//	current value variables
	int trip_id;
	int arrivalTime;
	int departureTime;
	int stop_id;
	//	optimal value variables
	int min_timeCost = -1;
	//	Checking for arrival or departure, start with looking for departure
	bool validDepartureFound = false;
	int validDepartureTime;
	int validTrip_id;
	int validIndex;
	//	Boolean marker to check whether to skip processing of line due
	//	to logically invalid possibilities
	bool skipLine = false;
	//	Loop Variables
	int fieldNum;
	char *field;
	while(fgets(line, sizeof line, stopTimeData) != NULL){
		//	Skip first line
		if(first){
			first = false;
			continue;
		}
		fieldNum = 0;
		//	find valid legs
		field = tokenizer(line,",");
		while(field != NULL){
			//	field 0 holds trip_id
			if(fieldNum == 0){
				trip_id = atoi(field);
				if(validDepartureFound && trip_id != validTrip_id){
					validDepartureFound = false;
				}
			}
			//	field 1 holds arrival time
			if(fieldNum == 1){
				arrivalTime = get_time(field);
				//	Can't arrive before current time
				if(arrivalTime < currentTime){
					skipLine = true;
					break;
				}
			}
			//	field 2 holds departure time
			if(fieldNum == 2){
				departureTime = get_time(field);
				//	departures can't be more than an hour after leaving home
				if(departureTime < currentTime || departureTime > currentTime + MAX_WAIT){
					skipLine = true;
					break;
				}
			}
			//	field 3 holds stop_id
			if(fieldNum == 3){
				stop_id = atoi(field);
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
		//	Skip processing line as a field was invalid
		if(skipLine){
			skipLine = false;
			continue;
		}

		//	Checking for valid link between stops
		//	validTrip_id may be a double check from field 0 check above
		if(validDepartureFound && arrivalTime > validDepartureTime){
			for(int j = 0; j < destStopNumber; j++){
			//	if valid origin to destination link found 
				if(stop_id == destStopsArr[j].id){
					//	The total cost of walking to the stop + wait + travel time + walking to destination is the following expression 
					//	validWalkTime1 + (validDepartureTime - validWalkTime1) + arrivalTime - validDepartureTime + (int)ceil(destStopsArr[j].distance/WALK_SPEED);
					//	However this can be simplified to arrival time + time to walk to destination
					int validTime = arrivalTime + (int)ceil(destStopsArr[j].distance/WALK_SPEED);
					//	Check for less than 0 min_timeCost meaning no links are yet found
					if(min_timeCost > validTime || min_timeCost < 0){
							min_timeCost = validTime;
							//	returning array of valid indices to construct answer from
							optimal_stops[0] = validIndex;
							optimal_stops[1] = j;
							optimal_stops[2] = validDepartureTime;
							optimal_stops[3] = arrivalTime;
							optimal_stops[4] = validTrip_id;
					}
				}
			}
		}
		
		//	Check for valid origin
		for(int i = 0; i < originStopNumber; i++){
			//	Check if stop is a valid origin
			if(stop_id == originStopsArr[i].id){
				//																				printf("anything here");
				//	Check if valid departure (i.e. walk to stop in time)
				int origin_timecost = (int)ceil(originStopsArr[i].distance/WALK_SPEED);
				if(currentTime + origin_timecost < departureTime){
					validDepartureFound = true;
					//	If true, record departure details
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
	return optimal_stops;
}


/*
 *	find_valid_stops
 *
 *	Fills origin and destination arrays of type Stop with STOP file
 *	information.
 */
 void find_valid_stops(double origLat, double origLon, double destLat, double destLon, int currentTime){
	//	Find number of valid stops at origin and destination
	int *size = get_stop_arraysize(origLat, origLon, destLat, destLon);
	int originStopNumber = size[0];
	int destStopNumber = size[1];
	//	Make array of stops of appropriate size
	Stop originStopsArr[ originStopNumber ];
	Stop destStopsArr[ destStopNumber ];
	//	Populate Stop Arrays
	populate_stop_arrays(originStopsArr, destStopsArr, origLat, origLon, destLat, destLon);
	//	Find Optimal Trip
	int min_trip[5];
	find_optimal_trip(originStopsArr,originStopNumber, destStopsArr, destStopNumber, currentTime, min_trip);
	
	int min_OriginIndex = min_trip[0];
	int min_DestIndex = min_trip[1];

	double min_walk1 = originStopsArr[min_OriginIndex].distance;
	int min_origin_stop = originStopsArr[min_OriginIndex].id;
	char *origin_stop_name = originStopsArr[min_OriginIndex].name;
	int min_departureTime = min_trip[2];
	int min_trip_id = min_trip[4];
	int min_dest_stop = destStopsArr[min_DestIndex].id;
	char *dest_stop_name = destStopsArr[min_DestIndex].name;
	int min_arrivalTime = min_trip[3];
	double min_walk2 = destStopsArr[min_DestIndex].distance;



	printf("%d walk %fm to stop %d %s\n%d catch %d to stop %d %s\n%d walk %fm to destination\n%d arrive\n",
			currentTime,min_walk1, min_origin_stop, origin_stop_name, min_departureTime, min_trip_id, min_dest_stop, dest_stop_name, min_arrivalTime, min_walk2, min_arrivalTime+(int)min_walk2);

	/*
	for(int i = 0; i < originStopNumber; i++){
		printf("cost is %f\n", originStopsArr[i].distance);
	}*/
	
}


int main(void){
	
	
	find_valid_stops(-32.014402,  115.758259,  -31.981039, 115.819120, get_time("10:56"));


	return 0;
}
