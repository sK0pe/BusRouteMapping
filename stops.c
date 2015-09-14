#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define PI acos(-1.0)
#define EARTH_RADIUS_M 6372797
#define FOLDER "transperth"
#define STOPS "/stops.txt"

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




int *find_valid_stops(double origLat, double origLon, double destLat, double destLon){
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
		if(haversine(origLat, origLon, stop_lat, stop_lon) <= 1000){
			originStops++;
		}
		if(haversine(destLat, destLon, stop_lat, stop_lon) <= 1000){
			destinationStops++;
		}
	}
	arraySizes[0] = originStops;
	arraySizes[1] = destinationStops;
	//	Close file
	if(stopData != NULL){
		fclose(stopData);
	}
	printf("%d\n", originStops);
	printf("%d\n", destinationStops);
	return arraySizes;
}

int main(void){
	
	find_valid_stops(-32.014402,  115.758259,  -31.981039, 115.819120);


	return 0;
}
