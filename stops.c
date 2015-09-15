#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define PI acos(-1.0)
#define EARTH_RADIUS_M 6372797
#define WALK_SPEED	60.0
#define MAX_WALK 1000
#define FOLDER "transperth"
#define STOPS "/stops.txt"

typedef struct {
	int id;
	char name[50];
	double time_cost;
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
 * Populatse the Origin and Destination Stop
 * Arrays with information from STOPS file
 *
 */
void populate_stop_arrays(Stop *originStopsArr, Stop *destStopsArr, 
		double origLat, double origLon, double destLat, double destLon){
	
	FILE *stopData = NULL;
	stopData = loadFile(stopData,STOPS);
	bool first = true;
	char line[BUFSIZ];	//	Line buffer
	int stop_id;
	char *stop_name;
	double origin_stop_cost;
	double dest_stop_cost;
	double stop_lat;
	double stop_lon;
	int origin_stop_counter = 0;
	int dest_stop_counter = 0;
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
					originStopsArr[origin_stop_counter].time_cost = origin_stop_cost/WALK_SPEED;
					++origin_stop_counter;
					break;
				}
				if((dest_stop_cost <= MAX_WALK) && (dest_stop_cost < origin_stop_cost)){
					//	Enter id
					destStopsArr[dest_stop_counter].id = stop_id;
					//	Enter / copy over name
					strcpy(destStopsArr[dest_stop_counter].name, stop_name);
					//	Enter time cost
					destStopsArr[dest_stop_counter].time_cost = dest_stop_cost/WALK_SPEED;
					++dest_stop_counter;
					break;
				}
			}
			fieldNum++;
			field = tokenizer(NULL, ",");
		}
	}
}

/*
 *	find_valid_stops
 *
 *	Fills origin and destination arrays of type Stop with STOP file
 *	information.
 */
 void find_valid_stops(double origLat, double origLon, double destLat, double destLon){
	//	Find number of valid stops at origin and destination
	int *size = get_stop_arraysize(origLat, origLon, destLat, destLon);
	int originStopNumber = size[0];
	int destStopNumber = size[1];
	//	Make array of stops of appropriate size
	Stop originStopsArr[ originStopNumber ];
	Stop destStopsArr[ destStopNumber ];
	//	Populate Stop Arrays
	populate_stop_arrays(originStopsArr, destStopsArr, origLat, origLon, destLat, destLon);

	for(int i = 0; i < originStopNumber; i++){
		printf("%d is %s\n", originStopsArr[i].id, originStopsArr[i].name);
	}

}


int main(void){
	
	
	find_valid_stops(-32.014402,  115.758259,  -31.981039, 115.819120);


	return 0;
}
