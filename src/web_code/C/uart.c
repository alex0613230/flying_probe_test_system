#include <sys/types.h>       /* basic system data types */
#include <sys/time.h>        /* timeval{} for select() */
#include <time.h>            /* timespec{} for pselect() */
#include <errno.h>
#include <fcntl.h>           /* for nonblocking */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>

#define Serial_7O1	1
#define Serial_7E1	2
#define Serial_7N2	3
#define Serial_8N1	4
#define Serial_8O1	5
#define Serial_8E1	6
#define Serial_8N2	7

// Used Type Definitions
typedef unsigned char 	BYTE;
typedef unsigned short	WORD;

#define FALSE	0
#define TRUE	1
#define SIZE	200

#define MAXLINE	1000	// max text line length

#ifndef HAVE_BZERO
#define bzero(ptr,n)        memset (ptr, 0, n)
#endif

// Definitions for Modbus Frame
typedef struct
{
	BYTE	slave_address;
	BYTE	function;
	WORD	start_address;
	WORD	no_of_points;
	BYTE	byte_count;
	WORD	reg_values[SIZE];
	BYTE	lrc;
	BYTE	exception_code;
} FRAME;

BYTE to_binary(BYTE firstbyte, BYTE secondbyte)
{
	BYTE	po = 0;
	
	if (firstbyte <= '9')
		po = (firstbyte - '0');
	else	po = (firstbyte - 'A') + 10;
	
	po = po << 4;
		
	if (secondbyte <= '9')
		po |= (secondbyte - '0');
	else	po |= (secondbyte - 'A') + 10;

	return po;
}

void to_hexascii(BYTE tmp[2], BYTE data)
{
	BYTE	p;
	
	p = (data & 0XF0) >> 4;
	if (p <= 9)
		tmp[0] = '0' + p;
	else	tmp[0] = 'A' + p - 10;
	
	p = data & 0X0F;
	if (p <= 9)
		tmp[1] = '0' + p;
	else	tmp[1] = 'A' + p - 10;
}

void strtoupper(char *ptr)
{
	while (*ptr != '\0')
	{
		if ((*ptr >= 'a') && (*ptr <= 'z'))
			*ptr -= 32;
		
		ptr ++;
	}
}

int open_and_initialize_device(char *device, int baudrate, int encoding)
{
	struct termios	newtio;
	int	fd;			// file descriptor for serial port
	struct flock fl;		// definition for file lock
		
	// open the device to be non-blocking (read will return immediatly) and locked
	fl.l_type = F_WRLCK; // F_RDLCK, F_WRLCK, F_UNLCK 
	fl.l_whence = SEEK_SET; // SEEK_SET, SEEK_CUR, SEEK_END
	fl.l_start = 0; // Offset from l_whence 
	fl.l_len = 0; // length，0 = to EOF
	fl.l_pid = getpid(); // our PID
	
	fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
		return fd;
	
	fcntl(fd, F_SETLKW, &fl);  // lock the device
		
	// set new port settings for canonical input processing
	bzero(&newtio, sizeof(newtio));

	// BAUDRATE: Set bps rate
	// CS8     : 8 data bits
	// CLOCAL  : local connection, no modem control
	// CREAD   : enable receiving characters
	switch(baudrate)
	{
		case 1200: newtio.c_cflag = B1200; break;
		case 2400: newtio.c_cflag = B2400; break;
		case 4800: newtio.c_cflag = B4800; break;
		case 9600: newtio.c_cflag = B9600; break;
		case 19200: newtio.c_cflag = B19200; break;
		case 38400: newtio.c_cflag = B38400; break;
		case 57600: newtio.c_cflag = B57600; break;
		case 115200: newtio.c_cflag = B115200; break;
		default: newtio.c_cflag = B38400; break;
	}
	
	switch(encoding)
	{
		case Serial_7O1: // 7O1
			newtio.c_cflag |= CS7 | CLOCAL | CREAD | PARENB | PARODD;
			newtio.c_cflag &= ~CSTOPB;
			break;
		case Serial_7E1: // 7E1
			newtio.c_cflag |= CS7 | CLOCAL | CREAD | PARENB;
			newtio.c_cflag &= ~CSTOPB;		// 1 stop bit
			newtio.c_cflag &= ~PARODD;
			break;
		case Serial_7N2: // 7N2
			newtio.c_cflag |= CS7 | CLOCAL | CREAD;
			newtio.c_cflag |= CSTOPB;		// 2 stop bits
			break;
		case Serial_8N1: // 8N1
			newtio.c_cflag |= CS8 | CLOCAL | CREAD;
			break;
		case Serial_8O1: // 8O1
			newtio.c_cflag |= CS8 | CLOCAL | CREAD | PARENB | PARODD;
			newtio.c_cflag &= ~CSTOPB;
			break;
		case Serial_8E1: // 8E1
			newtio.c_cflag |= CS8 | CLOCAL | CREAD | PARENB;
			newtio.c_cflag &= ~CSTOPB;		// 1 stop bit
			newtio.c_cflag &= ~PARODD;
			break;
		case Serial_8N2: // 8N2
			newtio.c_cflag |= CS8 | CLOCAL | CREAD;
			newtio.c_cflag |= CSTOPB;		// 2 stop bits
			break;
		default: // 8N1, Serial_8N1
			newtio.c_cflag |= CS8 | CLOCAL | CREAD | PARENB;
			newtio.c_cflag &= ~CSTOPB;		// 1 stop bit
			newtio.c_cflag &= ~PARODD;
			break;
	}

	newtio.c_iflag = IGNPAR;	// IGNPAR  : ignore bytes with parity errors
	newtio.c_oflag = 0;		// Raw output
	newtio.c_lflag = 0;		// set input mode (non-canonical, no echo,...)
	newtio.c_cflag &= ~CRTSCTS;	// disable hardware flow control

	// initialize all control characters 
	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

	// clean the serial port buffer and activate the settings for the port 
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
}

