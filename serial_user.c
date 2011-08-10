#include "serial_user.h"

char	SERIAL_TTY[SERIAL_MAX_DEVICES][SERIAL_MAX_DEV_LEN]; /* short form of the serial device name (eg "ttyUSB0" or "ttyS0") */
int	SERIAL_FD[SERIAL_MAX_DEVICES]; /* file descriptor of each serial port */
int	SERIAL_DEVICE_NO = 0; /* Keep track of where we put the devices and fds */

/****************************************************************************
 * OPEN
 ****************************************************************************/
int	serial_open(const char *tty, int baud_rate) {
int	l;
char	device[SERIAL_MAX_DEV_LEN];
char	short_device[SERIAL_MAX_DEV_LEN];
struct	termios options;
char	baud_rate_str[100];
int	fd;

    	if(tty[0]=='/')	snprintf(device, SERIAL_MAX_DEV_LEN, "%s", tty);
	else		snprintf(device, SERIAL_MAX_DEV_LEN, "/dev/%s", tty);
	strncpy(short_device, device + 5, strlen(device) - 5);

	fd = serial_create_lockfile(short_device);
	if(fd != SERIAL_LOCKFILE_CREATED_OK) return fd;

	for(l=0; l<SERIAL_MAX_DEVICES; l++) {
		if (strcmp(short_device, SERIAL_TTY[l]) == 0 ) {
			return SERIAL_ALREADY_OPEN; // Some other process already using this device.
			}
		}

	if(SERIAL_DEVICE_NO >= SERIAL_MAX_DEVICES) {
		return -SERIAL_MAX_DEVICES; // Maximum number of devices reached
		}
	else {
		if((fd = open(device, O_RDWR)) < 0) {
			fprintf(stderr, "serial_open: could not open device %s, ", device);
			fprintf(stderr, "the error is %d\n", fd);
			if(fd == -1) {
				fprintf(stderr, "Have you set the access permissions?\n");
				fprintf(stderr, "sudo chmod 666 %s\n", device);
				}
			return SERIAL_NOOPEN;
			}
		cfmakeraw(&options);
		options.c_cflag	= CS8|CLOCAL|CREAD;	// Character size mask, Ignore modem control lines, Enable reciever. This is very important it doesn't work without it

		switch(baud_rate) { // Embarrassingly, I don't know a better way of doing this...
			case 9600 	: cfsetispeed(&options, B9600);		cfsetospeed(&options, B9600);break;
			case 19200 	: cfsetispeed(&options, B19200);	cfsetospeed(&options, B19200);break;
			case 38400 	: cfsetispeed(&options, B38400);	cfsetospeed(&options, B38400);break;
			case 57600 	: cfsetispeed(&options, B57600);	cfsetospeed(&options, B57600);break;
			case 115200 	: cfsetispeed(&options, B115200);	cfsetospeed(&options, B115200);break;
			case 921600 	: cfsetispeed(&options, B921600);	cfsetospeed(&options, B921600);break;
			default		: cfsetispeed(&options, B9600);		cfsetospeed(&options, B9600);break;
			}
		options.c_cc[VTIME]	= 0; //Timeout in deciseconds for non-canonical read
		options.c_cc[VMIN]	= 0;
		if(tcsetattr(fd, TCSANOW, &options) < 0){
			fprintf(stderr, "serial_open: Couldn't set the usb serial port attributes.\n");
			return SERIAL_NOATTR;
			}
		fcntl(fd, F_SETFL, O_NONBLOCK);	       /* set up non-blocking read */

		// Successfully opened, we can now transfer the fd and device to the global arrays and increment the index
		strncpy(SERIAL_TTY[SERIAL_DEVICE_NO], short_device, SERIAL_MAX_DEV_LEN);
		SERIAL_FD[SERIAL_DEVICE_NO] = fd;
		SERIAL_DEVICE_NO++;

		return fd;
		}
	}

