from time import sleep
from UUGear import *

UUGearDevice.setShowLogs(1)

device1 = UUGearDevice('UUGear-Arduino-7643-2786')

if device1.isValid():
	print 'Device 1 is ready'
else:
	print 'Device 1 is not ready.'

device2 = UUGearDevice('UUGear-Arduino-1888-6266')

if device2.isValid():
        print 'Device 2 is ready'
else:
        print 'Device 2 is not ready.'

sleep(5)

device1.detach()
device2.detach()
device1.stopDaemon()
