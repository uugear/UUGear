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
		int pins[] = {3, 6, 9};
		int values[] = {96, 64, 32};
		int deltas[] = {4, 4, 4};
		
		int i, j;
		for (i = 0; i < 2560; i ++) {
			for (j = 0; j < 3; j ++) {
				values[j] += deltas[j];
				if (deltas[j] > 0)
				{
					deltas[j] *= (values[j] < 150) ? 1 : -1;
				}
				else
				{
					deltas[j] *= (values[j] > 5) ? 1 : -1;
				}
				analogWrite (&dev, pins[j], values[j]);
			}
			usleep(50000);
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