/****************************************************************************
 * CLOSE
 ****************************************************************************/
int	serial_close(int fd) {
int	l;
	close(fd);
	for(l=0; l < SERIAL_MAX_DEVICES; l++) {
		if(SERIAL_FD[l] == fd) {
			SERIAL_FD[l] = 0;
			serial_release_lockfile(SERIAL_TTY[l]);
			}
		}
	return SERIAL_OK;
	}

/****************************************************************************
 * SEND
 ****************************************************************************/
int	serial_send(int fd, const char *cmd, const char *term_chars, int term_chars_len) {
int	cmd_len, ret;
char	*out_cmd;
	cmd_len = strlen(cmd);
	out_cmd = new char[cmd_len + term_chars_len];
	memcpy(out_cmd, cmd, cmd_len);
	memcpy(out_cmd + cmd_len, term_chars, term_chars_len);
	ret = serial_send_raw(fd, out_cmd, cmd_len + term_chars_len);
	delete[] out_cmd;
	return ret;
	}

int	serial_send_str(int fd, const char *cmd) {
	return serial_send_raw(fd, cmd, strlen(cmd));
	}

int	serial_send_raw(int fd, const char *cmd, int len) {
struct	timeval Timeout;
fd_set	writefs;
int	ret,i;

	Timeout.tv_usec = 0;  /* microseconds */
	Timeout.tv_sec  = 1;  //zero just hangs
	FD_ZERO(&writefs);
	FD_SET(fd, &writefs);

	ret = select(fd+1, (fd_set *)0, &writefs,(fd_set *)0, &Timeout);
	if(ret <= 0) {
		fprintf(stderr, "SERIAL_set: select on WRITE return value = %d (should be >0).\n", ret);
		return SERIAL_WRITE_ERROR;
		}
	write(fd, (void *)cmd, len);

//	printf("Sent:\n");
//	for(i=0;i<len;i++){
//		if(cmd[i]>31)fprintf(stderr, "%c:",cmd[i]);
//		else fprintf(stderr, "[%d]:",(int)cmd[i]);
//		fflush(stdout);
//		}
//	fprintf(stderr, "\n");

//	FD_ZERO(&writefs);
//	FD_SET(fd, &writefs);
//	ret = select(fd+1, (fd_set *)0, &writefs,(fd_set *)0, &Timeout);
//	if(ret <= 0) {
//		fprintf(stderr, "SERIAL_set: select on WRITE return value = %d (should be >0).\n", ret);
//		return SERIAL_WRITE_ERROR;
//		}
//	write(fd, (void *)"\r", 2);
	return SERIAL_OK;
	}


/****************************************************************************
 * RECEIVE
 ****************************************************************************/
