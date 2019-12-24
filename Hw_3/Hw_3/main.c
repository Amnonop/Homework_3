#include <stdlib.h>
#include <limits.h>
#include <stdio.h> 
#include <string.h> 
#include <errno.h>
#include <math.h>
#include <windows.h>
#include <stdbool.h>
#include <tchar.h>
#include "Commons.h"
#include "FileHandle.h"
#include "ThreadHandle.h"

#define NUM_OF_ARGC 2
#define GRADE_DIRECTORY_ARG_INDEX 1

/*declerations*/
int getGuestsFromFile(char* filename, guest *guests_list[]);
int getRoomsFromFile(char* filename, room *rooms_list[]);
int computeGuestsNights(room *rooms_list[], int number_of_rooms, guest *guests_list[], int number_of_guests);
void writeLogFile(guest *guests_list[], room *rooms_list, int day_counter);

typedef struct _room
{
	char *name;
	int price;
	int num_of_beds;
	HANDLE room_mutex_handle;
} room;

typedef struct _guest
{
	char *name;
	int budget;
	int nights;
	char *room;
	HANDLE guestt_mutex_handle;
} guest;

int getGuestsFromFile(char* filename, guest *guests_list[])
{
	int sub_grade = 0;
	FILE *fp;
	errno_t error;
	char* name;
	int budget = 0;
	int count = NUM_OF_GUESTS;
	int i = 0;
		error = fopen_s(&fp, filename, "r");

	if (error != 0)
		printf("An error occured while openning file %s for reading.", filename);

	else if (fp)
	{
		for (i = 0; i < count; i++)
		{
			fscanf(fp, "%s %d", name, budget);
			strcpy_s(guests_list[i]->name, MAX_NAME_LENGTH ,name);
			guests_list[i]->budget = budget;
			//nights left will be filled later
		}

	}
	//returns number of guests
	return i;
}


int getRoomsFromFile(char* filename, room *rooms_list[])
{
	int sub_grade = 0;
	FILE *fp;
	errno_t error;
	char* name;
	int price = 0;
	int num_of_beds = 0;
	int count = NUM_OF_GUESTS;
	int i = 0;
	error = fopen_s(&fp, filename, "r");

	if (error != 0)
		printf("An error occured while openning file %s for reading.", filename);

	else if (fp)
	{
		for (i = 0; i < count; i++)
		{
			fscanf(fp, "%s %d %d", name, price ,num_of_beds);
			strcpy_s(rooms_list[i]->name, MAX_NAME_LENGTH, name);
			rooms_list[i]->price = price;
			rooms_list[i]->num_of_beds = num_of_beds;
		}
	}
	//resutrns number of rooms
	return i;
}

/*returns the number of nights for the last guest to stay*/
int computeGuestsNights(room *rooms_list[], int number_of_rooms, guest *guests_list[], int number_of_guests)
{
	int max_number_of_nights = 0;
	for (int i = 0; i < number_of_guests; i++)
	{
		for (int j = 0; j < number_of_rooms; j++)
		{
			if (rooms_list[j]->price % guests_list[i]->budget == 0)
			{
				guests_list[i]->nights = rooms_list[j]->price / guests_list[i]->budget;
				if (guests_list[i]->nights > max_number_of_nights)
				{
					max_number_of_nights = guests_list[i]->nights;
				}
				strcpy_s(guests_list[i]->room, MAX_NAME_LENGTH, rooms_list[j]->name);
				i++;
				j = 0;
			}
		}
	}
	return max_number_of_nights;
}

void writeLogFile(guest *guests_list[],room *rooms_list,int day_counter)
{
	
	return 0;
}



int main(int argc, char *argv[])
{
	/*read rooms - struct #1*/
	/*read residents - struct #2*/
	/*devide to rooms - mod(%)*/
	/*day counter ++*/
	/*remove resident when night to stay  = 0*/
	/*finish when nights to stay for every resident  = 0*/
	EXIT_CODE exit_code;
	int day_counter = 0;
	int number_of_guests = 0;
	int number_of_rooms = 0;
	guest *guests_list[NUM_OF_GUESTS];
	room *rooms_list[NUM_OF_ROOMS];
	int number_of_nights = 0;
	if (argc < NUM_OF_ARGC)
		return TG_NOT_ENOUGH_COMMAND_LINE_ARGUMENTS;
	//num_of_guests = getGuestsFromFile(guests_file,  guests_list);
	//num_of_rooms = getRoomsFromFile(rooms_file, rooms_list);
	number_of_nights = computeGuestsNights(rooms_list, number_of_rooms, guests_list, number_of_guests);
	while (number_of_nights > 0)
	{
		day_counter++;
		/*put in rooms*/
		/*update budget*/
		/*remove out of money guests*/
		writeLogFile(guests_list[], rooms_list, day_counter); //not written yet
		number_of_nights--; //the number of nights for the last guest to stay
	}
	return exit_code;