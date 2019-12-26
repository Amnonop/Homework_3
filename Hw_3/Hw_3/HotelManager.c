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
	char **path;
} guest_thread_input_t;

static const char DAY_PASSED_EVENT_NAME[] = "day_passed_event";

char *room_mutex_names[NUM_OF_ROOMS] = { "room_0_mutex", "room_1_mutex", "room_2_mutex", "room_3_mutex", "room_4_mutex" };
HANDLE room_semaphore_handles[NUM_OF_ROOMS];
HANDLE log_file_mutex_handle;
guest_t guests[NUM_OF_GUESTS];
int day;
int guests_count;
room_t rooms[NUM_OF_ROOMS];
int rooms_count;
int num_of_guests_out = 0;
//const char *main_dir_path;

DWORD WINAPI guestThread(LPVOID argument, char *main_dir_path);
int findRoom(int budget, room_t rooms[], int num_of_rooms);
EXIT_CODE updateRoomGuestCount(GUEST_STATUS guest_status, room_t *room);
DWORD WINAPI dayManagerThread(LPVOID arguments);
HANDLE getDayPassedEvent();





EXIT_CODE runHotel(const char *main_dir_path)
{
	const char *rooms_filename = "rooms.txt";
	const char *guests_filename = "names.txt";
	EXIT_CODE exit_code = HM_SUCCESS;
	int i;
	int room_semaphores_count;
	HANDLE thread_handles[NUM_OF_GUESTS + 1];
	guest_thread_input_t thread_inputs[NUM_OF_GUESTS];
	int threads_count;
	DWORD threads_wait_result;
	HANDLE day_manager_handle;
	day = 1;
	exit_code = readRoomsFromFile(main_dir_path, rooms_filename, rooms, &rooms_count);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	exit_code = readGuestsFromFile(main_dir_path, guests_filename, guests, &guests_count);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	// Create mutexes for the rooms
	for (i = 0; i < rooms_count; i++)
	{
		rooms[i].room_mutex_handle = CreateMutex(
			NULL, 
			FALSE, 
			room_mutex_names[i]);

		if (rooms[i].room_mutex_handle == NULL)
		{
			// Cleanup
		}
	}
	log_file_mutex_handle = CreateMutex(
		NULL,
		FALSE,
		"log_file_mutex_handle");
	if (log_file_mutex_handle == NULL)
	{
		// Cleanup
	}
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

		thread_inputs[threads_count].path = &main_dir_path;

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

		//printf("created thread for guest %s\n", guests[threads_count].name);
	}

	thread_handles[threads_count] = createThreadSimple(
		(LPTHREAD_START_ROUTINE)dayManagerThread, 
		NULL, 
		NULL);

	threads_wait_result = WaitForMultipleObjects(
		guests_count + 1, 
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

	//???????//
	while (number_of_nights > 0)
	{
		day_counter++;
		number_of_nights--; //the number of nights for the last guest to stay
	}

	return exit_code;
}

