from time import sleep
from UUGear import *

UUGearDevice.setShowLogs(0)

device = UUGearDevice('UUGear-Arduino-7853-2668')

if device.isValid():
	value = device.readDHT11(4)
	humidity = (value >> 8)
	temperature = (value & 255)
	print 'Humidity:', humidity, '%  Temperature:', temperature, 'C'
	device.detach()
	device.stopDaemon()
else:
	print 'UUGear device is not correctly initialized.'