#ifndef FILE_HANDLE_H
#define FILE_HANDLE_H

#include "Commons.h"

typedef struct guest
{
	char *name;
	int budget;
} guest_t;

typedef struct room
{
	char *name;
	int price;
	int max_occupants;
} room_t;

/* Reads a grade from the file specified in filename*/
int getGradeFromFile(char* filename);

/**
*	Reads a grade from a file in the grades directory.
*
*	Accepts
*	-------
*	grades_directory - a string representing the name of the directory containing the file.
*   grade_filename - a string representing the name of the file containing the grade.
*	grade - a pointer to an integer which will contain the grade value that was read.
*
*	Returns
*	-------
*	An EXIT_CODE inidcating wether the read operation was succefull.
**/
EXIT_CODE readGradeFromFile(const char *grades_directory, const char *grade_filename, int *grade);

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
