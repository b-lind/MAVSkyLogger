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

#include <EEPROM.h>

#include <string.h>
#include "MavSky.h"
#include "Logger.h"

extern Logger *logger;
extern MavLinkData *mav;

MavConsole::MavConsole(usb_serial_class port) 
{
  if(logger->debugOutput)
  { 
    serial = port;
    serial.begin(57600);
  }
}

MavConsole::~MavConsole() { }

void MavConsole::console_print(const char* fmt, ...) 
{
  if(logger->debugOutput)
  {
    char formatted_string[256];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(formatted_string, fmt, argptr);
    va_end(argptr);
    serial.print(formatted_string);
  }
}

void MavConsole::do_help() 
{
  console_print("%s\r\n", PRODUCT_STRING);
  console_print("debug [all|heartbeat|gps|attitude|vfr|status|rc] [on|off]\r\n");
  console_print("timing\r\n");
  console_print("errors\r\n");
  console_print("cleanup\r\n");
}
  
void MavConsole::parse_debug_on_off(char* p, int msg_id) 
{
  int val = 0;
  
  if(strcmp(p, "on") == 0) 
  {
    val = 1;
  } 
  else if(strcmp(p, "off") == 0) 
  {
    val = 0;
  }
  else 
  {
    console_print("Unknown parameter\r\n");
    return;
  }

  logger->whitelist[msg_id] = val;
}

void MavConsole::do_times() 
{
  console_print("HEARTBEAT:         %d\r\n", logger->get_timestamp_delta(Logger::MAVLINK_ID_HEARTBEAT));
  console_print("SYS_STATUS:        %d\r\n", logger->get_timestamp_delta(Logger::MAVLINK_ID_SYS_STATUS));
  console_print("GPS_RAW_INT:       %d\r\n", logger->get_timestamp_delta(Logger::MAVLINK_ID_GPS_RAW_INT));
  console_print("VFR_HUD:           %d\r\n", logger->get_timestamp_delta(Logger::MAVLINK_ID_VFR_HUD)); 
  console_print("ATTITUDE:          %d\r\n", logger->get_timestamp_delta(Logger::MAVLINK_ID_ATTITUDE));
  console_print("RC_CHANNELS_RAW:   %d\r\n", logger->get_timestamp_delta(Logger::MAVLINK_ID_RC_CHANNELS_RAW)); 
}

void MavConsole::do_errors()
{
  console_print("Card Error:          %d\r\n", EEPROM.read(CARD_ERROR));
  console_print("Config File Error:   %d\r\n", EEPROM.read(CONFIG_FILE_ERROR));
  console_print("Config Syntax Error: %d\r\n", EEPROM.read(CONFIG_SYNTAX_ERROR));
  console_print("Log File Error:      %d\r\n", EEPROM.read(LOG_OPEN_ERROR));
  console_print("Log Write Error:     %d\r\n", EEPROM.read(LOG_WRITE_ERROR));
}

void MavConsole::clear_errors()
{
  EEPROM.write(CARD_ERROR,          0);
  EEPROM.write(CONFIG_FILE_ERROR,   0);
  EEPROM.write(CONFIG_SYNTAX_ERROR, 0);
  EEPROM.write(LOG_OPEN_ERROR,           0);
  EEPROM.write(LOG_WRITE_ERROR,           0);
}

void MavConsole::do_command(char *cmd_buffer) 
{
  char* p;
  
  p = strtok(cmd_buffer, " ");
  if(p != NULL) 
  {
    if(strcmp(p, "debug") == 0) 
    {
      p = strtok(NULL, " ");
      if(strcmp(p, "all") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_ALL);
      } 
      else if (strcmp(p, "heartbeat") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_HEARTBEAT);
      } 
      else if (strcmp(p, "gps") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_GPS_RAW_INT);
      } 
      else if (strcmp(p, "attitude") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_ATTITUDE);
      } 
      else if (strcmp(p, "vfr") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_VFR_HUD);
      } 
      else if (strcmp(p, "status") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_SYS_STATUS);
      } 
      else if (strcmp(p, "rc") == 0) 
      {
        p = strtok(NULL, " ");
        parse_debug_on_off(p, Logger::MAVLINK_ID_RC_CHANNELS_RAW);
      } 
      else 
      {
        console_print("Unknown parameter %s\r\n", p);
      } 
    }
    else if(strcmp(p, "timing") == 0) 
    {
      do_times();
    } 
    else if(strcmp(p, "help") == 0) 
    {
      do_help();
    }
    else if(strcmp(p, "errors") == 0)
    {
      do_errors();
    }
    else if(strcmp(p, "cleanup") == 0)
    {
      clear_errors();
    }
    else 
    {
      console_print("Unknown command\r\n");
    }
  }
}

void MavConsole::check_for_console_command() 
{
  if(logger->debugOutput)
  {
    while(serial.available()) 
    { 
      uint8_t c = serial.read();
      if(c == '\r' || c == '\n') 
      {
        cmd_buffer[cmd_index++] = '\0';
        cmd_index = 0;      
        
        do_command(cmd_buffer);
        serial.write("]");
      } 
      else 
      {      
        cmd_buffer[cmd_index++] = tolower(c);
      }
    }
  }
}
