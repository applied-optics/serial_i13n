#include "serial_main.h"
int SERIAL_in;
int SERIAL_set=0;
struct termios SERIAL_tp;
struct termios SERIAL_save;

#define SERIAL_MAX_BUFFER	100

unsigned char *SERIAL_buffer;
int SERIAL_buff_size;

// at first use stty -F /dev/ttySx to set the attributes
// rather than setting them here
// matt 2001


int SERIAL_close(void){
	if(SERIAL_set==1){
		close(SERIAL_in);
		delete[] SERIAL_buffer;
		cout<<"SERIAL_MAIN: closed\n";
		return SERIAL_OK;
		}
	else{
		cout<<"SERIAL_MAIN: close called but interface was not open\n";
		return SERIAL_NOOPEN;
		}
	}

int SERIAL_open(char *tty,int buff_size){
char device[100];
// determine if the device is absolute, if not append /dev/
speed_t sp;

	SERIAL_buff_size=buff_size;
	if((SERIAL_buff_size<0)||(SERIAL_buff_size>SERIAL_MAX_BUFFER)){
		cout<<"SERIAL_MAIN: Buffer size error\n";
		exit(-3);
		}
	if(tty[0]=='/'){
		sprintf(device,"%s",tty);
		}
	else{
		sprintf(device,"/dev/%s",tty);
		}
	cout<<"SERIAL_MAIN: Attempting to open serial device "<<device<<"\n";
// open the serial device for reading
	if((SERIAL_in=open(device,O_RDWR))<0){
		cout<<"SERIAL_MAIN: could not open device "<<device<<endl;
		cout<<"The error is "<<SERIAL_in<<endl;
		return SERIAL_NOOPEN;
		}
	cout<<"SERIAL_MAIN: open ok\n";
	cout<<"SERIAL_MAIN: getting attributes\n";
	SERIAL_tp.c_cflag=CS8|CLOCAL;	
	SERIAL_tp.c_iflag=IGNBRK|IGNCR|IXANY;	
	cfsetispeed(&SERIAL_tp,B19200);
	cout<<"SERIAL: trying to set attributes (inc speed)\n";
	if(tcsetattr(SERIAL_in,TCSANOW,&SERIAL_tp)<0){
		cout<<"SERIAL_MAIN: Couldn't set the attributes\n";
		return SERIAL_NOATTR;
		}
	cout<<"SERIAL: done\n";
// allocate buffer static so memory allocation is avoided in loop
	SERIAL_buffer=new unsigned char[buff_size*2+1];
	if(SERIAL_buffer==NULL){
		cout<<"SERIAL: Unable to allocate buffer size "<<buff_size<<" bytes exiting \n";
		return SERIAL_NOMEM;
		}
	SERIAL_set=1;
	return SERIAL_OK;
	}

int SERIAL_get(char *buff){
int	i,j;
	tcflush(SERIAL_in,TCIFLUSH);
	while(read(SERIAL_in,(void *)SERIAL_buffer,SERIAL_buff_size*2)!=SERIAL_buff_size*2);
	i=0;
	while((SERIAL_buffer[i]<128)&&(i<SERIAL_buff_size*2))i++;
	if(i>SERIAL_buff_size){
		// data has no 8 bit flag just return the first N bytes
		memcpy(buff,SERIAL_buffer,SERIAL_buff_size);
		return SERIAL_ASYNC;
		}
	// the 8th bit sync has been found
	SERIAL_buffer[i]-=128;
	memcpy(buff,SERIAL_buffer+i,SERIAL_buff_size);
	return SERIAL_OK;
	}

