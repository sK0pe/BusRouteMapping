#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>


#define  EARTH_RADIUS_M   6372797


bool valid_location(double latitude, double longitude)
{
	return (latitude  >= -90.0  && latitude  <= 90.0  &&
			    longitude >= -180.0 && longitude <= 180.0);
}

double haversine(double lat1, double lon1, double lat2, double lon2) 
{
	double deltalat = (lat2 - lat1) / 2.0;
	double deltalon = (lon2 - lon1) / 2.0;
	double sin1     =	sin(deltalat * M_PI / 180.0);
	double cos1     = cos(lat1     * M_PI / 180.0);
	double cos2     = cos(lat2     * M_PI / 180.0);
	double sin2     = sin(deltalon * M_PI / 180.0);
	double x        = sin1*sin1 + cos1*cos2 * sin2*sin2;
	double metres   = 2 * EARTH_RADIUS_M * asin(sqrt(x));

	return metres;
}

void split(char *line, char d[][20][200]) 
{
	int  i;
	int  j;
	int  k;

	for(i=0, j=0, k=0; line[i]; i++)
	{
		if (line[i] == ',')
		{
			j++;
			k = 0;
			continue;
		}
		d[0][j][k] = line[i];
		k++;
	}
}

// Doesn't return or pass any value (distance, stop details) back up to main yet
// This is just a very small change: so far, it reads file stops.txt, finds the 
// coordinates that are closest within 1000m, and prints it for debugging purposes
void findStop(char path[], double lat1, double lon1, double lat2, double lon2)
{
	FILE  *fp;

	double distance = 1000;

	char line[400];
  char stop_data[400];
	char splitter[20][200];

	fp = fopen(path, "r");

	while (fgets(line, sizeof line, fp)) 
	{
		split(line, &splitter);

		double temp = haversine(lat1, lon1, atof(splitter[6]), atof(splitter[7]));
		if (temp < distance) 
		{
			distance = temp;
			strcpy(stop_data, line);
		}
	}
	// print the details and distance
	printf("%s%dm\n", stop_data, (int) distance);

	fclose(fp);
}

int main(int argc, char *argv[]) 
{
	int result;

	if(argc != 6)
	{
		fprintf(stderr, "%% Usage: %s source lat_start lon_start lat_dest lon_dest\n", argv[0]);
		result = EXIT_FAILURE;
	}
	else
	{
		double lat1 = atof(argv[2]);
		double lon1 = atof(argv[3]);

		double lat2 = atof(argv[4]);
		double lon2 = atof(argv[5]);

		// check if start and end locations are valid
		if (!valid_location(lat1, lon1))
		{
			printf("%s: Location 1 is invalid!\n", argv[0]);
			result = EXIT_FAILURE;
		}
		else if(!valid_location(lat2, lon2))
		{
			printf("%s: Location 2 is invalid!\n", argv[0]);
			result = EXIT_FAILURE;
		}
		else
		{

			// just temporary - getting the path name for stop.txt
			// will need to use a better method for finding the files later

			char path[30];

			strcpy(path, argv[1]);
			if (argv[1][strlen(argv[1])-1]=='/')
				strcat(path, "stops.txt"); 
			else
				strcat(path, "/stops.txt");
			
			findStop(path, lat1, lon1, lat2, lon2);
							
			result = EXIT_SUCCESS;
		}
	}

	return result;
}
