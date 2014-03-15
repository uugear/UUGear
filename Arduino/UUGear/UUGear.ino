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
 * 
 * Quick test: after uploading the sketch, send "U01" with both NL and CR in
 * the serial monitor, you should be able to see the 1 + {device id} + :)
 */
#include <EEPROM.h>

#define BAUD_RATE  115200

#define EEPROM_SIZE  1024

#define ID_PREFIX  "UUGear-Arduino-"

#define COMMAND_START_CHAR  'U'
#define COMMAND_END_STRING  "\r\n"

#define RESPONSE_START_CHAR  '\t'
#define RESPONSE_END_STRING  ":)"

String cmdBuf = "";
int cmdEndStrLen = strlen(COMMAND_END_STRING);

void setup() {
  // if has no id yet, generate one
  if (getID() == "") {
    generateID();
  }

  // initialize serial port
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  //Serial.println(getID());
}

void loop() {
  int len = Serial.available();
  for (int i = 0; i < len; i ++) {
    cmdBuf += (char)Serial.read();
  }
  if (cmdBuf != "") {
    // drop useless data
    if (cmdBuf.charAt(0) != COMMAND_START_CHAR) {
      int pos = cmdBuf.indexOf(COMMAND_START_CHAR);
      if (pos != -1) {
        cmdBuf = cmdBuf.substring(pos);
      } 
      else {
        cmdBuf = "";
      }
    }
    // extract complete command
    if (cmdBuf != "") {
      int pos = cmdBuf.indexOf(COMMAND_END_STRING);
      if (pos != -1) {
        String cmd = cmdBuf.substring(0, pos + cmdEndStrLen);
        cmdBuf = cmdBuf.substring(pos + cmdEndStrLen);
        processCommand(cmd);
      }
    }
  }
}

// read ID from EEPROM
String getID() {
  String id = "";
  for (int i = 0; i < EEPROM_SIZE; i ++) {
    int value = EEPROM.read(i);
    if (value == 0xFF) {
      return id;
    }
    id += char(value);
  }
  return id;
}

// generate ID and save it into EEPROM
void generateID() {
  randomSeed(analogRead(0));
  int part1 = random(1000, 10000);
  int part2 = random(1000, 10000);
  String id = ID_PREFIX + String(part1) + "-" + String(part2);
  int len = id.length();
  for (int i = 0; i < EEPROM_SIZE; i ++) {
    EEPROM.write(i, i < len ? id.charAt(i) : 0xFF);
  }
}

// log string to serial, with hex format
void logAsHex(String str) {
  String hex = "";
  int len = str.length();
  for (int i = 0; i < len; i ++) {
    int val = str.charAt(i);
    if (val < 16) {
      hex += "0";
    }
    hex += String(val, HEX);
    hex += " ";
  }
  hex.toUpperCase();
  Serial.println(hex);
}

// process the command
void processCommand(String cmd) {
  //logAsHex(cmd);
  if (cmd.length() > 3) {
    byte cmdId = cmd.charAt(1);
    switch(cmdId) {
    case 0x30:
      cmdGetID(cmd);
      break;
    case 0x31:
      cmdPinModeOutput(cmd);
      break;
    case 0x32:
      cmdPinModeInput(cmd);
      break;
    case 0x33:
      cmdSetPinUp(cmd);
      break;
    case 0x34:
      cmdSetPinDown(cmd);
      break;
    case 0x35:
      cmdGetPinStatus(cmd);
      break;
    case 0x36:
      cmdAnalogWrite(cmd);
      break;
    case 0x37:
      cmdAnalogRead(cmd);
      break;
    case 0x50:
      cmdReadDHT11(cmd);
    }
  }
}

// command to retrive the ID
// example: 55 30 01 0D 0A
void cmdGetID(String cmd) {
  if (cmd.length() > 4) {
    byte clientId = cmd.charAt(2);
    Serial.write(RESPONSE_START_CHAR);
    Serial.write(clientId);
    Serial.print(getID());
    Serial.print(RESPONSE_END_STRING);
  }
}

// command to set pin as output
// example: 55 31 09 0D 0A
void cmdPinModeOutput(String cmd) {
  if (cmd.length() > 4) {
    byte pin = cmd.charAt(2);
    pinMode(pin, OUTPUT);
  }
}

