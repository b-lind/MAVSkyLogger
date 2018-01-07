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
#ifndef LOGGER_H
#define LOGGER_H

#include <WProgram.h>

#define ID_COUNT 8
    
class Logger {     
  
  //Timestamp Arrays
  int32_t previous_time[ID_COUNT];
  int32_t current_time[ID_COUNT];
  
  public:  
  
	  //Message IDs for Logging Rules and Timestamps
	  static const byte MAVLINK_ID_HEARTBEAT         = 0;		
    static const byte MAVLINK_ID_SYS_STATUS        = 1;
    static const byte MAVLINK_ID_GPS_RAW_INT       = 2;
    static const byte MAVLINK_ID_ATTITUDE          = 3;
    static const byte MAVLINK_ID_VFR_HUD           = 4;
    static const byte MAVLINK_ID_RC_CHANNELS_RAW   = 5;
    static const byte MAVLINK_ID_UNKNOWN           = 6;  //only for logging rules
    static const byte MAVLINK_ID_ALL               = 7;  //only for logging rules

    //Logging Rules
    int whitelist[ID_COUNT];

    //Temp Log
    char temp_log  [6][256];
    char temp_debug[6][256];

    //Output Masks
    const char* debugMasks[7] = 
      { 
        "%%d  MAVLINK_MSG_ID_HEARTBEAT        base_mode: %d, custom_mode: %d \r\n", 
        "%%d  MAVLINK_MSG_ID_SYS_STATUS       voltage_battery: %d, current_battery: %d \r\n",
        "%%d  MAVLINK_MSG_ID_GPS_RAW_INT      msec: %ld fixtype: %d, sats: %d, hdop: %d, speed: %ld, alt: %ld lat: %ld lon: %ld \r\n",
        "%%d  MAVLINK_MSG_ID_ATTITUDE         roll_angle: %d deg, pitch_angle: %d deg, yaw_angle: %d deg \r\n",
        "%%d  MAVLINK_MSG_ID_VFR_HUD          groundspeed: %f, heading: %d, throttle: %d, alt: %f, climbrate: %f \r\n",
        "%%d  MAVLINK_MSG_ID_RC_CHANNELS_RAW  rssi: %d",
        "%%d  Unhandled MAVLINK message %d"
      };
    
    const char* logMasks[7] =
      { 
        "{ \"msg_type\": \"HEARTBEAT\", 	    \"timestamp\": %%d, \"base_mode\":       %d, 	 \"custom_mode\":     %d  },\r\n", 
        "{ \"msg_type\": \"SYS_STATUS\",      \"timestamp\": %%d, \"voltage_battery\": %d,   \"current_battery\": %d  },\r\n",
        "{ \"msg_type\": \"GPS_RAW_INT\",     \"timestamp\": %%d, \"msec\":            %ld,  \"fixtype\":         %d, \"sats\":      %d, \"hdop\": %d, \"speed\":     %ld, \"alt\": %ld, \"lat\": %ld, \"lon\": %ld  },\r\n",
        "{ \"msg_type\": \"ATTITUDE\",        \"timestamp\": %%d, \"roll_angle\":      %d, 	 \"pitch_angle\":     %d, \"yaw_angle\": %d  },\r\n",
        "{ \"msg_type\": \"VFR_HUD\",         \"timestamp\": %%d, \"groundspeed\":     %f, 	 \"heading\":         %d, \"throttle\":  %d, \"alt\":  %f, \"climbrate\": %f   },\r\n",
        "{ \"msg_type\": \"RC_CHANNELS_RAW\", \"timestamp\": %%d, \"rssi\":            %d    },\r\n",
        "{ \"msg_type\": \"UNHANDLED\",       \"timestamp\": %%d, \"id\":              %d    },\r\n"
      }; 

    // Writes all Log data to Console
    bool debugOutput = true;

    // Logging Rate in ms; 0 equals logging on receive
    int logRate = 1000;

    // Checks for outdated data before logging
    bool dataValidation = true;
    int dataExpiry = 3000;
    
    Logger();
    ~Logger();
    void log(int subsystem, ...);
    void retroactive_log();
    void report_error(int errorCode);
    void add_timestamp(int stamp_id);
    int32_t get_timestamp_delta(int stamp_id);
    int32_t get_timestamp_age(int stamp_id);
    int data_validation(int data_id);
};


#endif

