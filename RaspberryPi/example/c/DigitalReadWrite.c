#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>

#include "../../src/UUGear.h"

int main(int argc, char **argv)
{
	setupUUGear();
	
	setShowLogs(1);
	
	UUGearDevice dev = attachUUGearDevice ("UUGear-Arduino-7853-2668");
	
	if (dev.fd != -1)
	{
		setPinModeAsOutput (&dev, 13);
		int i = 0;
		for (i = 0; i < 5; i ++)
		{
			setPinHigh (&dev, 13);
			
			usleep(200000);
			
			setPinLow (&dev, 13);
		
			usleep(200000);
		}
		
		setPinModeAsInput (&dev, 9);
		
		printf("Pin 9 status=%d\n", getPinStatus(&dev, 9));
		
		sleep(1);
		
		detachUUGearDevice (&dev);
	}
	else
	{
		printf("Can not open UUGear device.\n");	
	}

	cleanupUUGear();
		
    return 0;
}