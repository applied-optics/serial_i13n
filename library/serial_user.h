#ifndef __SERIAL_USER__
#define __SERIAL_USER__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define SERIAL_LOCKFILE "/etc/serial/"
#define SERIAL_LOCKFILEDIR "/etc/serial/"

#define	SERIAL_MAX_DEVICES				256
#define	SERIAL_MAX_DEV_LEN				128

// the following #defines are also in serial_main, and it's possible both will
// be used by c_scan (hopefully not).
#ifndef SERIAL_NOOPEN
#define SERIAL_NOOPEN					-1
#endif

#ifndef SERIAL_NOATTR
#define SERIAL_NOATTR					-2
#endif

#ifndef	SERIAL_OK
#define SERIAL_OK					0	// this is 1 in Matt's library
#endif

#define	SERIAL_ALREADY_OPEN				-3

#define	SERIAL_LOCKFILE_PROCESS_CONSISTENTLY_RUNNING	-12
#define	SERIAL_CANNOT_WRITE_LOCKFILE			-11
#define	SERIAL_CANNOT_WRITE_ERRORLOG_FILE		-10
#define SERIAL_LOCKFILE_CREATED_OK			0

#define	SERIAL_WRITE_ERROR				-4

#define	SERIAL_READ_SELECT_ERROR			-5
#define	SERIAL_REPEATED_TIMEOUTS			-6
#define	SERIAL_REPEATED_EMPTY_DATA			-7
#define	SERIAL_EMPTY_DATA				-8
#define SERIAL_BUF					128

int serial_open(const char *tty, int baud_rate);
int serial_close(int fd);
int serial_send(int fd, const char *cmd, const char *term_chars,
		int term_chars_len);
int serial_send_str(int fd, const char *cmd);
int serial_send_raw(int fd, const char *cmd, int len);
int serial_receive(int fd, char *buf, int buf_len,
		   const char terminate_read_char,
		   const char end_of_message_char);
int serial_create_lockfile(const char *);
void serial_release_lockfile(const char *);

#endif
