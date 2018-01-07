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
#ifndef MAVCONSOLE_H
#define MAVCONSOLE_H

#include <WProgram.h> 
#include <GCS_MAVLink.h>

#include "MavLinkData.h"

class Logger;
  
class MavConsole {
  char cmd_buffer[128];
  int cmd_index = 0;
  usb_serial_class serial;
  
  public:
    MavConsole(usb_serial_class port);
    ~MavConsole();
    void console_print(const char* fmt, ...);
    void do_help();
    void parse_debug_on_off(char* p, int msg_id);
    void do_times();
    void do_errors();
    void clear_errors();
    void do_command(char *cmd_buffer);
    void check_for_console_command();
};

#endif
