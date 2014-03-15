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
		int value = readDHT11(&dev, 4);
		end = getCurTime();
		printf("Read DHT11 on D4 takes %f ms, value=%d\n", end - begin, value);
		
		printf("Humidity=%d%%, Temperature=%dC\n", value >> 8, value & 0xFF);
		
		detachUUGearDevice (&dev);
	}
	else
	{
		printf("Can not open UUGear device.\n");	
	}

	cleanupUUGear();
		
    return 0;
}