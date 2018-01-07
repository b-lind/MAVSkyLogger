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
#ifndef SDINTERFACE_H
#define SDINTERFACE_H

#include <WProgram.h> 
#include "SdFat.h"

#include "MavSky.h"

#define BLOCK_SIZE 1024

class SDInterface 
{    
  SdFatSdioEX sd;
  File configFile;
  File logFile;

  long writtenBytes = 0;
  
  public:
    SDInterface();
    ~SDInterface();
    void readConfig();
    void start_log();
    void finish_log();
    void writeln(char* fmt, ...);
    bool parse_bool(int b);
};

#endif
