#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <string.h>
//#include
//#include
// ...

#define  N_COLS              400
#define  N_FIELDS            15
#define  MAX_WALKING_DIST    1000
#define  EARTH_RADIUS_M      6372797
//#define 
//#define 
// ...

// temporary: global variables here for now
FILE   *fp_stops;
FILE   *fp_stop_times;
FILE   *fp_trips;
FILE   *fp_routes;
FILE   *fp_calendar;

double lat1;
double lon1;
double lat2;
double lon2;
double finalDistance;
double tempDistance;

char  line_stops[N_COLS];
char  line_stop_times[N_COLS];
char  line_trips[N_COLS];
char  line_routes[N_COLS];
char  line_cal[N_COLS];
char  distData[N_COLS];
char  distData1[N_COLS];
char  fields_stops[N_FIELDS][N_COLS];
char  fields_stop_times[N_FIELDS][N_COLS];
char  fields_trips[N_FIELDS][N_COLS];
char  fields_routes[N_FIELDS][N_COLS];
char  fields_cal[N_FIELDS][N_COLS];
char  *transType[] = {",",",","train","bus","ferry"}; // transport type...
char  yStrk[40]; // For strk function...

int   time;
int   day;
int   count = 0;
int   num   = 0;
int   test  = 0;

int   flag = 0;


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
	double distance = 2 * EARTH_RADIUS_M * asin(sqrt(x));

	return distance;
}

// splits line using delimeter
void split(char line[], char del, char splitter[][N_FIELDS][N_COLS]) 
{
	for(int i = 0, j = 0, k = 0; line[i]; i++)
	{
		if (line[i] == del)
		{
			j++;
			k = 0;
			continue;
		}
		splitter[0][j][k] = line[i];
		k++;
	}
}

// This function seperates tokens based on delimeters
char * strk(char *x, char del)
{	
	memset(yStrk, '\0', sizeof yStrk);

	for (int i = 0, j = 0, z = 0; x[i]; i++) {
		if (num == 0) {
			if(x[i] == del)
				break;
			yStrk[j] = x[i];
			j++;
		}
		else if (num == 1) {
			if (x[i] == del) {
				if (z)
					break;
				z = 1;
			}
			else if (z) {
				yStrk[j] = x[i];
				j++;
			}
		}
		else {
			if(x[i] == ' ') {
				z = 1;
			}
			else if (x[i] == del)
				break;
			else if (z) {
				yStrk[j] = x[i];
				j++;
			}
		}
	}
	return yStrk;
}

// find the day
int getDay(char *x)
{
	if (!strcmp(x, "Mon"))
		return 1;
	else if (!strcmp(x, "Tue"))
		return 2;
	else if (!strcmp(x, "Wed"))
		return 3;
	else if (!strcmp(x, "Thu"))
		return 4;
	else if (!strcmp(x, "Fri"))
		return 5;
	else if(!strcmp(x, "Sat"))
		return 6;
	else
		return 7;
}

void openFiles(char *dir)
{
	char path[30];
	memset(path, '\0', sizeof path);

	strcpy(path, dir);
	strcat(path, "stops.txt");
	fp_stops = fopen(path, "r");

	strcpy(path, dir);
	strcat(path, "stop_times.txt");
	fp_stop_times = fopen(path, "r");

	strcpy(path, dir);
	strcat(path, "trips.txt");
	fp_trips = fopen(path, "r");

	strcpy(path, dir);
	strcat(path, "routes.txt");
	fp_routes = fopen(path, "r");

	strcpy(path, dir);
	strcat(path, "calendar.txt");
	fp_calendar = fopen(path, "r");
}

void closeFiles(void)
{
	fclose(fp_stops);
	fclose(fp_stop_times);
	fclose(fp_trips);
	fclose(fp_routes);
	fclose(fp_calendar);
}

