#include <stdio.h>
#include <string.h>
#include "Commons.h"
#include "FileHandle.h"

#define ROOM_NAME_INDEX 0
#define ROOM_PRICE_INDEX 1
#define ROOM_MAX_OCCUPANTS_INDEX 2

#define GUEST_NAME_INDEX 0
#define GUEST_BUDGET_INDEX 1

EXIT_CODE addRoom(char *room_info_line, room_t rooms[], int room_index);
EXIT_CODE addGuest(char *guest_info_line, guest_t guests[], int guest_index);

/**
*	Reads information about all rooms in the hotel from the specified file and fills the 
*	specified rooms array.
*
*	Accepts
*	-------
*	dir_path - the path to the directory containing the rooms file.
*   rooms_filename - a string representing the name of the file containing the rooms.
*	rooms - an array to be filled with the rooms information.
*	rooms_count - a pointer to an integer which will contain the actual number of rooms in the hotel.
*
*	Returns
*	-------
*	An EXIT_CODE inidcating wether the read operation was succefull.
**/
EXIT_CODE readRoomsFromFile(const char *dir_path, const char *rooms_filename, room_t rooms[], int *rooms_count)
{
	EXIT_CODE exit_code = HM_SUCCESS;
	int rooms_filepath_length = 0;
	char *rooms_filepath;
	FILE *file;
	errno_t file_open_code;
	char room_info_line[MAX_LINE_LENGTH];
	int room_index = 0;

	rooms_filepath_length = strlen(dir_path) + 2 + strlen(rooms_filename) + 1;
	rooms_filepath = (char*)malloc(sizeof(char)*rooms_filepath_length);
	sprintf_s(rooms_filepath, rooms_filepath_length, "%s//%s", dir_path, rooms_filename);

	file_open_code = fopen_s(&file, rooms_filepath, "r");

	if (file_open_code != 0)
	{
		printf("An error occured while openning file %s for reading.", rooms_filepath);
		free(rooms_filepath);
		return HM_FILE_OPEN_FAILED;
	}

	while (fgets(room_info_line, MAX_LINE_LENGTH, file) != NULL)
	{
		addRoom(room_info_line, rooms, room_index);
		room_index++;
	}

	fclose(file);

	*rooms_count = room_index;

	free(rooms_filepath);
	return exit_code;
}

EXIT_CODE addRoom(char *room_info_line, room_t rooms[], int room_index)
{
	EXIT_CODE exit_code = HM_SUCCESS;
	const char *delim = " ";
	char *token;
	char *next_token;
	int token_count = ROOM_NAME_INDEX;
	room_t room;

	room.num_of_guests = 0;

	token = strtok_s(room_info_line, " ", &next_token);
	while (token != NULL)
	{
		switch (token_count)
		{
			case ROOM_NAME_INDEX:
				strcpy_s(room.name, MAX_NAME_LENGTH, token);
				break;
			case ROOM_PRICE_INDEX:
				sscanf_s(token, "%d", &(room.price));
				break;
			case ROOM_MAX_OCCUPANTS_INDEX:
				sscanf_s(token, "%d", &(room.max_occupants));
			default:
				break;
		}

		token = strtok_s(NULL, " ", &next_token);
		token_count++;
	}

	rooms[room_index] = room;

	return exit_code;
}

/**
*	Reads information about all guests in the hotel from the specified file and fills the
*	specified guests array.
*
*	Accepts
*	-------
*	dir_path - the path to the directory containing the rooms file.
*   guests_filename - a string representing the name of the file containing the guests.
*	guests - an array to be filled with the guests information.
*	guest_count - a pointer to an integer which will contain the actual number of guests in the hotel.
*
*	Returns
*	-------
*	An EXIT_CODE inidcating wether the read operation was succefull.
**/
EXIT_CODE readGuestsFromFile(const char *dir_path, const char *guests_filename, guest_t guests[], int *guest_count)
{
	EXIT_CODE exit_code = HM_SUCCESS;
	int guests_filepath_length = 0;
	char *guests_filepath;
	FILE *file;
	errno_t file_open_code;
	char guest_info_line[MAX_LINE_LENGTH];
	int guest_index = 0;

	guests_filepath_length = strlen(dir_path) + 2 + strlen(guests_filename) + 1;
	guests_filepath = (char*)malloc(sizeof(char)*guests_filepath_length);
	sprintf_s(guests_filepath, guests_filepath_length, "%s//%s", dir_path, guests_filename);

	file_open_code = fopen_s(&file, guests_filepath, "r");

	if (file_open_code != 0)
	{
		printf("An error occured while openning file %s for reading.", guests_filepath);
		free(guests_filepath);
		return HM_FILE_OPEN_FAILED;
	}

	while (fgets(guest_info_line, MAX_LINE_LENGTH, file) != NULL)
	{
		addGuest(guest_info_line, guests, guest_index);
		guest_index++;
	}

	fclose(file);

	*guest_count = guest_index;

	free(guests_filepath);
	return exit_code;
}

/*adds a guest to guests DB accepts a strng 
from guests file, guests array and guest index*/

EXIT_CODE addGuest(char *guest_info_line, guest_t guests[], int guest_index)
{
	EXIT_CODE exit_code = HM_SUCCESS;
	const char *delim = " ";
	char *token;
	char *next_token;
	int token_count = GUEST_NAME_INDEX;
	guest_t guest;

	token = strtok_s(guest_info_line, " ", &next_token);
	while (token != NULL)
	{
		switch (token_count)
		{
			case GUEST_NAME_INDEX:
				strcpy_s(guest.name, MAX_NAME_LENGTH, token);
				break;
			case GUEST_BUDGET_INDEX:
				sscanf_s(token, "%d", &(guest.budget));
				break;
			default:
				break;
		}

		token = strtok_s(NULL, " ", &next_token);
		token_count++;
	}

	guests[guest_index] = guest;

	return exit_code;
}

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
EXIT_CODE readFromFile(char *filename, int *value)
{
	FILE *file;
	errno_t exit_code;

	exit_code = fopen_s(&file, filename, "r");

	if (exit_code != 0)
	{
		printf("An error occured while openning file %s for reading.", filename);
		return HM_FILE_OPEN_FAILED;
	}

	fscanf_s(file, "%d", value);

	fclose(file);

	return 0;
}

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
EXIT_CODE writeToFile(const char *dir_path, char *log_filename, room_t *room, guest_t *guest, int day)
{
	FILE *file;
	errno_t exit_code;
	int log_filepath_length = 0;
	char *log_filepath;

	log_filepath_length = strlen(dir_path) + 2 + strlen(log_filename) + 1;
	log_filepath = (char*)malloc(sizeof(char)*log_filepath_length);
	sprintf_s(log_filepath, log_filepath_length, "%s//%s", dir_path, log_filename);

	exit_code = fopen_s(&file, log_filepath, "a");
	if (exit_code != 0)
	{
		printf("An error occured while openning file %s for writing.", log_filename);
		return HM_FILE_OPEN_FAILED;
	}
	switch (guest->status)
	{
	case GUEST_IN:
		fprintf_s(file, "%s %s IN %d\n", room->name, guest->name, day);
		break;
	case GUEST_OUT:
		fprintf_s(file, "%s %s OUT %d\n", room->name, guest->name, day); 
		break;
	default :
		break;
	}


	fclose(file);

	return HM_SUCCESS;
}