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
#ifndef MAVLINKDATA_H
#define MAVLINKDATA_H

#include <GCS_MAVLink.h>
#include <WProgram.h> 

#define MAVLINK_SERIAL  Serial2

class MavLinkData {       
  private:  
    uint8_t   mavlink_buffer[MAVLINK_MAX_PACKET_LEN];

    uint32_t  last_process_1000_gps_latitude = 0;
    uint32_t  last_process_1000_gps_longitude = 0;

    uint8_t   armed_bit = 0;
    
    double degrees_to_radians(double degrees);
    double radians_to_degrees(double radians);
    double get_distance_between_coordinates_int(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2);
    double get_distance_between_coordinates_double(double lat1, double lon1, double lat2, double lon2) ;
    double get_bearing_to_coordinates_int(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2);
    double get_bearing_to_coordinates_double(double lat1, double lon1, double lat2, double lon2);
           
  public:
    const double COORD_DEGREE_TO_INT_MULTIPLIER = 10000000.0;
    
    //  MAVLINK_MSG_ID_HEARTBEAT
    mavlink_heartbeat_t heartbeat;       
    
    // MAVLINK_MSG_ID_SYS_STATUS
    mavlink_sys_status_t sysStatus;
    
    // MAVLINK_MSG_ID_GPS_RAW_INT 
    mavlink_gps_raw_int_t gps;
    
    // MAVLINK_MSG_ID_VFR_HUD 
    mavlink_vfr_hud_t vfrHud;          
  
    // MAVLINK_MSG_ID_ATTITUDE
    mavlink_attitude_t attitude;

    // MAVLINK_MSG_ID_RC_CHANNELS_RAW
    mavlink_rc_channels_raw_t rcChannels;
  
    // Calculated     
    int32_t   armed_latitude      = 0;               
    int32_t   armed_longitude     = 0;
    uint32_t  armed_distance      = 0;    // in m
    uint16_t  armed_bearing       = 0;    // in degrees (0-359)
    
    MavLinkData();
    ~MavLinkData();
    void start_mavlink_packet_type(mavlink_message_t* msg_ptr, uint8_t stream_id, uint16_t rate);
    void start_mavlink_if_stopped(mavlink_message_t* msg_ptr);
    
    void process_mavlink_packets();
};


#endif


