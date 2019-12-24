#include <stdio.h>
#include "Commons.h"
#include "FileHandle.h"



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
EXIT_CODE readGradeFromFile(const char *grades_directory, const char *grade_filename, int *grade)
{
	char *grade_file_path;
	int filename_length;
	EXIT_CODE EXIT_CODE;

	filename_length = strlen(grades_directory) + 2 + strlen(grade_filename) + 1;
	grade_file_path = (char*)malloc(sizeof(char)*filename_length);
	sprintf_s(grade_file_path, filename_length, "%s//%s", grades_directory, grade_filename);

	EXIT_CODE = readFromFile(grade_file_path, grade);

	free(grade_file_path);

	return EXIT_CODE;
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
		return ERROR_OPEN_FILE;
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
EXIT_CODE writeToFile(char *filename, char *room_name, char *guest_name,char *guest_status, int number_of_guests)
{
	FILE *file;
	errno_t exit_code;

	exit_code = fopen_s(&file, "roomsLog.txt", "w");
	if (exit_code != 0)
	{
		printf("An error occured while openning file %s for writing.", filename);
		return ERROR_OPEN_FILE;
	}

	fprintf_s(file, "%s %s %s %d", room_name, guest_name, guest_status, number_of_guests);

	fclose(file);

	return TG_SUCCESS;
}