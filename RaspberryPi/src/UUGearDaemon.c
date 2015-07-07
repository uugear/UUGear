/*
 * UUGear Solution: extend your Raspberry Pi with Arduino
 *
 * Author: Shawn (shawn@uugear.com)
 *
 * Copyright (c) 2014 UUGear s.r.o.
 *
 ***********************************************************************
 *  This file is part of UUGear Solution: 
 *  http://www.uugear.com/uugear-rpi-arduino-solution/
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <mqueue.h>
#include <pthread.h>
#include <dirent.h>

#include "UUGearDaemon.h"
#include "serial.h"

/* array to store all client state (1=running) */
int clientStatus[256];

static void child_handler(int signum)
{
    switch (signum) {
    case SIGALRM: exit(EXIT_FAILURE); break;
    case SIGUSR1: exit(EXIT_SUCCESS); break;
    case SIGCHLD: exit(EXIT_FAILURE); break;
    }
}


static void daemonize(const char *lockfile)
{
    pid_t pid, sid, parent;
    int lfp = -1;

    /* Already a daemon */
    if (getppid () == 1) return;

    /* Create the lock file as the current user */
    if (lockfile && lockfile[0]) {
        lfp = open (lockfile,O_RDWR|O_CREAT,0640);
        if (lfp < 0) {
            syslog (LOG_ERR, "Could not create lock file %s, code=%d (%s)",
                    lockfile, errno, strerror (errno));
            exit (EXIT_FAILURE);
        }
    }

    /* Drop user if there is one, and we were run as root */
    if (getuid () == 0 || geteuid () == 0) {
        struct passwd *pw = getpwnam (RUN_AS_USER);
        if (pw) {
            syslog (LOG_NOTICE, "Run daemon as user: " RUN_AS_USER);
            setuid (pw->pw_uid);
        }
    }

    /* Trap signals that we expect to recieve */
    signal (SIGCHLD,child_handler);
    signal (SIGUSR1,child_handler);
    signal (SIGALRM,child_handler);

    /* Fork off the parent process */
    pid = fork ();
    if (pid < 0) {
        syslog (LOG_ERR, "Could not fork daemon, code=%d (%s)",
                errno, strerror (errno));
        exit (EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {

        /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
           for two seconds to elapse (SIGALRM).  pause() should not return. */
        alarm (2);
        pause ();

        exit (EXIT_FAILURE);
    }

    /* At this point we are executing as the child process */
    parent = getppid ();

    /* Cancel certain signals */
    signal (SIGCHLD,SIG_DFL); /* A child process dies */
    signal (SIGTSTP,SIG_IGN); /* Various TTY signals */
    signal (SIGTTOU,SIG_IGN);
    signal (SIGTTIN,SIG_IGN);
    signal (SIGHUP, SIG_IGN); /* Ignore hangup signal */
    signal (SIGTERM,SIG_DFL); /* Die on SIGTERM */

    /* Change the file mode mask */
    umask (0);

    /* Create a new SID for the child process */
    sid = setsid ();
    if (sid < 0) {
        syslog (LOG_ERR, "unable to create a new session, code %d (%s)",
                errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    /* Change the current working directory.  This prevents the current
       directory from being locked; hence not being able to remove it. */
    if ((chdir("/")) < 0) {
        syslog (LOG_ERR, "unable to change directory to %s, code %d (%s)",
                "/", errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    /* Redirect standard files to /dev/null */
    freopen ("/dev/null", "r", stdin);
    freopen ("/dev/null", "w", stdout);
    freopen ("/dev/null", "w", stderr);

    /* Notify the parent process that daemonize is done */
    kill (parent, SIGUSR1);
}


int startsWith(const char *str, const char *pre)
{
    size_t lenpre = strlen (pre),
           lenstr = strlen (str);
    return lenstr < lenpre ? 0 : strncmp (pre, str, lenpre) == 0;
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


void responseToClient(int clientId, char *resp)
{
	char queueName[MAX_QUEUE_NAME_LENGTH];
	sprintf (queueName,"%s%d", RESPONSE_QUEUE_PREFIX, clientId);
	mqd_t mq = mq_open (queueName, O_WRONLY);
	if ((mqd_t)-1 != mq)
	{
		mq_send (mq, resp, strlen (resp), 0);
		mq_close (mq);
	}
}


void *deviceOpener(void *arg)
{
	DeviceOpen *devOpen = (DeviceOpen *)arg;
	syslog (LOG_INFO, "Started device opener for: %s (%s)", devOpen->id, devOpen->devName);
	int i = 0;
	int j = 0;
	int head = -1;
	int clientId = -1;
	int chr;
	char buf[UUGEAR_ID_MAX_LENGTH + 1];
	memset (buf, 0, sizeof buf);
	
	// read response from serial
	while ((chr = serialReadChar (devOpen->fd)) > 0)
	{
		if (head == -1 && chr == UUGEAR_RESPONSE_START_CHAR)
		{
			head = chr;
		}
		else if (head == UUGEAR_RESPONSE_START_CHAR)
		{
			if (clientId == -1)
			{
				clientId = chr;
			}
			else
			{
				buf[j ++] = (chr & 0xFF);
				if (endsWith (buf, UUGEAR_RESPONSE_END_STRING))
				{
					break;
				}
			}
		}
	}
	
	// extract device name
	if (startsWith (buf, UUGEAR_ID_PREFIX))
	{
		char *end = strstr (buf, UUGEAR_RESPONSE_END_STRING);
		if (end != NULL)
		{
			buf[end - buf] = 0;
		}
		syslog (LOG_INFO, "Receive device id: %s", buf);
	  	if (strcmp(devOpen->id, buf) == 0)
	  	{
	  		syslog (LOG_INFO, "Device found, send fd=%d to clientId=%d", devOpen->fd, clientId);
	  		
	  		// device found, send back the fd
	  		char fdBuf[16];
			memset (fdBuf, 0, sizeof fdBuf);
			sprintf (fdBuf,"%d", devOpen->fd);
			
			char queueName[MAX_QUEUE_NAME_LENGTH];
			sprintf (queueName,"%s%d", RESPONSE_QUEUE_PREFIX, clientId);
			mqd_t mq = mq_open (queueName, O_WRONLY);
			if ((mqd_t)-1 != mq)
			{
	  			mq_send (mq, fdBuf, strlen (fdBuf), 0);
	  			
	  			// set client status
	  			clientStatus[clientId] = 1;
	  			
	  			// loop to keep reading response for other commands
	  			while (clientStatus[clientId])
	  			{
	  				// read response from serial
	  				int k = 0;
	  				int hd = -1;
	  				int cid = -1;
	  				memset (buf, 0, sizeof buf);
					while ((chr = serialReadChar (devOpen->fd)) > 0)
					{
						if (hd == -1 && chr == UUGEAR_RESPONSE_START_CHAR)
						{
							hd = chr;
						}
						else if (hd == UUGEAR_RESPONSE_START_CHAR)
						{
							if (cid == -1)
							{
								cid = chr;
							}
							else
							{
								buf[k ++] = (chr & 0xFF);
								if (endsWith (buf, UUGEAR_RESPONSE_END_STRING))
								{
									break;
								}
							}
						}
					}
					if (k > 0)
					{
						char *end = strstr (buf, UUGEAR_RESPONSE_END_STRING);
						if (end != NULL)
						{
							buf[end - buf] = 0;
						}
						syslog (LOG_INFO, "Received response from %s(fd=%d): %s", devOpen->devName, devOpen->fd, buf);
						
						if (cid == clientId)
						{
							// send back the response to client
							mq_send (mq, buf, strlen (buf), 0);
						}
						else
						{
							// If cid != clientId, it could be one of two cases: either trying to open the same device twice,
							// or another opener broadcasts CMD_GET_DEVICE_ID command to find a different device.
							// It seems not possible to distinguish these two cases, so the best way is not to response anything.
							syslog (LOG_INFO, "Receive response from another client, ignoring...");
						}
					}
	  			}
	  			
	  			// cleanup
	  			serialClose (devOpen->fd);
	  			mq_close (mq);
	  		}
	  		else
	  		{
	  			syslog (LOG_INFO, "Can not open message queue: %s", queueName);	
	  		}
	  	}
	  	else
	  	{
	  		serialClose(devOpen->fd);
	  	}	
	}
	syslog (LOG_INFO, "Terminated device opener for: %s (fd=%d)", devOpen->id, devOpen->fd);
	free (devOpen->id);
	free (devOpen->devName);
	free (devOpen);
	return NULL;
}


void openDeviceById(int clientId, char *id)
{
	syslog (LOG_INFO, "Try to open device as client=%d by id=%s", clientId, id);
	DIR *baseDir;
	struct dirent *dir;
	baseDir = opendir ("/dev");
	if (baseDir)
	{
		while ((dir = readdir (baseDir)) != NULL)
		{
			if (dir->d_type == DT_CHR)
  			{
  				if (startsWith (dir->d_name, UUGEAR_DEVICE_PREFIX1) || startsWith (dir->d_name, UUGEAR_DEVICE_PREFIX2))
  				{
  					// open the device for id query
					char devicePath[32];
					strcpy (devicePath, "/dev/");
					strcat (devicePath, dir->d_name);
  					int fd = serialOpen (devicePath, UUGEAR_DEVICE_BAUD_RATE);
  					if (fd > 0)
  					{
  						syslog (LOG_INFO, "Opened serial deive: %s (fd=%d)\n", dir->d_name, fd);
  						// start a device opener
  						DeviceOpen *devOpen;
  						devOpen = malloc (sizeof(*devOpen));
  						devOpen->fd = fd;
  						devOpen->devName = malloc (strlen (dir->d_name) + 1);
  						strcpy (devOpen->devName, dir->d_name);
  						devOpen->id = malloc (strlen(id) + 1);
  						strcpy (devOpen->id, id);
  						pthread_t doThread;
  						pthread_create (&doThread, NULL, deviceOpener, (void *)devOpen);
  						
  						// write "get ID" command to serial
  						char getIdCmd[] = { COMMAND_START_CHAR, CMD_GET_DEVICE_ID, (char)(clientId&0xFF), COMMAND_END_CHAR1, COMMAND_END_CHAR2, 0x00 };
  						serialWriteString (fd, getIdCmd);
  						serialFlush (fd);
  					}
  					else
  					{
  						syslog (LOG_INFO, "Could not open serial deive: %s\n", dir->d_name);
  					}
				}
			}
		}
		closedir (baseDir);
	}
}

void sendCommandWithoutParameter(char cmd, int clientId, int targetFd)
{
	syslog (LOG_INFO, "Send command: cmd=0x%02x, clientId=%d, fd=%d", cmd, clientId, targetFd);
	char command[] = { COMMAND_START_CHAR, cmd, (char)(clientId & 0xFF), COMMAND_END_CHAR1, COMMAND_END_CHAR2, 0x00 };
	serialWriteString (targetFd, command);
}

void sendCommand(char cmd, int clientId, int targetFd, int pin)
{
	syslog (LOG_INFO, "Send command: cmd=0x%02x, clientId=%d, fd=%d, pin=%d", cmd, clientId, targetFd, pin);
	char command[] = { COMMAND_START_CHAR, cmd, (char)(pin & 0xFF), (char)(clientId & 0xFF), COMMAND_END_CHAR1, COMMAND_END_CHAR2, 0x00 };
	serialWriteString (targetFd, command);
}

void sendCommandWithParameter(char cmd, int clientId, int targetFd, int pin, int parameter)
{
	syslog (LOG_INFO, "Send command: cmd=0x%02x, clientId=%d, fd=%d, pin=%d, parameter=%d", cmd, clientId, targetFd, pin, parameter);
	char command[] = { COMMAND_START_CHAR, cmd, (char)(pin & 0xFF), (char)(parameter & 0xFF), (char)(clientId & 0xFF), COMMAND_END_CHAR1, COMMAND_END_CHAR2, 0x00 };
	serialWriteString (targetFd, command);
}


int main(int argc, char **argv)
{
	/* initialize the logging system */
    openlog (DAEMON_NAME, LOG_PID, LOG_LOCAL5);
    syslog (LOG_INFO, "Starting UUGear daemon...");

    /* daemonize */
    daemonize ("/var/lock/" DAEMON_NAME);

    /* we are UUGear daemon now */
	
    /* create the message queue */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    mqd_t mq = mq_open (REQUEST_QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    ASSERT_TRUE ((mqd_t)-1 != mq);

	/* main loop */
    char buffer[MAX_MSG_SIZE + 1];
    int must_stop = 0;
    while (!must_stop) {
        ssize_t bytes_read;

        /* receive message */
        bytes_read = mq_receive (mq, buffer, MAX_MSG_SIZE, NULL);
        ASSERT_TRUE (bytes_read >= 0);

        buffer[bytes_read] = '\0';
        
        if (strlen (buffer) > 0)
        {
        	syslog (LOG_INFO, "Received: %s", buffer);
        	
			char *parts[MAX_MSG_PARTS];
			int count = 0;
			parts[count] = strtok (buffer, MSG_PART_SEPARATOR);
			while (parts[count] != NULL)
			{
			   parts[++count] = strtok (NULL, MSG_PART_SEPARATOR);
			}
			if (count > 0)
			{
				int cmd = atoi (parts[0]);
				int clientId = count > 1 ? (atoi (parts[1]) & 0xFF) : -1;
				int targetFd = count > 2 ? (atoi (parts[2]) & 0xFF) : -1;
				switch (cmd)
				{
					case MSG_EXIT:
						must_stop = 1;
						memset (clientStatus, 0, sizeof clientStatus);
						break;
					case MSG_GET_DEVICE:
						openDeviceById(clientId, count > 3 ? parts[3] : "");
						break;
					case MSG_CLOSE_DEVICE:
						clientStatus[clientId] = 0;
						break;
					case MSG_SET_PIN_OUTPUT:
						sendCommand(CMD_SET_PIN_OUTPUT, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_SET_PIN_INPUT:
						sendCommand(CMD_SET_PIN_INPUT, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_SET_PIN_HIGH:
						sendCommand(CMD_SET_PIN_HIGH, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_SET_PIN_LOW:
						sendCommand(CMD_SET_PIN_LOW, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_GET_PIN_STATUS:
						sendCommand(CMD_GET_PIN_STATUS, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_ANALOG_WRITE:
						sendCommandWithParameter(CMD_ANALOG_WRITE, clientId, targetFd,
							count > 3 ? (atoi (parts[3]) & 0xFF) : -1,
							count > 4 ? (atoi (parts[4]) & 0xFF) : -1);
						break;
					case MSG_ANALOG_READ:
						sendCommand(CMD_ANALOG_READ, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_ANALOG_REFERENCE:
						sendCommand(CMD_ANALOG_REFERENCE, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_SERVO_ATTACH:
						sendCommand(CMD_SERVO_ATTACH, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_SERVO_WRITE:
						sendCommandWithParameter(CMD_SERVO_WRITE, clientId, targetFd,
							count > 3 ? (atoi (parts[3]) & 0xFF) : -1,
							count > 4 ? (atoi (parts[4]) & 0xFF) : -1);
						break;
					case MSG_SERVO_READ:
						sendCommand(CMD_SERVO_READ, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_SERVO_DETACH:
						sendCommand(CMD_SERVO_DETACH, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_READ_DHT11:
						sendCommand(CMD_READ_DHT11, clientId, targetFd, count > 3 ? atoi(parts[3]) : -1);
						break;
					case MSG_READ_SR04:
						sendCommandWithParameter(CMD_READ_SR04, clientId, targetFd,
							count > 3 ? (atoi (parts[3]) & 0xFF) : -1,
							count > 4 ? (atoi (parts[4]) & 0xFF) : -1);
						break;
					case MSG_RESET_DEVICE:
						sendCommandWithoutParameter(CMD_RESET_DEVICE, clientId, targetFd);
						break;
				}
			}
        }
    }

    /* cleanup before exit */
    ASSERT_TRUE ((mqd_t)-1 != mq_close (mq));
    ASSERT_TRUE ((mqd_t)-1 != mq_unlink (REQUEST_QUEUE_NAME));
    syslog (LOG_NOTICE, "UUGear daemon is terminated.");
    closelog ();

    return 0;
}