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
#include "HotelManager.h"
#include "ThreadHandle.h"

/*declerations*/
int getGuestsFromFile(char* filename, guest_t *guests_list[]);
int getRoomsFromFile(char* filename, room_t *rooms_list[]);
int computeGuestsNights(room_t *rooms_list[], int number_of_rooms, guest_t *guests_list[], int number_of_guests);
void writeLogFile(guest_t *guests_list[], room_t *rooms_list[], int day_counter);

typedef struct _room
{
	char *name;
	int price;
	int num_of_beds;
	int num_of_guests;
	HANDLE room_mutex_handle;
} room;

typedef struct _guest
{
	char *name;
	int budget;
	int nights;
	char *room;
	char *status;
	HANDLE guestt_mutex_handle;
} guest;

typedef struct _guest_thread_input
{
	guest_t guest;
	room_t *rooms;
	int num_of_rooms;
	HANDLE *room_semaphore_handles;
} guest_thread_input_t;

HANDLE room_semaphore_handles[NUM_OF_ROOMS];

DWORD WINAPI guestThread(LPVOID argument);

EXIT_CODE runHotel(const char *main_dir_path)
{
	const char *rooms_filename = "rooms.txt";
	const char *guests_filename = "names.txt";
	EXIT_CODE exit_code = HM_SUCCESS;
	room_t rooms[NUM_OF_ROOMS];
	guest_t guests[NUM_OF_GUESTS];
	int rooms_count;
	int guests_count;

	
	int room_semaphores_count;

	HANDLE thread_handles[NUM_OF_GUESTS];
	guest_thread_input_t thread_inputs[NUM_OF_GUESTS];
	int threads_count;
	DWORD threads_wait_result;

	exit_code = readRoomsFromFile(main_dir_path, rooms_filename, rooms, &rooms_count);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	exit_code = readGuestsFromFile(main_dir_path, guests_filename, guests, &guests_count);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	// Create semaphores for the rooms
	for (room_semaphores_count = 0; room_semaphores_count < rooms_count; room_semaphores_count++)
	{
		room_semaphore_handles[room_semaphores_count] = CreateSemaphore(
			NULL,												/* Default security attributes */
			rooms[room_semaphores_count].max_occupants,			/* Initial Count - the room is empty */
			rooms[room_semaphores_count].max_occupants,			/* Maximum Count */
			NULL);												/* un-named */

		if (room_semaphore_handles[room_semaphores_count] == NULL)
		{
			printf("Failed to create semaphore. Terminating.\n");
			exit_code = HM_SEMAPHORE_CREATE_FAILED;
			// Cleanup
		}
	}

	for (threads_count = 0; threads_count < guests_count; threads_count++)
	{
		thread_inputs[threads_count].guest = guests[threads_count];
		thread_inputs[threads_count].rooms = rooms;
		thread_inputs[threads_count].num_of_rooms = rooms_count;
		thread_inputs[threads_count].room_semaphore_handles = room_semaphore_handles;

		thread_handles[threads_count] = createThreadSimple(
			(LPTHREAD_START_ROUTINE)guestThread, 
			(LPVOID)&(thread_inputs[threads_count]), 
			NULL);

		if (thread_handles[threads_count] == NULL)
		{
			printf("Failed to create thread. Terminating.\n");
			exit_code = HM_THREAD_CREATE_FAILED;
			// Cleanup
		}

		printf("created thread for guest %s\n", guests[threads_count].name);
	}

	threads_wait_result = WaitForMultipleObjects(
		guests_count, 
		thread_handles, 
		TRUE, 
		INFINITE);

	/*read rooms - struct #1*/
	/*read residents - struct #2*/
	/*devide to rooms - mod(%)*/
	/*day counter ++*/
	/*remove resident when night to stay  = 0*/
	/*finish when nights to stay for every resident  = 0*/
	int day_counter = 0;
	int number_of_guests = 0;
	int number_of_rooms = 0;
	guest *guests_list[NUM_OF_GUESTS];
	room *rooms_list[NUM_OF_ROOMS];
	int number_of_nights = 0;

	//num_of_guests = getGuestsFromFile(guests_file,  guests_list);
	//num_of_rooms = getRoomsFromFile(rooms_file, rooms_list);
	number_of_nights = computeGuestsNights(rooms_list, number_of_rooms, guests_list, number_of_guests);
	while (number_of_nights > 0)
	{
		day_counter++;
		/*put in rooms*/
		/*update budget*/
		/*remove out of money guests*/
		writeLogFile(guests_list, rooms_list, day_counter); //not written yet
		number_of_nights--; //the number of nights for the last guest to stay
	}

	return exit_code;
}

