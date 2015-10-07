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

/* 
 This example read humidity and temperature from DHT11 or DHT22 sensor
*/
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
			time_t mytime = time(NULL);
			int data = readDHT(&dev, 4);
			float humidity = ((float)(data >> 16)) / 10;
			if ( humidity > 100 )
			{
				humidity = (data >> 24);	// for DHT11
			}
			float temperature = ((float)(data & 0x7FFF)) / 10;
			if ( temperature > 125 ) 
			{
				temperature = ((data & 0x7F00) >> 8);	// for DHT11
			}
			if (data & 0x8000)
			{
				temperature = -temperature;
			}
		    printf(ctime(&mytime));
			printf("Humidity: %.1f%%  Temperature: %.1fC\n", humidity, temperature);
			// do not read too frequently
			sleep(2);
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