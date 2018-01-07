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
#include <GCS_MAVLink.h>
#include "Logger.h"
#include "MavConsole.h"
#include "SDInterface.h"

extern Logger *logger;
extern MavConsole *console;
extern SDInterface *sd;

#define START_MAVLINK_PACKETS       1

MavLinkData::MavLinkData() 
{
  MAVLINK_SERIAL.begin(57600);
}

double MavLinkData::degrees_to_radians(double degrees) 
{
  return degrees * (double)3.14159265358979323846 / (double)180.0;
}

double MavLinkData::radians_to_degrees(double radians) 
{  
  return radians * (double)180.0 / (double)3.14159265358979323846;
}

// http://www.movable-type.co.uk/scripts/latlong.html
double MavLinkData::get_distance_between_coordinates_double(double lat1, double lon1, double lat2, double lon2) 
{
  double theta1 = degrees_to_radians(lat1);
  double theta2 = degrees_to_radians(lat2);
  double delta_theta = degrees_to_radians(lat2 - lat1);
  double delta_lambda = degrees_to_radians(lon2 - lon1);
  double a = sin(delta_theta / 2.0) * sin(delta_theta / 2.0) + cos(theta1) * cos(theta2) * sin(delta_lambda / 2.0) * sin(delta_lambda / 2.0);
  double c = 2.0 * atan2(sqrt(a), sqrt((double)1.0 - a));
  double d = 6371000.0 * c;
  return d;
}

double MavLinkData::get_distance_between_coordinates_int(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2) 
{
  if (lat1 == 0 || lon1 == 0 || lat2 == 0 || lon2 == 0) {
    return 0;
  }
  return get_distance_between_coordinates_double((double)lat1 / COORD_DEGREE_TO_INT_MULTIPLIER, (double)lon1 / COORD_DEGREE_TO_INT_MULTIPLIER, (double)lat2 / COORD_DEGREE_TO_INT_MULTIPLIER, (double)lon2 / COORD_DEGREE_TO_INT_MULTIPLIER);
}

double MavLinkData::get_bearing_to_coordinates_double(double lat1, double lon1, double lat2, double lon2) 
{
  double theta1 = degrees_to_radians(lat1);
  double theta2 = degrees_to_radians(lat2);
  double lambda1 = degrees_to_radians(lon1);
  double lambda2 = degrees_to_radians(lon2);
  double y = sin(lambda2 - lambda1) * cos(theta2);
  double x = cos(theta1) * sin(theta2) - sin(theta1) * cos(theta2) * cos(lambda2 - lambda1);
  double bearing = radians_to_degrees(atan2(y, x));
  if (bearing < 0.0) {
    bearing += 360.0;
  }
  return bearing;
}

double MavLinkData::get_bearing_to_coordinates_int(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2) 
{
  if (lat1 == 0 || lon1 == 0 || lat2 == 0 || lon2 == 0) {
    return 0;
  }
  return get_bearing_to_coordinates_double((double)lat1 / COORD_DEGREE_TO_INT_MULTIPLIER, (double)lon1 / COORD_DEGREE_TO_INT_MULTIPLIER, (double)lat2 / COORD_DEGREE_TO_INT_MULTIPLIER, (double)lon2 / COORD_DEGREE_TO_INT_MULTIPLIER);
}

void MavLinkData::start_mavlink_packet_type(mavlink_message_t* msg_ptr, uint8_t stream_id, uint16_t rate) 
{
  uint16_t byte_length;

  mavlink_msg_request_data_stream_pack(0xFF, 0xBE, msg_ptr, 1, 1, stream_id, rate, START_MAVLINK_PACKETS);
  byte_length = mavlink_msg_to_send_buffer(mavlink_buffer, msg_ptr);
  MAVLINK_SERIAL.write(mavlink_buffer, byte_length);
  
  delay(10);
}

void MavLinkData::start_mavlink_if_stopped(mavlink_message_t* msg_ptr) 
{
  static uint32_t initializing_timeout = 0;

  if (millis() > initializing_timeout) 
  {
    if (!logger->data_validation(Logger::MAVLINK_ID_HEARTBEAT)) 
    {
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_RAW_SENSORS, 2);
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_EXTENDED_STATUS, 3);
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_RAW_CONTROLLER, 0);
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_POSITION, 3);
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_EXTRA1, 5);
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_EXTRA2, 2);
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_EXTRA3, 3);
      initializing_timeout = millis() + 1000L;
    } 
    else if (!logger->data_validation(Logger::MAVLINK_ID_RC_CHANNELS_RAW)) 
    {
      start_mavlink_packet_type(msg_ptr, MAV_DATA_STREAM_RC_CHANNELS, 3);
      initializing_timeout = millis() + 1000L;
    }
  }
}

