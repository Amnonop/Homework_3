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

#define NUM_OF_ARGC 2
#define MAIN_DIRECTORY_ARG_INDEX 1




int main(int argc, char *argv[])
{
	EXIT_CODE exit_code = HM_SUCCESS;
	int main_dir_path_length = 0;
	char *main_dir_path;
	if (argc < NUM_OF_ARGC)
		return HM_NOT_ENOUGH_COMMAND_LINE_ARGUMENTS;
	main_dir_path_length = strlen(argv[MAIN_DIRECTORY_ARG_INDEX]) + 1;
	main_dir_path = (char*)malloc(sizeof(char)*main_dir_path_length);
	strcpy_s(main_dir_path, main_dir_path_length, argv[MAIN_DIRECTORY_ARG_INDEX]);
	runHotel(main_dir_path);
	free(main_dir_path);
	return exit_code;
}