void process_stop() 
{
	double temp;
	double temp1;
	double temp2;
	int    tempTime;
	int    i, j;
	char   *x;

	flag = 0;
	fseek(fp_stops, 0, SEEK_SET); // Set File ptr to beginning of file
	while (fgets(line_stops,sizeof(line_stops), fp_stops)) // Read stop.txt...
	{ 
		memset(fields_stops, '\0', sizeof(fields_stops));
		split(line_stops, ',', &fields_stops);

		temp  = haversine(lat1,lon1,(double)atof(fields_stops[6]), (double)atof(fields_stops[7])); // get distance from start to stop...
		temp1 = haversine((double)atof(fields_stops[6]), (double)atof(fields_stops[7]),lat2, lon2); // get distance from start to stop...
		if (test == 0)
			test = 1000;
		if (temp < test) 
		{ // walkable distance...
			fseek(fp_stop_times, 0, SEEK_SET);
			while (fgets(line_stop_times, sizeof(line_stop_times), fp_stop_times))
			{
				// find bus/train time for a stop...
				memset(fields_stop_times, '\0', sizeof(fields_stop_times));
				split(line_stop_times, ',', &fields_stop_times);

				if (strcmp(fields_stop_times[3], fields_stops[2]) == 0) 
				{
					num      = 0;
					x        = strk(fields_stop_times[1], ':');
					tempTime = atoi(x) * 60;
					num      = 1;
					x        = strk(fields_stop_times[1], ':');

					tempTime += atoi(x);
					if (tempTime -(int)(temp/60) > time && time + (int)(temp/60) + 20 - tempTime > 0) // Give sufficient time for walking...
					{	
						fseek(fp_trips, 0 ,SEEK_SET);
						while (strcmp(fields_stop_times[0],fields_trips[2])) 
						{
							// Get trip ID...
							fgets(line_trips, sizeof(line_trips), fp_trips);
							memset(fields_trips, '\0', sizeof(fields_trips));
							split(line_trips, ',', &fields_trips);
						}
						fseek(fp_calendar, 0, SEEK_SET);
						while(strcmp(fields_trips[1], fields_cal[0]))
						{
							// For checking availability on a given day...
							fgets(line_cal, sizeof(line_cal), fp_calendar);
							memset(fields_cal, '\0', sizeof(fields_cal));
							split(line_cal, ',', &fields_cal);
						}
						if (strcmp(fields_cal[day], "1") == 0) // service available today...
						{ 
							printf("%d:%.2d\twalk %.0lfm to stop %s %s\n", time/60, time%60, temp, fields_stops[2], fields_stops[4]); // walk to stop...
							fseek(fp_routes, 0, SEEK_SET);

							while(strcmp(fields_trips[0], fields_routes[0]))
							{
								// Find bus/train name/no.from stop...
								fgets(line_routes, sizeof(line_routes), fp_routes);
								memset(fields_routes, '\0', sizeof(fields_routes));
								split(line_routes, ',', &fields_routes);
							}
							time = tempTime;
							printf("%d:%.2d\tcatch %s ", time/60, time%60, transType[atoi(fields_routes[5])]);
							if(fields_routes[2])
								printf("%s to stop ",fields_routes[2]); // Bus No...
							else
								printf("%s to stop ",fields_routes[3]); // Train Name...

							temp2 = tempDistance;
							while (strcmp(fields_stop_times[0], fields_trips[2]) == 0) 
							{
								// Get the best stop to get down from Bus/Train(Nearest to destn.)
								fgets(line_stop_times, sizeof(line_stop_times), fp_stop_times);
								memset(fields_stop_times, '\0', sizeof(fields_stop_times));
								split(line_stop_times, ',', &fields_stop_times);

								fseek(fp_stops, 0, SEEK_SET);
								while(strcmp(fields_stop_times[3], fields_stops[2])) 
								{
									fgets(line_stops, sizeof(line_stops), fp_stops);
									memset(fields_stops, '\0', sizeof(fields_stops));
									split(line_stops, ',', &fields_stops);
								}
								temp1 = haversine((double)atof(fields_stops[6]), (double)atof(fields_stops[7]), lat2, lon2);
								if(temp1 < temp2) // Best stop to get down...
								{ 
									temp2 = temp1;
									strcpy(distData, line_stops);
									strcpy(distData1, line_stop_times);
								}
							} // Bus Train...
							memset(fields_stops, '\0', sizeof(fields_stops));
							split(distData, ',', &fields_stops);
							memset(fields_stop_times, '\0', sizeof(fields_stop_times));
							split(distData1, ',', &fields_stop_times);
							printf("%s %s\n", fields_stops[2], fields_stops[4]); // stop to get down...

							num = 0;
							i =atoi(strk(fields_stop_times[1], ':'));
							i -= time/60;
							num = 1;
							j = atoi(strk(fields_stop_times[1], ':'));
							if (i)
								i += j + (60 - time % 60);
							else
								i += j - time % 60;
							time += i; // Find current time...
							lat1 = atof(fields_stops[6]); // Find current coordinates...
							lon1 = atof(fields_stops[7]);
							tempDistance = haversine((double)atof(fields_stops[6]), (double)atof(fields_stops[7]), lat2, lon2); // Update distance left to be travelled...
							flag = 1; // flag to get out of stop.txt loop...
							test = MAX_WALKING_DIST;
							count++;
							break;
						}
						else       // Service unavailable today.find another...
							continue;
					}
					if (flag)
						break;
				}
			}
		} // stop_times.txt
	}
} // stops.txt

int main(int argc, char **argv)
{
	int result;

	if (argc != 6) 
	{ 
		printf("%% Usage: ./whichbus <directory> <Source coordinates> <Destn coordinates>\n");
		result = EXIT_FAILURE;
	}
	else
	{
		lat1 = atof(argv[2]);
		lon1 = atof(argv[3]);
		lat2 = atof(argv[4]);
		lon2 = atof(argv[5]);
		if (valid_location(lat1, lon1) == false)
		{
			printf("%s: Location 1 is invalid!\n", argv[0]);
			result = EXIT_FAILURE;
		}
		else if (valid_location(lat2, lon2) == false)
		{
			printf("%s: Location 2 is invalid!\n", argv[0]);
			result = EXIT_FAILURE;
		}
		else
		{
			finalDistance = haversine(lat1, lon1, lat2, lon2); // Total Travel distance...
			tempDistance  = finalDistance;


			char *env = getenv("LEAVEHOME");
			char *x;


			num = 0;
			x    = strk(env, ' ');
			day  = getDay(x);      // find day as set in LEAVEHOME
			num = 2;
			x    = strk(env, ':');
			time = atoi(x) * 60;
			num = 1;
			x    = strk(env, ':');
			time = time + atoi(x);   // find time in mins

			openFiles(argv[1]);

			while (1) 	
			{
				if (tempDistance < MAX_WALKING_DIST) // walkable distance
				{
					printf("%d:%.2d\twalk %.0lfm to destination\n", time/60, time%60, tempDistance);
					time += tempDistance / 60;
					printf("%d:%.2d\tarrive\n", time/60, time%60);	
					break;
				}
				else if (count == 2) // walk the remainding distance left
				{
					printf("%d:%.2d\twalk %.0lfm to destination\n", time/60, time%60, tempDistance/1000);
					tempDistance /= 1000;
					time += tempDistance/60;
					printf("%d:%.2d\tarrive\n", time/60, time%60);
					break;
				}
				process_stop();
			}
			closeFiles();
			result = EXIT_SUCCESS;
		}
	}
	return result;
}
