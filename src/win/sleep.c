#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
unsigned int sleep(unsigned int sec) {
	Sleep(1000 * sec);
	return 0;
}

void msleep(unsigned int msec) {
	Sleep(msec);
}

int usleep(useconds_t useconds) {
	Sleep(useconds / 1000);
	return 0;
}

#endif

