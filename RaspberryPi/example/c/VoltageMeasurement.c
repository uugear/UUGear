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
		int pin = 3;	// analog input pin 3
		
		int i, value;
		float voltage;
		for (i = 0; i < 100; i ++) {
			value = analogRead(&dev, pin);
			
			voltage = (float)(value * 5) / 1024;
			
			printf("%.2fV\n", voltage);	
			
			usleep(200000);
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