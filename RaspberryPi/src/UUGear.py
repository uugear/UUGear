 #
 # UUGear Solution: extend your Raspberry Pi with Arduino
 #
 # Author: Shawn (shawn@uugear.com)
 #
 # Copyright (c) 2014 UUGear s.r.o.
 #
 #######################################################################
 #  This file is part of UUGear Solution: 
 #  http://www.uugear.com/uugear-rpi-arduino-solution/
 #  
 #  UUGear Solution is free software: you can redistribute it and/or modify
 #  it under the terms of the GNU Lesser General Public License as published
 #  by the Free Software Foundation, either version 3 of the License, or
 #  (at your option) any later version.
 #  
 #  UUGear Solution is distributed in the hope that it will be useful,
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #  GNU Lesser General Public License for more details.
 #  
 #  You should have received a copy of the GNU Lesser General Public License
 #  along with UUGear Solution.  If not, see <http://www.gnu.org/licenses/>.
 #
 #######################################################################
 
from ctypes import CDLL, Structure, POINTER, byref, c_char, c_int, c_float, c_char_p

UUGEAR_ID_MAX_LENGTH = 1024

uugearlib = CDLL('./libUUGear.so')

class UUGearDeviceProfile(Structure):
    _fields_ = [
    	('id', (c_char * UUGEAR_ID_MAX_LENGTH)), 
    	('clientId', c_int), 
    	('fd', c_int), 
    	('in', c_int), 
    	('out', c_int)
    ]

uugearlib.setShowLogs.restype = None
uugearlib.setShowLogs.argtypes = [c_int]

uugearlib.attachUUGearDevice.restype = UUGearDeviceProfile
uugearlib.attachUUGearDevice.argtypes = [c_char_p]

uugearlib.setPinModeAsOutput.restype = None
uugearlib.setPinModeAsOutput.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.setPinHigh.restype = None
uugearlib.setPinHigh.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.setPinLow.restype = None
uugearlib.setPinLow.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.setPinModeAsInput.restype = None
uugearlib.setPinModeAsInput.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.getPinStatus.restype = c_int
uugearlib.getPinStatus.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.analogWrite.restype = None
uugearlib.analogWrite.argtypes = [POINTER(UUGearDeviceProfile), c_int, c_int]

uugearlib.analogRead.restype = c_int
uugearlib.analogRead.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.analogReference.restype = None
uugearlib.analogReference.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.attachServo.restype = None
uugearlib.attachServo.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.writeServo.restype = None
uugearlib.writeServo.argtypes = [POINTER(UUGearDeviceProfile), c_int, c_int]

uugearlib.readServo.restype = c_int
uugearlib.readServo.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.detachServo.restype = None
uugearlib.detachServo.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.readDHT.restype = c_int
uugearlib.readDHT.argtypes = [POINTER(UUGearDeviceProfile), c_int]

uugearlib.readSR04.restype = c_float
uugearlib.readSR04.argtypes = [POINTER(UUGearDeviceProfile), c_int, c_int]

uugearlib.detachUUGearDevice.restype = None
uugearlib.detachUUGearDevice.argtypes = [POINTER(UUGearDeviceProfile)]


class UUGearDevice(object):
	
	devProfile = None

	@staticmethod
	def setShowLogs(show):
		uugearlib.setShowLogs(show)
	
	def __init__(self, id):
		uugearlib.setupUUGear()
		self.devProfile = uugearlib.attachUUGearDevice(id)
		
	def isValid(self):
		return self.devProfile != None and self.devProfile.fd != -1
	
	def detach(self):
		if self.isValid():
			uugearlib.detachUUGearDevice(byref(self.devProfile))
			self.devProfile = None	
		
	def stopDaemon(self):
		uugearlib.cleanupUUGear()
	
	def setPinModeAsOutput(self, pin):
		if self.isValid():
			uugearlib.setPinModeAsOutput(byref(self.devProfile), pin)
		
	def setPinModeAsInput(self, pin):
		if self.isValid():
			uugearlib.setPinModeAsInput(byref(self.devProfile), pin)
		
	def setPinHigh(self, pin):
		if self.isValid():
			uugearlib.setPinHigh(byref(self.devProfile), pin)
			
	def setPinLow(self, pin):
		if self.isValid():
			uugearlib.setPinLow(byref(self.devProfile), pin)
			
	def getPinStatus(self, pin):
		if self.isValid():
			return uugearlib.getPinStatus(byref(self.devProfile), pin)
		else:
			return -1

	def analogWrite(self, pin, value):
		if self.isValid():
			uugearlib.analogWrite(byref(self.devProfile), pin, value)

	def analogRead(self, pin):
		if self.isValid():
			return uugearlib.analogRead(byref(self.devProfile), pin)
		else:
			return -1
			
	def analogReference(self, refType):
		if self.isValid():
			uugearlib.analogReference(byref(self.devProfile), refType)

	def attachServo(self, pin):
		if self.isValid():
			uugearlib.attachServo(byref(self.devProfile), pin)
			
	def writeServo(self, pin, angle):
		if self.isValid():
			uugearlib.writeServo(byref(self.devProfile), pin, angle)

	def readServo(self, pin):
		if self.isValid():
			return uugearlib.readServo(byref(self.devProfile), pin)
		else:
			return -1
			
	def detachServo(self, pin):
		if self.isValid():
			uugearlib.detachServo(byref(self.devProfile), pin)

	def readDHT(self, pin):
		if self.isValid():
			return uugearlib.readDHT(byref(self.devProfile), pin)
		else:
			return -1
			
	def readSR04(self, trigPin, echoPin):
		if self.isValid():
			return uugearlib.readSR04(byref(self.devProfile), trigPin, echoPin)
		else:
			return -1