int my_serial_write(int fd, BYTE *vptr, int n)
{
        int nleft;
        int nwritten;
        BYTE *ptr;

        ptr = vptr;
        nleft = n;
        while (nleft > 0)
        {
                if ((nwritten = write(fd, ptr, nleft)) <= 0)
                {
                        if (nwritten < 0 && errno == EINTR)
                                nwritten = 0;   // and call write() again
                        else    return (-1);    // error
                }

                nleft -= nwritten;
                ptr += nwritten;
        }

        return (n);
}

static int read_cnt;
static BYTE *read_ptr;
static BYTE read_buf[MAXLINE];

ssize_t my_serial_read(int fd, BYTE *ptr)
{
	fd_set	readfs;			// file descriptor set
	int	res_select;		// for select()
	struct timeval timeout;		// for select() timeout
	
        if (read_cnt <= 0)
        {
		// set testing for source
		FD_ZERO(&readfs);
		FD_SET(fd, &readfs);
		timeout.tv_usec = (1000000) % 1000000ul;  /* useconds */
		timeout.tv_sec  = (1000000) / 1000000ul;  /* seconds */
		res_select = select(fd + 1, &readfs, NULL, NULL, &timeout);
		
		if ((res_select > 0) && (FD_ISSET(fd, &readfs)))
		{
			again:
			if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
			{
				if (errno == EINTR)
					goto again;

				return (-1);
			}
			else if (read_cnt == 0)
				return (0);

			read_ptr = read_buf;
		}
		else	// time out
		{
			return (-1);
		}
	}

        read_cnt--;
        *ptr = *read_ptr++;

        return (1);
}