DWORD WINAPI guestThread(LPVOID argument)
{
	guest_thread_input_t *thread_input;
	int room_index = -1;
	DWORD room_wait_result;
	DWORD room_release_result;
	LONG previous_count;

	thread_input = (guest_thread_input_t*)argument;

	// Look for a room
	printf("looking a room for %s\n", thread_input->guest.name);
	room_index = findRoom(thread_input->guest.budget, thread_input->rooms, thread_input->num_of_rooms);
	printf("%s can stay in room %d named %s\n", thread_input->guest.name, room_index, (thread_input->rooms[room_index]).name);

	// Use a semaphore to enter the room
	printf("%s waiting to enter room %s\n", thread_input->guest.name, thread_input->rooms[room_index].name);
	room_wait_result = WaitForSingleObject(room_semaphore_handles[room_index], INFINITE);
	if (room_wait_result != WAIT_OBJECT_0)
	{
		// Report error
	}

	/* Start critical section */

	printf("%s entered room %s\n", thread_input->guest.name, thread_input->rooms[room_index].name);

	// Wait the number of nights the guest can stay

	/* End criticial section */

	room_release_result = ReleaseSemaphore(room_semaphore_handles[room_index], 1, &previous_count); /* can set previous to null if not interested*/
	if (room_release_result == FALSE)
	{
		// Report error
	}

	printf("%s left room %s\n", thread_input->guest.name, thread_input->rooms[room_index].name);
}

int findRoom(int budget, room_t rooms[], int num_of_rooms)
{
	int i;

	for (i = 0; i < num_of_rooms; i++)
	{
		if ((budget % rooms[i].price) == 0)
			break;
	}

	return i;
}

int getGuestsFromFile(char* filename, guest *guests_list[])
{
	int sub_grade = 0;
	FILE *fp;
	errno_t error;
	char* name = "";
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
			fscanf_s(fp, 256, "%s %d", name, budget);
			strcpy_s(guests_list[i]->name, MAX_NAME_LENGTH, name);
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
	char* name = "";
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
			fscanf_s(fp, 256, "%s %d %d", name, price, num_of_beds);
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
	for (int i = 0; i < number_of_rooms; i++)
	{
		rooms_list[i]->num_of_guests = 0;
	}
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
				i++;
				j = 0;
			}
		}
	}
	return max_number_of_nights;
}



int computeGuestsInRooms(room *rooms_list[], int number_of_rooms, guest *guests_list[], int number_of_guests)
{
	for (int i = 0; i < number_of_guests; i++)
	{
		for (int j = 0; j < number_of_rooms; j++)
		{
			if (rooms_list[j]->price % guests_list[i]->budget == 0)
			{
				if (rooms_list[j]->num_of_guests < rooms_list[j]->num_of_beds)
				{
					strcpy_s(guests_list[i]->room, MAX_NAME_LENGTH, rooms_list[j]->name);
					rooms_list[j]->num_of_guests++;
					strcpy_s(guests_list[i]->status, 5, "IN");
					guests_list[i]->nights--;
					if (guests_list[i]->nights < 0)
					{
						strcpy_s(guests_list[i]->status, 5, "OUT");
					}
				}
				else
				{
					strcpy_s(guests_list[i]->status, 4, "WAITING");
				}
				i++;
				j = 0;
			}
		}
	}
}



void writeLogFile(guest *guests_list[], room *rooms_list[], int day_counter)
{

	writeToFile;
	return 0;
}