from time import sleep
from UUGear import *
from time import gmtime, strftime

UUGearDevice.setShowLogs(1)

device = UUGearDevice('UUGear-Arduino-7853-2668')

if device.isValid():
	for i in range(200):
		data = device.readDHT(4)
		humidity = (data >> 16) / 10
		if humidity > 100:
			humidity = (data >> 24)
		temperature = (data & 32767) / 10
		if temperature > 125:
			temperature = ((data & 32512) >> 8)
		if (data & 32768):
			temperature = -temperature
		print strftime("Time: %H:%M:%S", gmtime())
		print 'H:', humidity, '%  T:', temperature, 'C'
		sleep(1)
	device.detach()
	device.stopDaemon()
else:
	print 'UUGear device is not correctly initialized.'