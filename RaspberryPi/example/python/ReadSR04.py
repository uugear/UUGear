from time import sleep
from UUGear import *
from time import gmtime, strftime

UUGearDevice.setShowLogs(0)

device = UUGearDevice('UUGear-Arduino-7853-2668')

if device.isValid():
	for i in range(200):
		value = device.readSR04(4, 4)
		print 'Distance:', value, 'cm'
		sleep(1)
	device.detach()
	device.stopDaemon()
else:
	print 'UUGear device is not correctly initialized.'