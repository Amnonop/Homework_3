#ifndef COMMONS_H
#define COMMONS_H

typedef int EXIT_CODE;

#define NUM_OF_ROOMS 20
#define NUM_OF_GUESTS 20
#define MAX_NAME_LENGTH 256
#define TRUE 1
#define FALSE 0

typedef enum {
	TG_SUCCESS,
	TG_THREADS_WAIT_FAILED,
	TG_FILE_OPEN_FAILED,
	TG_THREAD_PARAMS_NULL,
	TG_THREAD_CREATE_FAILED,
	TG_MUTEX_CREATE_FAILED,
	TG_MUTEX_ABANDONED,
	TG_MUTEX_WAIT_FAILED,
	TG_MUTEX_RELEASE_FAILED,
	TG_NOT_ENOUGH_COMMAND_LINE_ARGUMENTS
} EXIT_CODES;

#define TG_THREADS_WAIT_FAILED ((int)(-1))
#define ERROR_CLOSING_THREAD ((int)(-2))
#define ERROR_OPEN_FILE ((int)(-3))
#define ERROR_LPPARAM_NULL ((int)(-4))
#define ERROR_CREATING_THREAD ((int)(-5))
#define ERROR_MUTEX_CREATE_FAILED -6
#define ERROR_MUTEX_ABANDONED -7
#define ERROR_MUTEX_WAIT_FAILED -8
#define ERROR_MUTEX_RELEASE_FAILED -9;

/*global variables*/
static int midterm_grade = 0;
static int exam_grade = 0;
//static int hw_grades[NUM_OF_HW] = { 0 };

#endif