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
	
	/* replace the device id with yours (listed by lsuu) */
	UUGearDevice dev = attachUUGearDevice ("UUGear-Arduino-9886-9947");
	
	if (dev.fd != -1)
	{
		/* attach servo on pin 4 */
		attachServo(&dev, 4);
		
		/* move servo to 30 degrees */
		writeServo(&dev, 4, 30);
		
		/* give some time to move */
		sleep(1);
		
		/* move servo to 90 degrees */
		writeServo(&dev, 4, 90);
		
		/* give some time to move */
		sleep(1);
		
		/* detach servo */
		detachServo(&dev, 4);
		
		detachUUGearDevice (&dev);
	}
	else
	{
		printf("Can not open UUGear device.\n");	
	}

	cleanupUUGear();
		
    return 0;
}