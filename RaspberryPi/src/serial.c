/*
 * UUGear Solution: extend your Raspberry Pi with Arduino
 *
 * Author: Shawn (shawn@uugear.com)
 *
 * Copyright (c) 2014 UUGear s.r.o.
 *
 ***********************************************************************
 *  This file is part of UUGear Solution: 
 *  http://www.uugear.com/?page_id=50
 *  
 *  UUGear Solution is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  UUGear Solution is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with UUGear Solution.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "serial.h"


int serialOpen (char *device, int baud)
{
	int fd = open (device, O_RDWR | O_NOCTTY | O_SYNC);
	
	struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("Error %d from tcgetattr\n", errno);
        return -1;
    }

	speed_t baudRate;
	switch (baud)
	{
		case 9600:
			baudRate = B9600;
			break;
		case 19200:
			baudRate = B19200;
			break;
		case 38400:
			baudRate = B38400;
			break;
		case 57600:
			baudRate = B57600;
			break;
		case 115200:
			baudRate = B115200;
			break;
		default:
			printf("Unsupported baud rate: %d\n", baud);
	  		return -1;
	}
    cfsetospeed (&tty, baudRate);
    cfsetispeed (&tty, baudRate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag &= ~HUPCL;   /* disable hangup-on-close to avoid auto reset */
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf ("Error %d setting term attributes\n", errno);
        return -1;
    }
    
	return fd;
}


void serialSetBlocking (int fd, int block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) == 0)
	{
		tty.c_cc[VMIN]  = block ? 1 : 0;
		tty.c_cc[VTIME] = 10;
		if (tcsetattr (fd, TCSANOW, &tty) != 0)
		{
			printf ("Error %d setting term attributes\n", errno);
		}
	}
	else
	{
		printf ("Error %d from tggetattr\n", errno);
	}
}


void serialWriteChar (int fd, unsigned char c)
{
	write (fd, &c, 1);
}


void serialWriteString (int fd, char *s)
{
	write (fd, s, strlen (s));
}

void serialWriteData (int fd, char *buf, int len)
{
	write (fd, buf, len);
}

void serialFlush (int fd)
{
	tcflush(fd, TCIOFLUSH);
}


int serialReadChar (int fd)
{
	char c;
	if(read (fd, &c, 1) == 1)
	{
		return ((int)c) & 0xFF;
	}
	return -1;
}


void serialClose (int fd)
{
	close(fd);
}