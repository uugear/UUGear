#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/time.h>

#include "../../src/UUGear.h"


double getCurTime()
{
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	double time_in_mill = ((double)tv.tv_sec) * 1000 + ((double)tv.tv_usec) / 1000 ;
	return time_in_mill;
}

int main(int argc, char **argv)
{
	setupUUGear();
	
	setShowLogs(1);
	
	UUGearDevice dev = attachUUGearDevice ("UUGear-Arduino-7853-2668");
	
	if (dev.fd != -1)
	{
		double begin, end;
		
		begin = getCurTime();
		int value = analogRead(&dev, 3);
		end = getCurTime();
		printf("Analog read on A3 takes %f ms, value=%d\n", end - begin, value);
		
		begin = getCurTime();
		setPinModeAsInput (&dev, 9);
		end = getCurTime();
		printf("Set pin mode on D9 takes %f ms\n", end - begin);
		
		begin = getCurTime();
		value = getPinStatus(&dev, 9);
		end = getCurTime();
		printf("Digital read on D9 takes %f ms, value=%d\n", end - begin, value);			
		
		detachUUGearDevice (&dev);
	}
	else
	{
		printf("Can not open UUGear device.\n");	
	}

	cleanupUUGear();
		
    return 0;
}