### UUGear Solution: Extend Your Raspberry Pi with Arduino
====
The solution includes a sketch project on Arduino side and a programing library on Raspberry Pi Side. After uploading the sketch, the Arduino becomes a UUGear device and could be found by its unique id. The library provides APIs in C and Python languages, and allows your application to find the UUGear device (Arduino) and access its digital/analog pins, or interact with other modules connect to the Arduino.

Some Arduinos have USB port on board, and could connect to Raspberry Pi via USB cable.  After the connection, Raspberry Pi can talk to the Arduino, as a serial device. Your application can open the serial device as a file, and read/write data from/to it, as long as you know its serial device name, and the correct baud rate for communication. However the serial device name may change every time you connect Arduino to Raspberry Pi, and your application could not adapt that change automatically. Also your Arduino will need to run a sketch as protocol stack to make sure it understand the commands sent from your application.

The idea of this project is to provide an abstracted model of Arduino device, and allows your application to access the Arduino device via a unique and constant id. This project designs a protocol for the communication between Raspberry Pi and Arduino, and develop a set of friendly APIs to support programming the GPIO pins on Arduino.  In order to make the APIs thread-safe, there will be a daemon process that manages all Arduino devices via the corresponding serial ports, which works just like a server.  The client application, which will be developed by you, will communicate with the daemon process via message queue. Don't worry, all these details are transparent for you, you don't really need to know daemon process or message queue.

#### Project Homepage
- please visit [here](http://www.uugear.com/uugear-rpi-arduino-solution/)