DWORD WINAPI guestThread(LPVOID argument)
{
	EXIT_CODE exit_code = HM_SUCCESS;
	guest_thread_input_t *thread_input;
	int room_index = -1;
	DWORD room_wait_result;
	DWORD room_release_result;
	DWORD wait_log_file_mutex;
	DWORD release_log_file_mutex;
	LONG previous_count;
	HANDLE day_passed_event_handle;
	DWORD event_wait_result;
	const char *log_filename = "roomLog.txt";

	thread_input = (guest_thread_input_t*)argument;

	// Look for a room
	//printf("looking a room for %s\n", thread_input->guest.name);
	room_index = findRoom(thread_input->guest.budget, thread_input->rooms, thread_input->num_of_rooms);
	thread_input->guest.room_index = room_index;
	//printf("%s can stay in room %d named %s\n", thread_input->guest.name, room_index, (thread_input->rooms[room_index]).name);

	// Use a semaphore to enter the room
	thread_input->guest.status = GUEST_WAITING;
	//printf("%s waiting to enter room %s\n", thread_input->guest.name, thread_input->rooms[room_index].name);
	room_wait_result = WaitForSingleObject(room_semaphore_handles[room_index], INFINITE);
	if (room_wait_result != WAIT_OBJECT_0)
	{
		// Report error
	}

	// Update number of guests in the room
	thread_input->guest.status = GUEST_IN;
	exit_code = updateRoomGuestCount(GUEST_IN, &(thread_input->rooms[room_index]));

	//printf("%s entered room %s\n", thread_input->guest.name, thread_input->rooms[room_index].name);
	//printf("%s %s IN %d\n", thread_input->rooms[room_index].name, thread_input->guest.name, day);
	wait_log_file_mutex = WaitForSingleObject(log_file_mutex_handle, INFINITE);
	if (wait_log_file_mutex != WAIT_OBJECT_0)
	{
		if (wait_log_file_mutex == WAIT_ABANDONED)
			return HM_MUTEX_ABANDONED;
		else
			return HM_MUTEX_WAIT_FAILED;
	}
	writeToFile(*(thread_input->path), log_filename, &(thread_input->rooms[room_index]), &(thread_input->guest), day);
	release_log_file_mutex = ReleaseMutex(log_file_mutex_handle);
	if (release_log_file_mutex == FALSE)
		return HM_MUTEX_RELEASE_FAILED;

	// Wait the number of nights the guest can stay
	while (thread_input->guest.budget != 0)
	{
		day_passed_event_handle = getDayPassedEvent();
		//printf("%s waiting for the day to pass.\n", thread_input->guest.name);
		event_wait_result = WaitForSingleObject(day_passed_event_handle, INFINITE);
		ResetEvent(day_passed_event_handle);

		// A day has passed - update budget
		thread_input->guest.budget = thread_input->guest.budget - thread_input->rooms[room_index].price;
		//printf("%s day %d budget %d\n", thread_input->guest.name, day, thread_input->guest.budget);
	}

	room_release_result = ReleaseSemaphore(room_semaphore_handles[room_index], 1, &previous_count); /* can set previous to null if not interested*/
	if (room_release_result == FALSE)
	{
		// Report error
	}

	thread_input->guest.status = GUEST_OUT;
	exit_code = updateRoomGuestCount(GUEST_OUT, &(thread_input->rooms[room_index]));

	//printf("%s left room %s\n", thread_input->guest.name, thread_input->rooms[room_index].name);
	//printf("%s %s OUT %d\n", thread_input->rooms[room_index].name, thread_input->guest.name, day);
	wait_log_file_mutex = WaitForSingleObject(log_file_mutex_handle, INFINITE);
	if (wait_log_file_mutex != WAIT_OBJECT_0)
	{
		if (wait_log_file_mutex == WAIT_ABANDONED)
			return HM_MUTEX_ABANDONED;
		else
			return HM_MUTEX_WAIT_FAILED;
	}
	writeToFile(*(thread_input->path), log_filename, &(thread_input->rooms[room_index]), &(thread_input->guest), day);
	num_of_guests_out++;
	release_log_file_mutex = ReleaseMutex(log_file_mutex_handle);
	if (release_log_file_mutex == FALSE)
		return HM_MUTEX_RELEASE_FAILED;
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

EXIT_CODE updateRoomGuestCount(GUEST_STATUS guest_status, room_t *room)
{
	DWORD wait_result;
	DWORD release_result;

	wait_result = WaitForSingleObject(room->room_mutex_handle, INFINITE);
	if (wait_result != WAIT_OBJECT_0)
	{
		if (wait_result == WAIT_ABANDONED)
			return HM_MUTEX_ABANDONED;
		else
			return HM_MUTEX_WAIT_FAILED;
	}

	switch (guest_status)
	{
		case GUEST_IN:
			room->num_of_guests += 1;
			//printf("1 guest entered room %s, now has %d/%d guests.\n", room->name, room->num_of_guests, room->max_occupants);
			break;
		case GUEST_OUT:
			room->num_of_guests -= 1;
			//printf("1 guest left room %s, now has %d/%d guests.\n", room->name, room->num_of_guests, room->max_occupants);
			break;
		default:
			break;
	}

	release_result = ReleaseMutex(room->room_mutex_handle);
	if (release_result == FALSE)
		return HM_MUTEX_RELEASE_FAILED;

	return HM_SUCCESS;
}

DWORD WINAPI dayManagerThread(LPVOID arguments)
{
	int all_guests_handled = FALSE;
	int handled_guests = 0;
	HANDLE day_passed_event_handle;
	BOOL is_success;

	while (true)
	{
		Sleep(2000);

		day_passed_event_handle = getDayPassedEvent();
		day++;

		is_success = SetEvent(day_passed_event_handle);
	}
}

HANDLE getDayPassedEvent()
{
	HANDLE day_passed_event_handle;
	DWORD last_error;

	day_passed_event_handle = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		DAY_PASSED_EVENT_NAME
	);

	last_error = GetLastError();

	return day_passed_event_handle;
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


