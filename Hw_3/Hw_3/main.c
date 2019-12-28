#include <stdlib.h>
#include <stdio.h> 
#include "Commons.h"
#include "HotelManager.h"

#define NUM_OF_ARGC 2
#define MAIN_DIRECTORY_ARG_INDEX 1

/**
*	This program manages a hotel.
*	It assigns each guest a room, using semaphores to manage the number of guests allowed into each room.
*	Once all guests have been handled for the day, the next day begins.
*	The program ends when all guests are out of the hotel. 
*	The in and out status of each guest is written to a log file.
*
*	Accepts
*	-------
*	full_path - a path to the directory containing all relevant files.
*
*	Example
*	-------
*	Hw3.exe <full_path>
**/
int main(int argc, char *argv[])
{
	EXIT_CODE exit_code = HM_SUCCESS;
	int main_dir_path_length = 0;
	char *main_dir_path;
	if (argc < NUM_OF_ARGC)
		return HM_NOT_ENOUGH_COMMAND_LINE_ARGUMENTS;

	main_dir_path_length = strlen(argv[MAIN_DIRECTORY_ARG_INDEX]) + 1;
	main_dir_path = (char*)malloc(sizeof(char)*main_dir_path_length);
	strcpy_s(main_dir_path, main_dir_path_length, argv[MAIN_DIRECTORY_ARG_INDEX]);

	exit_code = runHotel(main_dir_path);

	free(main_dir_path);
	return exit_code;
}