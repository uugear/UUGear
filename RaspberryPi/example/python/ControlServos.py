from time import sleep
from UUGear import *
from time import gmtime, strftime

UUGearDevice.setShowLogs(0)

device = UUGearDevice('UUGear-Arduino-9886-9947')

if device.isValid():
	
	device.attachServo(4)
	device.attachServo(5)
	
	device.writeServo(4, 60)
	device.writeServo(5, 30)
	
	sleep(3)
	
	#device.detachServo(4)
	#device.detachServo(5)
	
	device.detach()
	device.stopDaemon()
else:
	print 'UUGear device is not correctly initialized.'