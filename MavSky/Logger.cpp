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
#include "MavConsole.h"
#include "Logger.h"
#include "MavSky.h"
#include "SDInterface.h"

#include <EEPROM.h>

extern Logger       *logger;
extern MavConsole   *console;
extern MavLinkData  *mav;
extern SDInterface  *sd;

Logger::Logger() 
{
  for(int i = 0; i < ID_COUNT; i++) 
  {
    previous_time[i] = -9999999L;      // made very negative but leave some room in the long for comparing to current time
    current_time[i]  = -9999999L;
  }

  memset(temp_log,   0x00, sizeof(temp_log));
  memset(temp_debug, 0x00, sizeof(temp_debug));
}

Logger::~Logger() {  
}
    
void Logger::log(int msg_id, ...) 
{
  if(console == NULL)
  {
    return;
  }
  
  if(whitelist[msg_id] == 1 || whitelist[MAVLINK_ID_ALL] == 1) 
  {    
    va_list argptr;

    //Generate Debug Output
    char debug_output[256];
    const char* debug_mask = debugMasks[msg_id];
    va_start(argptr, msg_id);
    vsprintf(debug_output, debug_mask, argptr);
    va_end(argptr);

    //Generate Log Output
    char log_output[256];
    const char* log_mask = logMasks[msg_id];
    va_start(argptr, msg_id);
    vsprintf(log_output, log_mask, argptr);
    va_end(argptr);

    if(logRate == 0)
    {
      console->console_print(debug_output, millis());
      sd->writeln(log_output, millis());
    }
    else
    {
      strcpy(temp_debug[msg_id], debug_output);
      strcpy(temp_log[msg_id],   log_output); 
    }
  }
}

void Logger::retroactive_log()
{
  for(int i = 0; i < 6; i++)
  {
    if(data_validation(i) == 1)
    {
      if(temp_debug[i][0] != 0x00 && (whitelist[i] == 1 || whitelist[MAVLINK_ID_ALL] == 1))
      {
        console->console_print(temp_debug[i], millis());
        sd->writeln(temp_log[i], millis());
      }  
    }
    else
    {
      if(temp_debug[i][0] != 0x00 && (whitelist[i] == 1 || whitelist[MAVLINK_ID_ALL] == 1))
      {
        char debug_output[256];
        char log_output[256];
        
        sprintf(debug_output, temp_debug[i], current_time[i]);
        sprintf(log_output, temp_log[i], current_time[i]);
        
        console->console_print("%d DATA EXPIRED [%d]: %s", millis(), get_timestamp_age(i), debug_output);
        sd->writeln("{ \"msg_type\": \"EXPIRED\", \"timestamp\": %d, \"age\": %d, \"msg\": %s },\r\n", millis(), get_timestamp_age(i), log_output);
      }
    }
  }
}

void Logger::report_error(int errorCode)
{
  digitalWrite(LEDPIN, HIGH);
  
  EEPROM.write(errorCode, 1);
}

void Logger::add_timestamp(int stamp_id) 
{
  previous_time[stamp_id] = current_time[stamp_id];
  current_time[stamp_id] = millis();
}

int32_t Logger::get_timestamp_delta(int stamp_id) 
{
  return current_time[stamp_id] - previous_time[stamp_id];
}

int32_t Logger::get_timestamp_age(int stamp_id) 
{
  uint32_t age = millis() - current_time[stamp_id];            
  return age;
}

int Logger::data_validation(int data_id) 
{
  if(!dataValidation)
  {
    return 1;
  }
  
  if (get_timestamp_age(data_id) < dataExpiry) 
  {
    return 1;
  } 
  else 
  {
    return 0;
  }
}
