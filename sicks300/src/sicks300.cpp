/*
 *
 * sicks300.cpp
 *
 *
 * Copyright (C) 2010
 * Autonomous Intelligent Systems Group
 * University of Bonn, Germany
 *
 *
 * Authors: Andreas Hochrath, Torsten Fiolka
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * Origin:
 *  Player - One Hell of a Robot Server
 *  serialstream.cc & sicks3000.cc
 *  Copyright (C) 2003
 *     Brian Gerkey
 *  Copyright (C) 2000
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
 *
 * Adaptations:
 *  - remoted tf publication, made field of view controllable.
 */

#include "sicks300.h"

#include "termios.h"

SickS300::SickS300()
{

  ros::NodeHandle param_node("~");
  ros::NodeHandle nodeHandle("/");

  // reading transformation parameters from parameter server
  param_node.param(std::string("frame"), scan_data_.header.frame_id, std::string("base_laser_link"));
 
  // Reduce field of view to this number of degrees
  double fov;
  param_node.param(std::string("field_of_view"), fov, 270.0);
  field_of_view_ = (int)(fov*2.0); // angle increment is .5 degrees
  field_of_view_ <<=1; field_of_view_ >>=1; // round to a multiple of two
  start_scan_ = 270 - field_of_view_/2;
  end_scan_ = 270 + field_of_view_/2;

  scan_data_.angle_min = -(field_of_view_/4.0) / 180.f * M_PI;
  scan_data_.angle_max = (field_of_view_/4.0) / 180.f * M_PI;
  scan_data_.angle_increment = 0.5f / 180.f * M_PI;
  scan_data_.time_increment = 0;
  scan_data_.scan_time = 0.08;
  scan_data_.range_min = 0.1;
  scan_data_.range_max = 29.0;
  scan_data_.ranges.resize(field_of_view_);
  scan_data_.intensities.resize(field_of_view_);


  // Reading device parameters
  param_node.param(std::string("devicename"), device_name_, std::string("/dev/sick300"));
  param_node.param(std::string("baudrate"), baud_rate_, 500000);

  connected_ = serial_comm_.connect(device_name_, baud_rate_);

  scan_data_publisher_ = nodeHandle.advertise<sensor_msgs::LaserScan> ("scan", 10);

}

SickS300::~SickS300()
{
}

void SickS300::update()
{

  if (connected_ != 0)
  {
    connected_ = serial_comm_.connect(device_name_, baud_rate_);
  }

  if (connected_ == 0 && serial_comm_.readData() == 0)
  {

    float* ranges = serial_comm_.getRanges();
    unsigned int numRanges = serial_comm_.getNumRanges();

    for (unsigned int i = start_scan_, j=0; i < end_scan_; i++, j++)
      scan_data_.ranges[j] = ranges[i];

    scan_data_.header.stamp = ros::Time::now();

    scan_data_publisher_.publish(scan_data_);

  }

}

int main(int argc, char** argv)
{

  ros::init(argc, argv, "sicks300");
  ros::Time::init();
  ros::Rate loop_rate(20);

  ROS_INFO("Opening connection to Sick300-Laser...");

  SickS300 sickS300;

  ROS_INFO("Sick300 connected.");

  while (ros::ok())
  {

    sickS300.update();

    ros::spinOnce();
    loop_rate.sleep();
  }

  ROS_INFO("Laser shut down.");

  return 0;
}