// return buffer in Binary format
WORD serial_readline_ASCII(int fd, BYTE *vptr, int maxlen, FRAME *fr)
{
        WORD	size, i, j;
	int	rc;
	BYTE    c, *ptr, tmp[4];
	BYTE	t1, t2;
	
	size = 0;
        ptr = vptr;
	
	fr->slave_address = 0;
	fr->function = 0;
	fr->start_address = 0;
	fr->no_of_points = 0;
	fr->byte_count = 0;
	fr->lrc = 0;
	fr->exception_code = 0;
	
	// get leading character and check
	if ((rc = my_serial_read(fd, &c)) == 1)
	{
		if (c != ':') return (0);
	}
	else    return (0);
	
	// get slave address, 1 bytes
        for (i = 0; i < 2; i++)
        {
                if ((rc = my_serial_read(fd, &c)) == 1)
                        tmp[i] = c;
                else    return (0);
        }
	
	t1 = to_binary(tmp[0], tmp[1]);
	fr->slave_address = t1;
	*ptr ++ = t1;
	size ++;
	
	// get function, 1 bytes
	for (i = 0; i < 2; i++)
        {
                if ((rc = my_serial_read(fd, &c)) == 1)
			tmp[i] = c;
                else    return (0);
        }
	
	t1 = to_binary(tmp[0], tmp[1]);
	fr->function = t1;
	*ptr ++ = t1;
	size ++;
	
	if ((fr->function & 0X80) != 0)	// Exception
	{
		for (i = 0; i < 2; i++) // get exception code
		{
			if ((rc = my_serial_read(fd, &c)) == 1)
				tmp[i] = c;
			else    return (0); 
		}
		
		t1 = to_binary(tmp[0], tmp[1]);
		fr->exception_code = t1;
		*ptr ++ = t1;
		size ++;
	}
	else
	{
		switch (fr->function)
		{
			case 0X03:	// for read Holding Registers
				// get Byte count, 1 bytes
				for (i = 0; i < 2; i++)
				{
					if ((rc = my_serial_read(fd, &c)) == 1)
						tmp[i] = c;
					else    return (0);
				}
			
				t1 = to_binary(tmp[0], tmp[1]);
				fr->byte_count = t1;
				*ptr ++ = t1;
				size ++;

				// get 16-bit Registers' values
				for (i = 0; i < fr->byte_count / 2; i++)
				{					
					for (j = 0; j < 4; j++)
					{
						if ((rc = my_serial_read(fd, &c)) == 1)
							tmp[j] = c;
						else    return (0);
					}
					
					t1 = to_binary(tmp[0], tmp[1]);
					*ptr ++ = t1;
					size ++;
					t2 = to_binary(tmp[2], tmp[3]);
					*ptr ++ = t2;
					size ++;
					
					fr->reg_values[i] = (WORD) t1 * 256 + t2;
				
					if (size >= maxlen - 6)
						return (0);	// exceed the max. length
				}
				break;
				
			case 0X10:	// for Preset Multiple Registers
				// get Starting address of registers, 2 bytes
				for (i = 0; i < 4; i++)
				{
					if ((rc = my_serial_read(fd, &c)) == 1)
						tmp[i] = c;
					else    return (0);
				}
				
				t1 = to_binary(tmp[0], tmp[1]);
				*ptr ++ = t1;
				size ++;
				t2 = to_binary(tmp[2], tmp[3]);
				*ptr ++ = t2;
				size ++;
					
				fr->start_address = (WORD) t1 * 256 + t2;
						
				// get no. of points, 2 bytes
				for (i = 0; i < 4; i++)
				{
					if ((rc = my_serial_read(fd, &c)) == 1)
						tmp[i] = c;
					else    return (0);
				}
				
				t1 = to_binary(tmp[0], tmp[1]);
				*ptr ++ = t1;
				size ++;
				t2 = to_binary(tmp[2], tmp[3]);
				*ptr ++ = t2;
				size ++;
				
				fr->no_of_points = (WORD) t1 * 256 + t2;
								
				break;
		}
	}
	
	// get '.', 1 bytes
	for (i = 0; i < 1; i++)
        {
                if ((rc = my_serial_read(fd, &c)) == 1)
			tmp[i] = c;
                else    return (0);        /* error, errno set by read() */
        }
	
        return (size);
}

int send_frame_to_device(int fd, BYTE *job, WORD length)
{
	WORD	i, j;
	BYTE	data[300], tmp[3];
	BYTE	lrc;
	
	// lrc = gen_lrc(job, length);
	
	j = 0;
	data[j ++] = ':';
	
	for (i = 0; i < length; i ++)
	{
		to_hexascii(tmp, job[i]);
		data[j ++] = tmp[0];
		data[j ++] = tmp[1];
	}
	
	/*
	// append LRC
	to_hexascii(tmp, lrc);
	data[j ++] = tmp[0];
	data[j ++] = tmp[1];
	
	// append 0X0D, 0X0A
	data[j ++] = 0X0D;
	data[j ++] = 0X0A;
	*/	
		
	// append '.'
	data[j ++] = '.';
	
	#ifdef DEBUG
	printf("send_frame_to_device_ASCII(): Frame= [");
	for (i = 0; i < j; i ++)
		printf("%c", data[i]);
	printf("]\n");
	#endif

	i = my_serial_write(fd, data, j);
	
	if (j != i)
		return FALSE;
	return TRUE;
}

// For commands: 0X03: Read Holding Registers
void gen_03_frame(BYTE *msg, WORD *size, FRAME *fr)
{
	msg[0] = fr->slave_address;			// slave address
	msg[1] = 0X03;					// function
	msg[2] = (BYTE) (fr->start_address / 256);	// start address
	msg[3] = (BYTE) (fr->start_address % 256);
	msg[4] = (BYTE) (fr->no_of_points / 256);	// no. of points
	msg[5] = (BYTE) (fr->no_of_points % 256);
	
	*size = 6;
}

