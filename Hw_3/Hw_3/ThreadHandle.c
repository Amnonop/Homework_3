#include <windows.h>
#include "ThreadHandle.h"
#include "Commons.h"

/**
*	Handles the creation of a thread. It returns a handle to the created thread.
*
*	Accepts
*	-------
*	p_start_routine - a pointer to the routine the thread will run.
*	p_thread_parameters - a pointer to the thread parameters.
*	p_thread_id - a pointer to be assigned the thread id.
*
*	Returns
*	-------
*	A handle to the created thread.
**/
HANDLE createThreadSimple(LPTHREAD_START_ROUTINE p_start_routine,
	LPVOID p_thread_parameters,
	LPDWORD p_thread_id)
{
	HANDLE thread_handle;

	if (NULL == p_start_routine)
	{
		printf("Error when creating a thread");
		printf("Received null pointer");
	}

	/*if (NULL == p_thread_id)
	{
		printf("Error when creating a thread");
		printf("Received null pointer");
	}*/

	thread_handle = CreateThread(
		NULL,                /*  default security attributes */
		0,                   /*  use default stack size */
		p_start_routine,     /*  thread function */
		p_thread_parameters, /*  argument to thread function */
		0,                   /*  use default creation flags */
		p_thread_id);        /*  returns the thread identifier */

	if (NULL == thread_handle)
	{
		printf("Couldn't create thread\n");
	}

	return thread_handle;
}