//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  A copy of the GNU General Public License is available at <http://www.gnu.org/licenses/>.
//    
//  Creator:  Björn Lindenblatt
//  based on https://github.com/scottflys/mavsky by Scott Simpson
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <GCS_MAVLink.h>
#include <EEPROM.h>
#include "MavSky.h"
#include "MavConsole.h"
#include "Logger.h"
#include "SDInterface.h"
                          
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Logger      *logger;
MavConsole  *console;
MavLinkData *mav;
SDInterface *sd;

void setup()  
{
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  
  console = new MavConsole(Serial);
  logger  = new Logger();
  mav     = new MavLinkData();
  sd      = new SDInterface();

  sd->readConfig();
  
  console->console_print("%s\r\nStarting\r\n]", PRODUCT_STRING);
}

uint32_t logLoop  = 0L;

void loop()  
{
  uint32_t milli = millis();

  mav->process_mavlink_packets();        

  console->check_for_console_command();

  if(milli >= logLoop && logger->logRate != 0)
  {
    logLoop = milli + logger->logRate;
    logger->retroactive_log();
  }
}

