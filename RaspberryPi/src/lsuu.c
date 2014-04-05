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
#include <dirent.h>

#include "serial.h"


#define UUGEAR_DEVICE_PREFIX1		"ttyUSB"
#define UUGEAR_DEVICE_PREFIX2		"ttyACM"
#define UUGEAR_DEVICE_BAUD_RATE		115200
#define UUGEAR_ID_PREFIX			"UUGear-"
#define UUGEAR_ID_MAX_LENGTH		1024
#define UUGEAR_RESPONSE_START_CHAR	'\t'
#define UUGEAR_RESPONSE_END_STRING	":)"


int startsWith(const char *str, const char *prefix)
{
    size_t lenpre = strlen(prefix),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(prefix, str, lenpre) == 0;
}

int endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int openDevice(struct dirent *dir)
{
	// open the device for id query
	char buf[UUGEAR_ID_MAX_LENGTH + 1];
	char devicePath[32];
	strcpy (devicePath, "/dev/");
	strcat (devicePath, dir->d_name);
	int fd = serialOpen (devicePath, UUGEAR_DEVICE_BAUD_RATE);
	if (fd > 0)
	{
		// write "get ID" command to serial
		char getIdCmd[] = { 0x55, 0x30, 0x01, 0x0D, 0x0A, 0x00};
		serialWriteString (fd, getIdCmd);
		serialFlush (fd);
		
		// read response from serial
		int i = 0;
		int j = 0;
		int head = 0;
		int callId = 0;
		int chr;
		memset (buf, 0, sizeof buf);
		while ((chr = serialReadChar (fd)) > 0)
		{
			if (head == 0 && chr == UUGEAR_RESPONSE_START_CHAR)
			{
				head = chr;
			}
			else if (head == UUGEAR_RESPONSE_START_CHAR)
			{
				if (callId == 0)
				{
					callId = chr;
				}
				else
				{
					buf[j ++] = (chr & 0xFF);
				}
			}
		}
		
		// output the fixed device id and device name
		if (startsWith (buf, UUGEAR_ID_PREFIX))
		{
			char *end = strstr (buf, UUGEAR_RESPONSE_END_STRING);
			if (end != NULL)
			{
				buf[end - buf] = 0;
			}
			printf ("%s\t(%s)\n", buf, devicePath);
			return fd;
		}
	}
	return 0;
}


int main(int argc, char **argv)
{
	int found = 0;
	
	DIR *baseDir;
	struct dirent *dir;
	
	printf ("--------------------------------------------------\n");
	
	baseDir = opendir ("/dev");
	if (baseDir)
	{
		while ((dir = readdir (baseDir)) != NULL)
		{
			if (dir->d_type == DT_CHR)
  			{
  				if (startsWith (dir->d_name, UUGEAR_DEVICE_PREFIX1) || startsWith (dir->d_name, UUGEAR_DEVICE_PREFIX2))
  				{
  					int fd1 = 0;
  					int fd2 = 0;
  					if (fd1 = openDevice(dir))
  					{
						found ++;
  					}
  					else
  					{
  						/* newly connected device may need a second shoot */
  						sleep (1);
  						if (fd2 = openDevice(dir))
  						{
  							found ++;	
  						}						
  					}
  					// close the device
					if (fd1) serialClose (fd1);
					if (fd2) serialClose (fd2);
				}
			}
		}
		closedir (baseDir);
	}
	
	printf ("--------------------------------------------------\n%d device(s) found.\n", found);
	
	return(0);
}