#ifndef THREAD_HANDLE_H
#define THREAD_HANDLE_H

#include <windows.h>

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
	LPDWORD p_thread_id);

#endif
