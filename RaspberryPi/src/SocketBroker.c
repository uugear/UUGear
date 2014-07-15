#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include "UUGear.h"

#define SOCKET_FILE_PATH	"/tmp/uugear_socket_broker.sock"
#define SOCKET_FILE_PERM	"0666"
#define LISTEN_BACKLOG		64
#define BUFFER_SIZE			512

#define TEXT_REQ_RECEIVED	"Request received: "

#define MAX_PARAM_NUMBER	4
#define MAX_PARAM_LENGTH	256

#define DATA_TYPE_VOID		0x01
#define DATA_TYPE_STRING	0x02
#define DATA_TYPE_INTEGER	0x03
#define DATA_TYPE_FLOAT		0x04

#define MAX_UUGEAR_DEVICES	16


/* struct for parameter storage */
struct Param
{
	unsigned char type;
	unsigned char length;
	union ParamVal
	{
		int i;
		float f;
		char str[MAX_PARAM_LENGTH];
	} value;
};

void exitWithError(char * func, char * msg)
{
	perror(func);
	syslog (LOG_ERR, msg);
	exit(EXIT_FAILURE);
}

int main()
{
	int running = 1;
    int server_socket, client_socket;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_size;
    char req_buf[BUFFER_SIZE];
    char resp_buf[BUFFER_SIZE];
    mode_t mode = strtol(SOCKET_FILE_PERM, 0, 8);
    UUGearDevice *devices[MAX_UUGEAR_DEVICES];
    int deviceCount = 0;

    /* create server socket */
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket  == -1)
    {
    	exitWithError("socket", "Unable to create server socket.");
    }

    /* bind server socket to file */
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_FILE_PATH, sizeof(server_addr.sun_path) - 1);
    unlink(server_addr.sun_path);
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1)
	{
    	exitWithError("bind", "Unable to bind server socket to file.");
	}

    /* listen for incoming connections */
    if (listen(server_socket, LISTEN_BACKLOG) == -1) 
	{
    	exitWithError("listen", "Failed on listening.");
	}

    /* set file permission of the socket */
    if (chmod(SOCKET_FILE_PATH, mode) == -1)
	{
    	exitWithError("chmod", "Can not change file permission.");
	}

    while(running) 
    {
		/* wait for client connection */
		syslog (LOG_INFO, "Waiting for incoming connection...");

		client_addr_size = sizeof(struct sockaddr_un);
		client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_size);
		if (client_socket == -1)
		{
	    	syslog (LOG_ERR, "Failed accepting client socket, skipping..,");
			continue;
		}
		
		syslog (LOG_INFO, "Socket is connected.");
		
		/* read data from client socket */
		int req_len = recv(client_socket, req_buf, BUFFER_SIZE, 0);
		if (req_len <= 0) 
		{
	    	syslog (LOG_ERR, "Received data length <= 0, skipping...");
	    	continue;
		}
		
		/* log received data */
		snprintf(resp_buf, strlen(TEXT_REQ_RECEIVED) + 1, "%s", TEXT_REQ_RECEIVED);
		strncat(resp_buf, req_buf, req_len);
		syslog (LOG_INFO, resp_buf);
		
		/* parse request */
		int i;
		unsigned char req_type = req_buf[0];
    	unsigned char return_type = DATA_TYPE_VOID;
    	struct Param params[MAX_PARAM_NUMBER];
		int paramCount = 0;
		if (req_len > 1)
		{
			/* return type */
			return_type = req_buf[1];
			/* collect all parameters */
			for (i = 2; i < req_len; )
			{
				params[paramCount].type = req_buf[i ++];
				params[paramCount].length = req_buf[i ++];
				snprintf(params[paramCount].value.str, params[paramCount].length + 1, "%s", &req_buf[i]);
				
				syslog (LOG_INFO, "Parameter type=%d, lenth=%d, value=%s", params[paramCount].type, params[paramCount].length, params[paramCount].value.str);
				
				switch (params[paramCount].type)
				{
					case DATA_TYPE_INTEGER:
						params[paramCount].value.i = atoi(params[paramCount].value.str);
						break;
					case DATA_TYPE_FLOAT:
						params[paramCount].value.f = atof(params[paramCount].value.str);
						break;	
				}
				i += params[paramCount].length;
				if (paramCount == MAX_PARAM_NUMBER)
				{
					syslog (LOG_ERR, "Too many parameters, skipping...");
					break;
				}
				paramCount ++;
			}
		}
		
		/* process request */
		switch (req_type)
		{
			case MSG_EXIT:
				syslog (LOG_INFO, "Received exit request, socket broker will quit.");
				running = 0;
				break;
			case MSG_GET_DEVICE:
				strcpy(resp_buf, "0");
				if (paramCount >= 1) {
					int found = 0;
					int firstEmptyDevice = -1;
					for (i = 0; i < deviceCount; i ++)
					{
						if (devices[i] != NULL && strncmp(devices[i]->id, params[0].value.str, params[0].length) == 0)
						{
							found = 1;
							break;
						}
						else
						{
							firstEmptyDevice = (firstEmptyDevice == -1 ? (devices[i] == NULL ? i : firstEmptyDevice) : firstEmptyDevice);
						}
					}
					if (found == 0)
					{
						if (deviceCount < MAX_UUGEAR_DEVICES)
						{
							setupUUGear();
							syslog (LOG_INFO, "Try to open device with id=%s.", params[0].value.str);
							UUGearDevice dev = attachUUGearDevice(params[0].value.str);
							if (dev.fd != -1)
							{
								if (firstEmptyDevice != -1)
								{
									devices[firstEmptyDevice] = &dev;	
								}
								else
								{
									devices[deviceCount] = &dev;
								}
								syslog (LOG_INFO, "Device opened and saved in index=%d.", firstEmptyDevice == -1 ? deviceCount : firstEmptyDevice);
								strcpy (resp_buf, "1");
								deviceCount ++;
							}
							else
							{
								syslog (LOG_ERR, "Device openning failed.");
							}
						}
						else
						{
							syslog (LOG_ERR, "Can not open too many devices.");
						}
					}
					else
					{
						strcpy(resp_buf, "2");
					}
				}
				if (send(client_socket, resp_buf, strlen(resp_buf), 0) < 0)
				{
			    	syslog (LOG_ERR, "Failed on response open device request, skipping...");
			    	continue;
				}
				break;
			case MSG_CLOSE_DEVICE:
				if (paramCount >= 1) {
					for (i = 0; i < deviceCount; i ++)
					{
						if (devices[i] != NULL && strncmp(devices[i]->id, params[0].value.str, params[0].length) == 0)
						{
							detachUUGearDevice(devices[i]);
							devices[i] = NULL;
							syslog (LOG_INFO, "Device on index=%d is closed.", i);
							break;
						}
					}
				}
				break;
			default:
				/* for other requests, process them in a common way */
				if (paramCount >= 1) {
					for (i = 0; i < deviceCount; i ++)
					{
						if (devices[i] != NULL && strncmp(devices[i]->id, params[0].value.str, params[0].length) == 0)
						{
							if (paramCount == 2)
							{
								if (params[1].type == DATA_TYPE_INTEGER)
								{
									sendMessage(devices[i]->in, req_type, devices[i]->clientId, devices[i]->fd, params[1].value.i);
								}
								else
								{
									syslog (LOG_ERR, "Unsupported request type is found!");
								}
							}
							else if (paramCount == 3)
							{
								if (params[1].type == DATA_TYPE_INTEGER && params[2].type == DATA_TYPE_INTEGER)
								{
									sendMessageWithParameter(devices[i]->in, req_type, devices[i]->clientId, devices[i]->fd, params[1].value.i, params[2].value.i);
								}
								else
								{
									syslog (LOG_ERR, "Unsupported request type is found!");
								}	
							}
							
							/* if return value is needed */
							if (return_type != DATA_TYPE_VOID)
							{
								int errorCode = 0;
								int intResult;
								float floatResult;
								switch (return_type)
								{
									case DATA_TYPE_STRING:
										strcpy(resp_buf, waitForString (devices[i], &errorCode));
										break;
									case DATA_TYPE_INTEGER:
										intResult = waitForInteger(devices[i], &errorCode);
										intResult = (errorCode == 0 ? intResult : -1);
										sprintf (resp_buf, "%d", intResult);
										break;
									case DATA_TYPE_FLOAT:
										floatResult = waitForFloat(devices[i], &errorCode);
										floatResult = (errorCode == 0 ? floatResult : errorCode);
										sprintf (resp_buf, "%f", floatResult);
										break;
								}
								if (send(client_socket, resp_buf, strlen(resp_buf), 0) < 0)
								{
							    	syslog (LOG_ERR, "Failed on sending back the response data, skipping...");
								}
							}
							break;
						}
					}
				}
				break;
		}
		
		/* clean up */
		close(client_socket);
    }
    
    close(server_socket);

    return 0;
}