// command to set pin as input
// example: 55 32 09 0D 0A
void cmdPinModeInput(String cmd) {
  if (cmd.length() > 4) {
    byte pin = cmd.charAt(2);
    pinMode(pin, INPUT);
  }
}

// command to set pin up
// example: 55 33 09 0D 0A
void cmdSetPinUp(String cmd) {
  if (cmd.length() > 4) {
    byte pin = cmd.charAt(2);
    digitalWrite(pin, HIGH);
  }
}

// command to set pin down
// example: 55 34 09 0D 0A
void cmdSetPinDown(String cmd) {
  if (cmd.length() > 4) {
    byte pin = cmd.charAt(2);
    digitalWrite(pin, LOW);
  }
}

// command to read pin status
// example: 55 35 09 01 0D 0A
void cmdGetPinStatus(String cmd) {
  if (cmd.length() > 5) {
    byte pin = cmd.charAt(2);
    byte clientId = cmd.charAt(3);    
    Serial.write(RESPONSE_START_CHAR);
    Serial.write(clientId);
    Serial.print(String(digitalRead(pin)));
    Serial.print(RESPONSE_END_STRING);
  }
}

// command to write analog value
// example: 55 36 09 70 0D 0A
void cmdAnalogWrite(String cmd) {
  if (cmd.length() > 5) {
    byte pin = cmd.charAt(2);
    byte value = cmd.charAt(3);
    analogWrite(pin, value);
  }
}

// command to read analog value
// example: 55 37 09 01 0D 0A
void cmdAnalogRead(String cmd) {
  if (cmd.length() > 5) {
    byte pin = cmd.charAt(2);
    byte clientId = cmd.charAt(3);
    Serial.write(RESPONSE_START_CHAR);
    Serial.write(clientId);
    Serial.print(String(analogRead(pin)));
    Serial.print(RESPONSE_END_STRING);
  }
}

// command to read DHT11 sensor
// example: 55 50 04 01 0D 0A
void cmdReadDHT11(String cmd) {
  if (cmd.length() > 5) {
    byte pin = cmd.charAt(2);
    byte clientId = cmd.charAt(3);

    // pull down 18ms as start signal
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(18);

    // pull up to receive data
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);

    // now let's read 3 + 40 * 2 edges
    // first 3 edges are falling, raising and falling, as start
    // the following 80 edges are raising and falling, repeat 40 times, as 40 bits
    // none of the edges should let you wait longer than 80us
    unsigned long startTime;
    word rawHumidity;
    word rawTemperature;
    word data;
    for (char i = -3; i < 80; i ++) {
      byte age = 0;
      startTime = micros();
      // wait until expected edge shows up, response -1 for timeout
      while (digitalRead(pin) == (i & 1) ? HIGH : LOW) {
        age = micros() - startTime;
        if (age > 80) {
          Serial.write(RESPONSE_START_CHAR);
          Serial.write(clientId);
          Serial.print("-1");
          Serial.print(RESPONSE_END_STRING);
          return;
        }
      }
      // save the bit (zero bit never be longer than 30us)
      if (i >= 0 && (i & 1)) {
        data <<= 1;
        if (age > 30) {
          data |= 1;
        }
      }
      // save the raw humidity and raw temperature
      if (i == 31) {
        rawHumidity = data;
      } 
      else if (i == 63) {
        rawTemperature = data;
        data = 0;
      }
    }

    // verify the checksum, response -2 for failure
    byte humidity = rawHumidity >> 8;
    byte temperature = rawTemperature >> 8;
    if ((byte)(((byte)rawHumidity) + humidity + ((byte)rawTemperature) + temperature) != data) {
      Serial.write(RESPONSE_START_CHAR);
      Serial.write(clientId);
      Serial.print("-2");
      Serial.print(RESPONSE_END_STRING);
      return;
    }

    // response with the result
    int result = (humidity << 8) + temperature;
    Serial.write(RESPONSE_START_CHAR);
    Serial.write(clientId);
    Serial.print(result);
    Serial.print(RESPONSE_END_STRING);
  }
}


