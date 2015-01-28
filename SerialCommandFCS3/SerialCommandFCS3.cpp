// Fixed #40 (checksum is checked if it is there, not a compile option)
// Fixed #41 (issues with premature '(')
// Fixed #42 (CSLEN)
// Fixed #43 (rotating buffer)
// Fixed #45? (strtok_r called twice) => testen

/*******************************************************************************
This is a library modified to safe RAM on project Laadpaal
***********************************************************************************/

/*******************************************************************************
SerialCommand - An Arduino library to tokenize and parse commands received over
a serial port.
Copyright (C) 2011-2013 Steven Cogswell  <steven.cogswell@gmail.com>
http://awtfy.com

See SerialCommand.h for version history.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "SerialCommandFCS3.h"
#include <string.h>
#ifndef SERIALCOMMAND_HARDWAREONLY
#include <SoftwareSerial.h>
#endif

const char commandString_00[] PROGMEM = "Version?";   
const char commandString_01[] PROGMEM = "ChangeState";   
const char commandString_02[] PROGMEM = "State?";   
const char commandString_03[] PROGMEM = "Reset";   
const char commandString_04[] PROGMEM = "setLed";   
const char commandString_05[] PROGMEM = "setInstallation";   
const char commandString_06[] PROGMEM = "setCurrentCheck";   
const char commandString_07[] PROGMEM = "setVoltageCheck";   
const char commandString_08[] PROGMEM = "setHeartbeatCheck";   
const char commandString_09[] PROGMEM = "setTemperatureLimit";   
const char commandString_10[] PROGMEM = "setVentilationPermission";   
const char commandString_11[] PROGMEM = "setTagScanPeriod";   
const char commandString_12[] PROGMEM = "setBuzzer";   
const char commandString_13[] PROGMEM = "accepted";   
const char commandString_14[] PROGMEM = "setLed?";   
const char commandString_15[] PROGMEM = "setInstallation?";   
const char commandString_16[] PROGMEM = "setCurrentCheck?";   
const char commandString_17[] PROGMEM = "setVoltageCheck?";   
const char commandString_18[] PROGMEM = "setHeartbeatCheck?";   
const char commandString_19[] PROGMEM = "setTemperatureLimit?";   
const char commandString_20[] PROGMEM = "setVentilationPermission?";   
const char commandString_21[] PROGMEM = "setTagScanPeriod?";   
const char commandString_22[] PROGMEM = "setBuzzer?";   
const char commandString_23[] PROGMEM = "LockStatus?";   
const char commandString_24[] PROGMEM = "VehicleStatus?";   
const char commandString_25[] PROGMEM = "RelayStatus?";   
const char commandString_26[] PROGMEM = "TemperatureStatus?";   
const char commandString_27[] PROGMEM = "CableCurrentMax?";   
const char commandString_28[] PROGMEM = "EnclosureStatus?";   
const char commandString_29[] PROGMEM = "EmergencyStop";   
const char commandString_30[] PROGMEM = "Unlock";   
const char commandString_31[] PROGMEM = "ChargeStart";   
const char commandString_32[] PROGMEM = "ChargeStop";   
const char commandString_33[] PROGMEM = "Buzzer";   
const char commandString_34[] PROGMEM = "LedOverride";   
const char commandString_35[] PROGMEM = "LedIntensity";   
const char commandString_36[] PROGMEM = "SmartCurrentMax";   
const char commandString_37[] PROGMEM = "EnergyActiveExport?";   
const char commandString_38[] PROGMEM = "EnergyActiveImport?";   
const char commandString_39[] PROGMEM = "EnergyReactiveExport?";   
const char commandString_40[] PROGMEM = "EnergyReactiveImport?";   
const char commandString_41[] PROGMEM = "PowerActiveExport?";   
const char commandString_42[] PROGMEM = "PowerActiveImport?";   
const char commandString_43[] PROGMEM = "PowerReactiveExport?";   
const char commandString_44[] PROGMEM = "PowerReactiveImport?";   
const char commandString_45[] PROGMEM = "CurrentExport?";   
const char commandString_46[] PROGMEM = "CurrentImport?";   
const char commandString_47[] PROGMEM = "Voltage?";   
const char commandString_48[] PROGMEM = "PowerFactor?";   
const char commandString_49[] PROGMEM = "PhaseAngle?";   
const char commandString_50[] PROGMEM = "Frequency?";   
const char commandString_51[] PROGMEM = "SerialNumber?";   


const char * const commandString_table[] PROGMEM = {   
  commandString_00,
  commandString_01,  
  commandString_02,  
  commandString_03,  
  commandString_04,  
  commandString_05,  
  commandString_06,  
  commandString_07,  
  commandString_08,  
  commandString_09,  
  commandString_10,  
  commandString_11,  
  commandString_12,  
  commandString_13,  
  commandString_14,  
  commandString_15,  
  commandString_16,  
  commandString_17,  
  commandString_18,  
  commandString_19,  
  commandString_20,  
  commandString_21,  
  commandString_22,  
  commandString_23,  
  commandString_24,  
  commandString_25,  
  commandString_26,  
  commandString_27,  
  commandString_28,  
  commandString_29,  
  commandString_30,  
  commandString_31,  
  commandString_32,  
  commandString_33,  
  commandString_34,  
  commandString_35,  
  commandString_36,  
  commandString_37,  
  commandString_38,  
  commandString_39,  
  commandString_40,  
  commandString_41,  
  commandString_42,  
  commandString_43,  
  commandString_44,  
  commandString_45,  
  commandString_46,  
  commandString_47,  
  commandString_48,  
  commandString_49,  
  commandString_50,  
  commandString_51  
};


// Constructor makes sure some things are set.
SerialCommand::SerialCommand() {
	usingSoftwareSerial=0;
	strncpy(delim,"=",MAXDELIMETER);  // strtok_r needs a null-terminated string
	term='\r';   // return character, default terminator for commands
	numCommand=0;    // Number of callback handlers installed
	clearBuffer();
}

#ifndef SERIALCOMMAND_HARDWAREONLY
// Constructor to use a SoftwareSerial object
SerialCommand::SerialCommand(SoftwareSerial &_SoftSer) {
	usingSoftwareSerial=1;
	SoftSerial = &_SoftSer;
	strncpy(delim,"=",MAXDELIMETER);  // strtok_r needs a null-terminated string
	term='\r';   // return character, default terminator for commands
	numCommand=0;    // Number of callback handlers installed
	clearBuffer();
}
#endif

// Initialize the command buffer being processed to all null characters
void SerialCommand::clearBuffer() {
	for (int i = 0; i < SERIALCOMMANDBUFFER; i++)
		buffer[i]='\0';
	bufPos=0;

	for (int i = 0; i < CSLEN; i++)
		cs[i]='\0';
	csPos=0;
	hook = 0;
	totaal = 0;
}

// Retrieve the next token ("word" or "argument") from the Command buffer.
// returns a NULL if no more tokens exist.
char *SerialCommand::next() {
	char *nextToken;
	nextToken = strtok_r(NULL, delim, &last);
	return nextToken;
}

int SerialCommand::returncs(char* outStr) {
  	String str1 = "";
  	if(totaal%256 < 16)
  		str1 += '0';
  	str1 += String(totaal%256, HEX);

  	if(str1 == cs)
  		return 1;
 	return 0;
}

// This checks the Serial stream for characters, and assembles them into a buffer.
// When the terminator character (default '\r') is seen, it starts parsing the
// buffer for a prefix command, and calls handlers setup by addCommand() member
void SerialCommand::readSerial(unsigned long& ms) {
	char commandString_buffer[25]; 

	// If we're using the Hardware port, check it.   Otherwise check the user-created SoftwareSerial Port
	#ifdef SERIALCOMMAND_HARDWAREONLY
	while (Serial.available() > 0)
	#else
	while ((usingSoftwareSerial==0 && Serial.available() > 0) || (usingSoftwareSerial==1 && SoftSerial->available() > 0) )
	#endif
	{
		int i;
		boolean matched;
		if (usingSoftwareSerial==0) {   // Hardware serial port
			inChar=Serial.read();   	// Read single available character, there may be more waiting
		} else {
			#ifndef SERIALCOMMAND_HARDWAREONLY   // SoftwareSerial port
			inChar = SoftSerial->read();   		// Read single available character, there may be more waiting
			#endif
		}
		#ifdef SERIALCOMMANDDEBUG
		Serial.print(inChar);   // Echo back to serial stream
		#endif

		if (inChar==term) {     // Check for the terminator (default '\r') meaning end of command
			ms = millis();

			#ifdef SERIALCOMMANDDEBUG
			Serial.print("Received: ");
			Serial.println(buffer);
			#endif

			char csval[3];
			if(buffer[bufPos-4] ==  '(') {
				cs[0] = tolower(buffer[bufPos-3]);
				cs[1] = tolower(buffer[bufPos-2]);
				buffer[bufPos-4] = 0;
				for(int i = 0; i < bufPos-3; i++)
					totaal += buffer[i];

				if(!returncs(csval)){
					Serial.println("error(2A)");
			//		Serial.println("Checksum="+String(totaal%256, HEX)); //What the checksum should have been
					clearBuffer();
					return;
				}
			}

			bufPos=0;           // Reset to start of buffer
			token = strtok_r(buffer,"=",&last);   // Search for command at start of buffer
			if (token == NULL)
				return;
			matched=false;
			for (i=0; i<numCommand; i++) {
				strcpy_P(commandString_buffer, (char*)pgm_read_word(&(commandString_table[i]))); 
				#ifdef SERIALCOMMANDDEBUG
				Serial.print("Comparing [");
				Serial.print(token);
				Serial.print("] to [");
				Serial.print(commandString_buffer);
				Serial.println("]");
				#endif
				// Compare the found command against the list of known commands for a match
				if (strncmp(token,commandString_buffer,SERIALCOMMANDBUFFER) == 0) {
					#ifdef SERIALCOMMANDDEBUG
					Serial.print("Matched Command: ");
					Serial.println(token);
					#endif
					// Execute the stored handler function for the command
					(*CommandList[i].function)();
					clearBuffer();
					matched=true;
					break;
				}
			}
			if (matched==false) {
				(*defaultHandler)();
				clearBuffer();
			}
		}
		if (isprint(inChar) && bufPos < SERIALCOMMANDBUFFER - 1) {  // Only printable characters into the buffer
			buffer[bufPos++] = inChar;   // Put character into buffer
			buffer[bufPos] = '\0';  // Null terminate
		}
	}
}

// Adds a "command" and a handler function to the list of available commands.
// This is used for matching a found token in the buffer, and gives the pointer
// to the handler function to deal with it.
void SerialCommand::addCommand(const char *command, void (*function)()) {
	if (numCommand < MAXSERIALCOMMANDS) {
		#ifdef SERIALCOMMANDDEBUG
		Serial.print(numCommand);
		Serial.print("-");
		Serial.print("Adding command for ");
		Serial.println(command);
		#endif

		CommandList[numCommand].function = function;
		numCommand++;
	} else {
		// In this case, you tried to push more commands into the buffer than it is compiled to hold.
		// Not much we can do since there is no real visible error assertion, we just ignore adding
		// the command
		//pep#ifdef SERIALCOMMANDDEBUG
		Serial.println("Too many handlers - recompile changing MAXSERIALCOMMANDS");
		//pep#endif
	}
}

// This sets up a handler to be called in the event that the receveived command string
// isn't in the list of things with handlers.
void SerialCommand::addDefaultHandler(void (*function)()) {
	defaultHandler = function;
}
