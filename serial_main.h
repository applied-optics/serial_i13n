#ifndef __SERIAL_MAIN__
#define __SERIAL_MAIN__
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
using namespace std;

#define SERIAL_NOOPEN	-1
#define SERIAL_NOATTR	-2
#define	SERIAL_NOMEM	-3
#define SERIAL_ASYNC	2
#define SERIAL_OK	1

int SERIAL_open(char *,int);
int SERIAL_get(char *);
int SERIAL_close(void);

#endif
