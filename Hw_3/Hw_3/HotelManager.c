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

typedef enum
{
	THREAD_STATUS_NONE,
	WAITING_TO_ENTER_ROOM,
	IN_WORKING,
	IN_WAITING_FOR_DAY,
	OUT_DONE,
	THREAD_STATUS_COUNT
} GUEST_THREAD_STATUS;

/*structs*/

/*single thread input database*/
typedef struct _guest_thread_input
{
	guest_t guest;
	room_t *rooms;
	int num_of_rooms;
	HANDLE *room_semaphore_handles;
	char **path;
	HANDLE guest_status_mutex_handle;
} guest_thread_input_t;

typedef struct _day_manager_input
{
	HANDLE guest_status_mutex_handle;
} day_manager_input_t;

/*global variables*/
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
int guest_status_count[THREAD_STATUS_COUNT];

/*Function declarations*/
DWORD WINAPI guestThread(LPVOID argument);
int findRoom(int budget, room_t rooms[], int num_of_rooms);
EXIT_CODE updateRoomGuestCount(GUEST_STATUS guest_status, room_t *room);
DWORD WINAPI dayManagerThread(LPVOID arguments);
HANDLE getDayPassedEvent();
EXIT_CODE updateGuestStatusCount(GUEST_THREAD_STATUS curr_guest_status, GUEST_THREAD_STATUS prev_guest_status, HANDLE guest_status_mutex_handle);
EXIT_CODE countHandledGuests(HANDLE guest_status_mutex_handle, int *handled_guests_count, int *out_guests_count);
BOOL closeHandle(HANDLE handle);
BOOL closeHandles(HANDLE handles_list[], int num_of_handles);
BOOL closeRoomMutexHandles(room_t rooms[], int mutex_handles_count);

/*main tread - runs the program from main, receives directory*/
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
	HANDLE guest_status_mutex_handle;
	day_manager_input_t day_manager_input;

	// Initialize
	day = 1;

	exit_code = readRoomsFromFile(main_dir_path, rooms_filename, rooms, &rooms_count);
	if (exit_code != HM_SUCCESS)
		return exit_code;
	exit_code = readGuestsFromFile(main_dir_path, guests_filename, guests, &guests_count);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	guest_status_count[THREAD_STATUS_NONE] = guests_count;
	for (i = 1; i < THREAD_STATUS_COUNT; i++)
	{
		guest_status_count[i] = 0;
	}

	// Create mutexes for the rooms
	for (i = 0; i < rooms_count; i++)
	{
		rooms[i].room_mutex_handle = CreateMutex(
			NULL, 
			FALSE, 
			room_mutex_names[i]);

		if (rooms[i].room_mutex_handle == NULL)
		{
			closeRoomMutexHandles(rooms, i);
			return HM_MUTEX_CREATE_FAILED;
		}
	}

	log_file_mutex_handle = CreateMutex(
		NULL,
		FALSE,
		"log_file_mutex_handle");
	if (log_file_mutex_handle == NULL)
	{
		closeRoomMutexHandles(rooms, rooms_count);
		return HM_MUTEX_CREATE_FAILED;
	}

	guest_status_mutex_handle = CreateMutex(
		NULL,
		FALSE,
		"guest_status_mutex");
	if (guest_status_mutex_handle == NULL)
	{
		closeRoomMutexHandles(rooms, rooms_count);
		closeHandle(log_file_mutex_handle);
		return HM_MUTEX_CREATE_FAILED;
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

			closeRoomMutexHandles(rooms, rooms_count);
			closeHandle(log_file_mutex_handle);
			closeHandles(room_semaphore_handles, room_semaphores_count);
			return HM_SEMAPHORE_CREATE_FAILED;
		}
	}

	//thread per guest creations
	for (threads_count = 0; threads_count < guests_count; threads_count++)
	{
		//guest-thread arguments
		thread_inputs[threads_count].guest = guests[threads_count];
		thread_inputs[threads_count].rooms = rooms;
		thread_inputs[threads_count].num_of_rooms = rooms_count;
		thread_inputs[threads_count].room_semaphore_handles = room_semaphore_handles;
		thread_inputs[threads_count].path = &main_dir_path;
		thread_inputs[threads_count].guest_status_mutex_handle = guest_status_mutex_handle;

		thread_handles[threads_count] = createThreadSimple(
			(LPTHREAD_START_ROUTINE)guestThread,
			(LPVOID)&(thread_inputs[threads_count]),
			NULL);

		if (thread_handles[threads_count] == NULL)
		{
			printf("Failed to create thread. Terminating.\n");
			
			closeRoomMutexHandles(rooms, rooms_count);
			closeHandle(log_file_mutex_handle);
			closeHandles(room_semaphore_handles, room_semaphores_count);
			closeHandles(thread_handles, threads_count);
			return HM_THREAD_CREATE_FAILED;
		}
	}

	day_manager_input.guest_status_mutex_handle = guest_status_mutex_handle;
	thread_handles[threads_count] = createThreadSimple(
		(LPTHREAD_START_ROUTINE)dayManagerThread, 
		(LPVOID)&day_manager_input, 
		NULL);

	if (thread_handles[threads_count] == NULL)
	{
		closeRoomMutexHandles(rooms, rooms_count);
		closeHandle(log_file_mutex_handle);
		closeHandles(room_semaphore_handles, room_semaphores_count);
		closeHandles(thread_handles, threads_count);
		return HM_THREAD_CREATE_FAILED;
	}

	threads_count++;

	// Wait for all threads to finish their execution
	threads_wait_result = WaitForMultipleObjects(
		threads_count, 
		thread_handles, 
		TRUE, 
		INFINITE);

	// Print the number of days it took to manage the hotel
	printf("%d\n", day);

	// Close all created handles
	closeRoomMutexHandles(rooms, rooms_count);
	closeHandle(log_file_mutex_handle);
	closeHandles(room_semaphore_handles, room_semaphores_count);
	closeHandles(thread_handles, threads_count);

	// Return with success code
	return HM_SUCCESS;
}


