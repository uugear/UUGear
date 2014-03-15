/*
 * UUGear Solution: extend your Raspberry Pi with Arduino
 *
 * Author: Shawn (shawn@uugear.com)
 *
 * Copyright (c) 2014 UUGear s.r.o.
 *
 ***********************************************************************
 *  This file is part of UUGear Solution: 
 *  http://www.uugear.com/?page_id=50
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
#include "UUGearDaemon.h"

struct UUGearDevice
{
	char id[UUGEAR_ID_MAX_LENGTH];
	int clientId;
	int fd;
	mqd_t in;
	mqd_t out;
};

typedef struct UUGearDevice UUGearDevice;

extern void setShowLogs (int show);

extern void setupUUGear ();

extern struct UUGearDevice attachUUGearDevice (char *id);

extern void setPinModeAsOutput(UUGearDevice *dev, int pin);

extern void setPinModeAsInput(UUGearDevice *dev, int pin);

extern void setPinHigh(UUGearDevice *dev, int pin);

extern void setPinLow(UUGearDevice *dev, int pin);

extern int getPinStatus(UUGearDevice *dev, int pin);

extern void analogWrite(UUGearDevice *dev, int pin, int value);

extern int analogRead(UUGearDevice *dev, int pin);

extern void detachUUGearDevice (UUGearDevice *dev);

/*
 * Read humidity and temperature values from DHT11 sensor
 *
 * parameters:
 *	dev is the pointer of the device struct
 *	pin is the digital pin that connects to DATA wire of DHT11
 * returns:
 *	integer that lowest byte is temperature while higher byte is humidity
 *	-1 for timeout
 *	-2 for checksum error
 *	-3 for communication error (please check /var/log/syslog)
 */
extern int readDHT11(UUGearDevice *dev, int pin);

extern void cleanupUUGear ();
