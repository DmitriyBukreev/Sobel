#include <errno.h>
#include <pam.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef HANDLE_ERROR
#define HANDLE_ERROR(result, condition) \
do { \
	if ((result) == (condition)) { \
		perror("Error"); \
		exit(errno); \
	} \
} while (0)
#endif