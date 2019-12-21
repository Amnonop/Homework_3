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
#include "ThreadHandle.h"

#define NUM_OF_ARGC 2
#define GRADE_DIRECTORY_ARG_INDEX 1

int main(int argc, char *argv[])
{
	int grades_directory_length = 0;
	char *grades_directory;
	EXIT_CODE exit_code;

	if (argc < NUM_OF_ARGC)
		return TG_NOT_ENOUGH_COMMAND_LINE_ARGUMENTS;

	grades_directory_length = strlen(argv[GRADE_DIRECTORY_ARG_INDEX]) + 1;
	grades_directory = (char*)malloc(sizeof(char)*grades_directory_length);
	strcpy_s(grades_directory, grades_directory_length, argv[GRADE_DIRECTORY_ARG_INDEX]);

	exit_code = calculateGrade(grades_directory);

	free(grades_directory);

	return exit_code;