int	serial_receive(int fd, char *buf, int buf_len, const char terminate_read_char, const char end_of_message_char) {

ssize_t	bytes_read;
ssize_t	total_bytes_read;
int	res;
int	i;
int	all_done;
int	hold_up;
struct timeval Timeout;
fd_set	readfs;
	total_bytes_read = 0;

	i = 0;
	hold_up = 0;
	do {
		bytes_read = 0;
		all_done = 0;
		Timeout.tv_usec = 300000;	/* microseconds */
		Timeout.tv_sec  = 0;		/* seconds */

		FD_ZERO(&readfs);
		FD_SET(fd,&readfs);

		res = select(fd+1, &readfs, (fd_set *)0,(fd_set *)0, &Timeout);

		if (res<0) {
			fprintf(stderr, "Serious error: select returned %d (< 0), returning %d\n", i, SERIAL_READ_SELECT_ERROR);
			return SERIAL_READ_SELECT_ERROR;
			}

		if (res==0) {
//			fprintf(stderr, "Timed out\n");
			hold_up++;
			}
		else{
			bytes_read = read(fd,(void *)(buf + i), buf_len - i);
			total_bytes_read += bytes_read;
//			fprintf(stderr, "%d bytes read this time, %d bytes read so far\n", bytes_read, total_bytes_read);
			i = i + bytes_read;
			}
		} while (buf[i-1] != terminate_read_char && i < buf_len && hold_up<10 && bytes_read != 0);
	// The PI Mercury controller sends a termination sequence of "\r\n\0x03" at then end of the message (ascii 03 is "ETX")
	// We look for "03" to find out when we're done.

//	printf("\ni = %d\n",i);
//	fprintf(stderr, "%d bytes read in total, which were: \n",(int)total_bytes_read);
//	for(i=0;i<(int)total_bytes_read;i++){
//		if(buf[i]>31)fprintf(stderr, "%c:",buf[i]);
//		else fprintf(stderr, "[%d]:",(int)buf[i]);
//		fflush(stdout);
//		}
//	fprintf(stderr, "\n");

	// This is relatively common, and can happen for a variety of reasons. Closing the serial
	// port and reopening it again usually fixes it.
	if(bytes_read == 0) {
//		fprintf(stderr, "Timed out, and have not read any data (0 bytes). Returning with -3\n");
		return SERIAL_EMPTY_DATA;
		}
	i = 0;
	do (1);
	while (buf[i++] != end_of_message_char && i<buf_len);
	if(i>=buf_len) {
		fprintf(stderr, "Error: couldn't find end_of_message char in buf\n");
		}

	buf[i - 1]='\0'; // overwrite the end_of_message_char

	if(hold_up == 10) {
		fprintf(stderr, "Timed out 10 times, although there was still data coming in.\nVery odd! Returning %d\n", SERIAL_REPEATED_TIMEOUTS);
		return SERIAL_REPEATED_TIMEOUTS;
		}

	return SERIAL_OK;
	}

/****************************************************************************
 * LOCKFILE
 ****************************************************************************/
int	serial_create_lockfile(const char *device_name) {
FILE	* temp;
char	serial_lockfile[SERIAL_MAX_DEV_LEN];
char	pidname[200];
int	id, count = 0;

	snprintf(serial_lockfile, SERIAL_MAX_DEV_LEN, "%s%s", SERIAL_LOCKFILEDIR, device_name);
	if(access(serial_lockfile, F_OK) == 0){
		temp=fopen(serial_lockfile, "r");
		fscanf(temp, "%d", &id);
		fclose(temp);
		sprintf(pidname, "/proc/%d", id);
		while (count <= 3 && access(pidname, F_OK)==0) {
			sleep(1);
			count++;
			}	
		if (count >= 3) {
			fprintf(stderr, "Lockfile %s found.\n", serial_lockfile);
			fprintf(stderr, "It is being held by process with PID %d, which is still running.\n", id);
			fprintf(stderr, "Cannot release lock, returning with exit code %d.\n", SERIAL_LOCKFILE_PROCESS_CONSISTENTLY_RUNNING);
			return SERIAL_LOCKFILE_PROCESS_CONSISTENTLY_RUNNING;
			}
		}

	if(access(SERIAL_LOCKFILEDIR,W_OK)!=0){
		fprintf(stderr, "No lock file found (good); however, cannot write to directory %s\n", SERIAL_LOCKFILEDIR);
		fprintf(stderr, "Returning with exit code %d\n", SERIAL_CANNOT_WRITE_LOCKFILE);
		return SERIAL_CANNOT_WRITE_LOCKFILE;
		}
	temp = fopen(serial_lockfile, "w");
	id = getpid();
	fprintf(temp, "%d\n", (int)id);
	fclose(temp);
	return SERIAL_LOCKFILE_CREATED_OK;
	}

void	serial_release_lockfile(const char *device_name) {
char	serial_lockfile[SERIAL_MAX_DEV_LEN];
	snprintf(serial_lockfile, SERIAL_MAX_DEV_LEN, "%s%s", SERIAL_LOCKFILEDIR, device_name);
	unlink(serial_lockfile);
	}

