#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "../../src/UUGear.h"


int main(int argc, char **argv)
{
	setupUUGear();
	
	setShowLogs(0);
	
	UUGearDevice dev = attachUUGearDevice ("UUGear-Arduino-7853-2668");
	
	if (dev.fd != -1)
	{
		int i;
		for (i = 0; i < 200; i ++)
		{
			float value = readSR04(&dev, 4, 4);
			printf("Distance = %f cm\n", value);
			sleep(1);
		}
		detachUUGearDevice (&dev);
	}
	else
	{
		printf("Can not open UUGear device.\n");	
	}

	cleanupUUGear();
		
    return 0;
}