/*runs the thread of every guest.
called from runHotel 
receives the arguments pre-defined per guests-thread - see:guest-thread arguments*/
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
	room_index = findRoom(thread_input->guest.budget, thread_input->rooms, thread_input->num_of_rooms);
	thread_input->guest.room_index = room_index;

	// Update status to waiting
	exit_code = updateGuestStatusCount(WAITING_TO_ENTER_ROOM, THREAD_STATUS_NONE, thread_input->guest_status_mutex_handle);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	thread_input->guest.status = GUEST_WAITING;

	// Use a semaphore to enter the room
	room_wait_result = WaitForSingleObject(room_semaphore_handles[room_index], INFINITE);
	if (room_wait_result != WAIT_OBJECT_0)
	{
		// Report error
	}

	// Update number of guests in the room and update status
	exit_code = updateGuestStatusCount(IN_WORKING, WAITING_TO_ENTER_ROOM, thread_input->guest_status_mutex_handle);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	thread_input->guest.status = GUEST_IN;
	exit_code = updateRoomGuestCount(GUEST_IN, &(thread_input->rooms[room_index]));
	//critical part - write to log file
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
	//end of critical part
	// Wait the number of nights the guest can stay
	while (thread_input->guest.budget != 0)
	{
		day_passed_event_handle = getDayPassedEvent();
		updateGuestStatusCount(IN_WAITING_FOR_DAY, IN_WORKING, thread_input->guest_status_mutex_handle);
		event_wait_result = WaitForSingleObject(day_passed_event_handle, INFINITE);
		ResetEvent(day_passed_event_handle);
		// A day has passed - update budget
		updateGuestStatusCount(IN_WORKING, IN_WAITING_FOR_DAY, thread_input->guest_status_mutex_handle);
		thread_input->guest.budget = thread_input->guest.budget - thread_input->rooms[room_index].price;
	}

	room_release_result = ReleaseSemaphore(room_semaphore_handles[room_index], 1, &previous_count); /* can set previous to null if not interested*/
	if (room_release_result == FALSE)
	{
		// Report error
	}

	//out of budget- getting out of the room
	exit_code = updateGuestStatusCount(OUT_DONE, IN_WORKING, thread_input->guest_status_mutex_handle);
	if (exit_code != HM_SUCCESS)
		return exit_code;

	thread_input->guest.status = GUEST_OUT;
	exit_code = updateRoomGuestCount(GUEST_OUT, &(thread_input->rooms[room_index]));
	//critical part - write to log file
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
	//end of critical part
}

/* find every guest his desired room, return the room index to the guest thread*/
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

