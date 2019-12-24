#ifndef FILE_HANDLE_H
#define FILE_HANDLE_H

#include "Commons.h"

typedef struct guest
{
	char name[MAX_NAME_LENGTH];
	int budget;
} guest_t;

typedef struct room
{
	char name[MAX_NAME_LENGTH];
	int price;
	int max_occupants;
} room_t;

/**
*	Reads information about all rooms in the hotel from the specified file and fills the
*	specified rooms array.
*
*	Accepts
*	-------
*   rooms_filename - a string representing the name of the file containing the rooms.
*	rooms - an array to be filled with the rooms information.
*	rooms_count - a pointer to an integer which will contain the actual number of rooms in the hotel.
*
*	Returns
*	-------
*	An EXIT_CODE inidcating wether the read operation was succefull.
**/
EXIT_CODE readRoomsFromFile(const char *rooms_filename, room_t rooms[], int *rooms_count);

/**
*	reads a single value from the file specified in filename.
*
*	Accepts
*	-------
*	filename - a string representing the name of the file.
*	value - a pointer to an integer which will contain the value that was read.
*
*	Returns
*	-------
*	An EXIT_CODE inidcating wether the read operation was succefull.
**/
EXIT_CODE readFromFile(char *filename, int *value);

/**
*	Writes the specified value to the file specified in filename.
*
*	Accepts
*	-------
*	filename - a string representing the name of the file.
*	value - an integer containing the value that should be written into the file.
*
*	Returns
*	-------
*	An EXIT_CODE inidcating wether the write operation was succefull.
**/
EXIT_CODE writeToFile(char *filename, int value);

#endif
