from time import sleep
from UUGear import *

UUGearDevice.setShowLogs(0)

device = UUGearDevice('UUGear-Arduino-7853-2668')

if device.isValid():
	for i in range(100):
		print "%0.2f" % (float(device.analogRead(3)) * 5 / 1024), "V"
		sleep(0.2)
	
	device.detach()
	device.stopDaemon()
else:
	print 'UUGear device is not correctly initialized.'