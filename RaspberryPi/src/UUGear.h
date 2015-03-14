/*
 * UUGear Solution: extend your Raspberry Pi with Arduino
 *
 * Author: Shawn (shawn@uugear.com)
 *
 * Copyright (c) 2014 UUGear s.r.o.
 *
 ***********************************************************************
 *  This file is part of UUGear Solution: 
 *  http://www.uugear.com/uugear-rpi-arduino-solution/
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

/**
 * Configures the reference voltage used for analog input
 * (i.e. the value used as the top of the input range)
 *
 * parameters:
 *	dev is the pointer of the device struct
 *	type is the reference type:
 *		0 - DEFAULT
 *		1 - EXTERNAL
 */
extern void analogReference(UUGearDevice *dev, int type);

extern void detachUUGearDevice (UUGearDevice *dev);

extern void resetUUGearDevice(UUGearDevice *dev);

/**
 * Attach the servo on given pin
 * 
 * parameters:
 *  dev is the pointer of the device struct
 *	pin is the digital pin that links to the signal line of servo
 */
extern void attachServo(UUGearDevice *dev, int pin);

/**
 * Write the angle of servo on given pin
 * 
 * parameters:
 *  dev is the pointer of the device struct
 *	pin is the digital pin that connects to the signal line of servo
 *  angle is the target angle of the servo
 */
extern void writeServo(UUGearDevice *dev, int pin, int angle);

/**
 * Read the angle of servo on given pin
 * 
 * parameters:
 *  dev is the pointer of the device struct
 *	pin is the digital pin that connects to the signal line of servo
 *
 * returns:
 *  integer that represents the angle of the servo
 */
extern int readServo(UUGearDevice *dev, int pin);

/**
 * Detach the servo on given pin
 * 
 * parameters:
 *  dev is the pointer of the device struct
 *	pin is the digital pin that links to the signal line of servo
 */
extern void detachServo(UUGearDevice *dev, int pin);

/*
 * Read humidity and temperature values from DHT11 or DHT22 sensor
 *
 * parameters:
 *	dev is the pointer of the device struct
 *	pin is the digital pin that connects to the DATA wire of DHT sensor
 *
 * returns:
 *	integer (4 bytes) that contains both humidity and temperature data in order:
 *    1 byte humidity integer data
 *    1 byte humidity decimal data
 *    1 byte temperature integer data
 *    1 byte temperature decimal data
 *  if any error hanppens, return a negative value:
 *	  -1 for timeout
 *	  -2 for checksum error
 *	  -3 for communication error (please check /var/log/syslog)
 */
extern int readDHT(UUGearDevice *dev, int pin);

/**
 * Read distance value (in cm) from HC-SR04 sensor
 *
 * parameters:
 *	dev is the pointer of the device struct
 *	trigPin is the digital pin that connects to the TRIG wire of SR04
 *	echoPin is the digital pin that connects to the ECHO wire of SR04
 *  echoPin could be the same than trigPin if you wire them together.
 *
 * returns:
 *	float value of the distance in cm
 *  you may need to check if it is in the correct range by yourself.
 */
extern float readSR04(UUGearDevice *dev, int trigPin, int echoPin);

extern void cleanupUUGear();


extern void sendMessage(mqd_t in, int msgType, int clientId, int fd, int pin);

extern void sendMessageWithParameter(mqd_t in, int msgType, int clientId, int fd, int pin, int parameter);

extern int waitForInteger(UUGearDevice *dev, int * errorCode);

extern char * waitForString(UUGearDevice *dev, int * errorCode);

extern float waitForFloat(UUGearDevice *dev, int * errorCode);
