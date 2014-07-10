#ifndef _SERIAL_USER_H_
#define _SERIAL_USER_H_

#define SERIAL_LOCKFILE "/etc/serial/"
#define SERIAL_LOCKFILEDIR "/etc/serial/"

#define	SERIAL_MAX_DEVICES				256
#define	SERIAL_MAX_DEV_LEN				128

#define SERIAL_OK					0
#define SERIAL_NOOPEN					-1
#define SERIAL_NOATTR					-2
#define	SERIAL_ALREADY_OPEN				-3
#define	SERIAL_WRITE_ERROR				-4
#define	SERIAL_READ_SELECT_ERROR			-5
#define	SERIAL_REPEATED_TIMEOUTS			-6
#define	SERIAL_REPEATED_EMPTY_DATA			-7
#define	SERIAL_EMPTY_DATA				-8
#define	SERIAL_CANNOT_WRITE_ERRORLOG_FILE		-10
#define	SERIAL_CANNOT_WRITE_LOCKFILE			-11
#define	SERIAL_LOCKFILE_PROCESS_CONSISTENTLY_RUNNING	-12
#define SERIAL_LOCKFILE_CREATED_OK			0

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
int serial_create_lockfile(const char *device_name);
void serial_release_lockfile(const char *device_name);

#endif