// For commands: 0X10: Preset Multiple Registers
void gen_10_frame(BYTE *msg, WORD *size, FRAME *fr)
{
	WORD	i, j;
	
	j = 0;
	msg[j ++] = fr->slave_address;			// slave address
	msg[j ++] = 0X10;				// function
	msg[j ++] = (BYTE) (fr->start_address / 256);	// start address
	msg[j ++] = (BYTE) (fr->start_address % 256);
	msg[j ++] = (BYTE) (fr->no_of_points / 256);	// no. of points
	msg[j ++] = (BYTE) (fr->no_of_points % 256);
	msg[j ++] = (BYTE) (fr->no_of_points * 2);	// Byte Count

	for (i = 0; i < fr->no_of_points; i ++)
	{
		msg[j ++] = (BYTE) (fr->reg_values[i] / 256);
		msg[j ++] = (BYTE) (fr->reg_values[i] % 256);
	}
	
	*size = j;
}

int main(int argc, char **argv)
{
	FRAME	frame1, frame2;
	FILE	*in;
	BYTE	command[MAXLINE], request[MAXLINE], response[MAXLINE];
	int	serial_fd;
	BYTE	*p, len, i;
	
	WORD	size;
	int	res;
	
	if (argc != 2)
	{
		printf("Usage: %s Modbus_command\n", argv[0]);
		printf("Example of Modbus command: 010300000001\n");
		exit(0);
	}
	
	strcpy(command, argv[1]);
	strtoupper(command);
	p = command;
	len = strlen(command);
	
	// parsing and checking input command
	if (len < 12)
	{
		printf("Command error: %s\n", command);
		exit(0);
	}
	
	frame1.slave_address = to_binary(*p, *(p + 1));		
	p = p + 2;
	
	frame1.function = to_binary(*p, *(p + 1));		
	p = p + 2;
	
	if ((frame1.function != 0X03) && (frame1.function != 0X10))
	{
		printf("Function error: %s\n", command);
		exit(0);
	}
	
	frame1.start_address = to_binary(*p, *(p + 1));		
	p = p + 2;
	frame1.start_address = frame1.start_address * 256 + to_binary(*p, *(p + 1));	
	p = p + 2;
	
	frame1.no_of_points = to_binary(*p, *(p + 1));		
	p = p + 2;
	frame1.no_of_points = frame1.no_of_points * 256 + to_binary(*p, *(p + 1));
	p = p + 2;
	
	len = len - 12;
	
	if (frame1.function == 0X10)
	{
		frame1.byte_count = to_binary(*p, *(p + 1));		
		p = p + 2;
		len = len - 2;
		if (len < frame1.byte_count)
		{
			printf("Command error: %s\n", command);
			exit(0);
		}
		
		// read values of registers
		for (i = 0; i < frame1.no_of_points; i ++)
		{
			frame1.reg_values[i] = to_binary(*p, *(p + 1));	
			p = p + 2;
			frame1.reg_values[i] = frame1.reg_values[i] * 256 + to_binary(*p, *(p + 1));
			p = p + 2;
		}
	}
	
	// open device, 115200 bps, 8N1
	serial_fd = open_and_initialize_device("/dev/ttyUSB0", 115200, Serial_8N1);
	if (serial_fd < 0)
	{
		printf("Serial port open error: %s\n", "/dev/ttyUSB0");
		exit(0);
	}
	
	// generate frame
	switch (frame1.function)
	{
		case 0X03: gen_03_frame(request, &size, &frame1); break;
		case 0X10: gen_10_frame(request, &size, &frame1); break;
	}
	
	bzero(response, MAXLINE);
	tcflush(serial_fd, TCIFLUSH); // clear input buffer
	
	// send frame to device
	res = send_frame_to_device(serial_fd, request, size);
	if (res == FALSE)
	{
		printf("Serial port writing error: %s\n", "/dev/ttyUSB0");
		exit(0);
	}
	
	// read device response Frame
	res = serial_readline_ASCII(serial_fd, response, MAXLINE - 1, &frame2);
	
	// check response
	if ((res < 3) || (res > 512))
	{	
		printf("Frame reading error...\n");
		exit(0);
	}
	
	//printf("Response from slave device: [");
	for (i = 0; i < res; i ++)
		printf("%02X", response[i]);
	printf("\n");
	
	/*
	if (! modbus_lrc_correct(response, res))
	{
		printf("LRC Error in response...\n");
		exit(0);
	}
	*/

	if (frame1.slave_address != frame2.slave_address)
	{
		printf("Slave address Error in response...\n");
		exit(0);
	}
	
	close(serial_fd);
}