void MavLinkData::process_mavlink_packets()
{
  mavlink_message_t msg;
  mavlink_status_t status;
  uint8_t act_bit;

  start_mavlink_if_stopped(&msg);

  while (MAVLINK_SERIAL.available())
  {
    uint8_t c = MAVLINK_SERIAL.read();
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status))
    {
      switch (msg.msgid)
      {
        case MAVLINK_MSG_ID_HEARTBEAT:
          logger->add_timestamp(Logger::MAVLINK_ID_HEARTBEAT);
          mavlink_msg_heartbeat_decode(&msg, &heartbeat);          
          logger->log(Logger::MAVLINK_ID_HEARTBEAT, heartbeat.base_mode, heartbeat.custom_mode);

          act_bit = (heartbeat.base_mode >> 7) & 1;
                    
          if (act_bit && armed_bit != act_bit) 
          {
            armed_bit = act_bit;
            console->console_print("Log started");
            sd->start_log();
          }
          else if(!act_bit && armed_bit != act_bit)
          {
            armed_bit = act_bit;
            console->console_print("Log stopped");
            sd->finish_log(); 
          }
          break;

        case MAVLINK_MSG_ID_SYS_STATUS:
          logger->add_timestamp(Logger::MAVLINK_ID_SYS_STATUS);
          mavlink_msg_sys_status_decode(&msg, &sysStatus);          
          logger->log(Logger::MAVLINK_ID_SYS_STATUS, sysStatus.voltage_battery, sysStatus.current_battery);
          break;

        case MAVLINK_MSG_ID_GPS_RAW_INT:
          logger->add_timestamp(Logger::MAVLINK_ID_GPS_RAW_INT);
          mavlink_msg_gps_raw_int_decode(&msg, &gps);          
          logger->log(Logger::MAVLINK_ID_GPS_RAW_INT, gps.time_usec, gps.fix_type, gps.satellites_visible, gps.eph, gps.vel, gps.alt, gps.lat, gps.lon);
          
          act_bit = (heartbeat.base_mode >> 7) & 1;
          
          if (act_bit && armed_bit != act_bit) 
          {
            armed_bit = act_bit;
            
            armed_latitude  = gps.lat;
            armed_longitude = gps.lon;
              
            armed_distance  = round(get_distance_between_coordinates_int(armed_latitude, armed_longitude, gps.lat, gps.lon));
            armed_bearing   = round(get_bearing_to_coordinates_int(armed_latitude, armed_longitude, gps.lat, gps.lon));
          } 
          else if(!act_bit && armed_bit != act_bit)
          {                           
            armed_bit = act_bit;
            
            armed_latitude  = 0;
            armed_longitude = 0;
            armed_distance  = 0;
            armed_bearing   = 0;
          }          
          break;

        case MAVLINK_MSG_ID_ATTITUDE:
          logger->add_timestamp(Logger::MAVLINK_ID_ATTITUDE);
          mavlink_msg_attitude_decode(&msg, &attitude);          
          logger->log(Logger::MAVLINK_ID_ATTITUDE, radians_to_degrees(attitude.roll), radians_to_degrees(attitude.pitch), radians_to_degrees(attitude.yaw));
          break;

        case MAVLINK_MSG_ID_VFR_HUD:
          logger->add_timestamp(Logger::MAVLINK_ID_VFR_HUD);
          mavlink_msg_vfr_hud_decode(&msg, &vfrHud);
          logger->log(Logger::MAVLINK_ID_VFR_HUD, vfrHud.groundspeed, vfrHud.heading, vfrHud.throttle, vfrHud.alt, vfrHud.climb);
          break;

        case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
          logger->add_timestamp(logger->MAVLINK_ID_RC_CHANNELS_RAW);
          mavlink_msg_rc_channels_raw_decode(&msg, &rcChannels);
          logger->log(Logger::MAVLINK_ID_RC_CHANNELS_RAW, rcChannels.rssi);
          break;

        default:
          logger->log(logger->MAVLINK_ID_UNKNOWN, msg.msgid);
          break;
      }
    }
  }
}


