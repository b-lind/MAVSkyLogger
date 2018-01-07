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
#include "SDInterface.h"
#include "MavSky.h"
#include "Logger.h"

extern Logger *logger;
extern MavConsole *console;

SDInterface::SDInterface() 
{
  if(!sd.begin())
  {
    logger->report_error(CARD_ERROR);
  }
  else
  {
    sd.chvol();

    if(!configFile.open("MAV_CONF.ini", O_READ))
    {
      logger->report_error(CONFIG_FILE_ERROR);
    }
  } 
}

SDInterface::~SDInterface() { }

bool parse_bool(int b)
{
  if(b == 1)
  {
    return true;
  }
  else if (b == 0)
  {
    return false;  
  }
  else
  {
    return false;  
  }
}

void SDInterface::readConfig() 
{    
  if(!configFile.isOpen())
  {
    return;
  }
  
  while (configFile.available()) 
  {
    char key[30];
    char val[20];

    configFile.fgets(key, sizeof(key), "=");
    configFile.fgets(val, sizeof(val));

    if(key[0] == 'M')
    {
      char index[1];
      index[0] = key[1];
      
      logger->whitelist[atoi(index)] = atoi(val);
    }
    else if(key[0] == 'D')
    {
      if(key[1] == '0')
      {
        logger->dataValidation = atoi(val); 
      }
      else
      {
        logger->dataExpiry = atoi(val);
      }
    }
    else if(key[0] == 'L')
    {
      if(key[1] == '0')
      {
        logger->logRate = atoi(val);
      }
      else
      {
        logger->debugOutput = atoi(val); 
      }
    }
    else
    {
      logger->report_error(CONFIG_SYNTAX_ERROR);
    }
  }

  configFile.close();
}

void SDInterface::start_log()
{    
  if(!logFile.open("MAV_LOG.log", O_CREAT | O_WRITE))
  {
    logger->report_error(LOG_OPEN_ERROR);
  }
}

void SDInterface::finish_log()
{
  logFile.close();
}

void SDInterface::writeln(char* fmt, ...) 
{
  if(logFile.isOpen())
  {
    return;
  }
  
  char formatted_string[256];
  va_list argptr;
  va_start(argptr, fmt);
  vsprintf(formatted_string, fmt, argptr);
  va_end(argptr);
    
  int b = logFile.write(formatted_string);

  if(b != -1)
  {
    writtenBytes += b;

    if(writtenBytes >= BLOCK_SIZE)
    {
      logFile.sync();
    }
  }
  else
  {
    logger->report_error(LOG_WRITE_ERROR);
  }
}