/*updates how many guests in the room after a single operation or more in the hotel*/
EXIT_CODE updateRoomGuestCount(GUEST_STATUS guest_status, room_t *room)
{
	DWORD wait_result;
	DWORD release_result;
	//critical part
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
			//1 guest entered room %s, now has guests
			break;
		case GUEST_OUT:
			room->num_of_guests -= 1;
			//1 guest left room 
			break;
		default:
			break;
	}
	release_result = ReleaseMutex(room->room_mutex_handle);
	if (release_result == FALSE)
		return HM_MUTEX_RELEASE_FAILED;
	//end of critical part
	return HM_SUCCESS;
}

EXIT_CODE updateGuestStatusCount(GUEST_THREAD_STATUS curr_guest_status, GUEST_THREAD_STATUS prev_guest_status, HANDLE guest_status_mutex_handle)
{
	DWORD handle_result;

	handle_result = WaitForSingleObject(guest_status_mutex_handle, INFINITE);
	if (handle_result != WAIT_OBJECT_0)
	{
		if (handle_result == WAIT_ABANDONED)
			return HM_MUTEX_ABANDONED;
		else
			return HM_MUTEX_WAIT_FAILED;
	}

	/* Enter critical section */
	guest_status_count[prev_guest_status]--;
	guest_status_count[curr_guest_status]++;
	/* End critical section */

	handle_result = ReleaseMutex(guest_status_mutex_handle);
	if (handle_result == FALSE)
		return HM_MUTEX_RELEASE_FAILED;

	return HM_SUCCESS;
}

/*changing and counting the days/ nights in the hotel, sends an event to all guests threads*/
DWORD WINAPI dayManagerThread(LPVOID arguments)
{
	day_manager_input_t *thread_input;
	int all_guests_handled = FALSE;
	int handled_guests = 0;
	int out_guests_count = 0;
	HANDLE day_passed_event_handle;
	BOOL is_success;

	thread_input = (day_manager_input_t*)arguments;
		
	while (true)
	{
		Sleep(1000);

		countHandledGuests(thread_input->guest_status_mutex_handle, &handled_guests, &out_guests_count);
		if (out_guests_count == guests_count)
		{
			return HM_SUCCESS;
		}
		else if (handled_guests == guests_count)
		{
			day_passed_event_handle = getDayPassedEvent();
			day++;
			printf("\n---- DAY %d ----\n", day);
			//event : day has passed
			is_success = SetEvent(day_passed_event_handle);
		}
	}
}

EXIT_CODE countHandledGuests(HANDLE guest_status_mutex_handle, int *handled_guests_count, int *out_guests_count)
{
	DWORD handle_result;

	handle_result = WaitForSingleObject(guest_status_mutex_handle, INFINITE);
	if (handle_result != WAIT_OBJECT_0)
	{
		if (handle_result == WAIT_ABANDONED)
			return HM_MUTEX_ABANDONED;
		else
			return HM_MUTEX_WAIT_FAILED;
	}

	/* Enter critical section */
	*handled_guests_count = 
		guest_status_count[WAITING_TO_ENTER_ROOM] 
		+ guest_status_count[IN_WAITING_FOR_DAY] 
		+ guest_status_count[OUT_DONE];
	*out_guests_count = guest_status_count[OUT_DONE];
	printf("waiting for room = %d, waiting for day = %d, out = %d\n", guest_status_count[WAITING_TO_ENTER_ROOM], guest_status_count[IN_WAITING_FOR_DAY], guest_status_count[OUT_DONE]);
	/* End critical section */

	handle_result = ReleaseMutex(guest_status_mutex_handle);
	if (handle_result == FALSE)
		return HM_MUTEX_RELEASE_FAILED;

	return HM_SUCCESS;
}

//for the day to get passed - handking the event
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

BOOL closeHandle(HANDLE handle)
{
	BOOL close_handle_result;

	close_handle_result = CloseHandle(handle);
	return close_handle_result;
}

BOOL closeHandles(HANDLE handles_list[], int num_of_handles)
{
	int i;

	for (i = 0; i < num_of_handles; i++)
	{
		CloseHandle(handles_list[i]);
	}

	return TRUE;
}

BOOL closeRoomMutexHandles(room_t rooms[], int mutex_handles_count)
{
	int i;

	for (i = 0; i < mutex_handles_count; i++)
	{
		CloseHandle(rooms[i].room_mutex_handle);
	}

	return TRUE;
}

