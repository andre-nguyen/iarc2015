/************************************************************************
* Copyright(c) 2014  YING Jiahang
* All rights reserved.
*
* File:	uav_state_controller.cpp
* Brief: 
* Version: 1.0
* Author: YING Jiahang
* Email: yingjiahang@gmail.com
* Date:	2014/9/4 16:06
* History:
************************************************************************/

// ros
#include "ros/ros.h"
// msg
#include <ukftest/avoidCtrl.h>
#include <ukftest/trackCtrl.h>
#include <ukftest/randomCtrl.h>
#include <ukftest/Ctrl.h>
#include <geometry_msgs/Quaternion.h>
#include <std_msgs/Float32.h>

using namespace std;
using namespace ros;

// Ros
ros::Subscriber avoidsub, tracksub, randomsub, landsub ,boundarysub, takeoffsub, statussub, opticalflowsub;
ros::Publisher ctrlpub, statuspub , requestpub ;

ros::Timer stateTimer;
int trackCnt = 0;
int randomCnt = 0;
int takeoffCnt = 0;
int allTimeCnt = 0 ;
int pitchCnt = 0 ;
int uav_status = 0;
int outCnt = 0;
int avoidCnt = 0;
int landCnt = 0;
int hoverCnt = 0;
int opticalflowCnt = 0;

//state msg
bool isAvoid = false;
int avoid_z, avoid_x, avoid_y, avoid_w;

bool isTrack = false;
int track_z, track_x, track_y, track_w;

bool isRandom = false;
int random_z, random_x, random_y, random_w;

bool isOut = false;
int boundary_z, boundary_x, boundary_y, boundary_w;

bool isTakeoff = false;
int takeoff_z, takeoff_x, takeoff_y, takeoff_w; 

bool isOpticalflow = false;
int opticalflow_z, opticalflow_x, opticalflow_y, opticalflow_w;

bool isLand = false;


double freq;
double yawRate;

bool isDebug = true;
// callback func
void avoidCallback(const ukftest::avoidCtrl& avoidMsg);
void trackCallback(const ukftest::Ctrl& trackMsg);
void randomCallback(const ukftest::randomCtrl& randomMsg);
void landCallback(const std_msgs::Float32& landMsg);
void takeoffCallback(const ukftest::Ctrl& takeoffMsg);
void boundaryCallback(const ukftest::Ctrl& boundaryMsg);
void opticalflowCallback(const ukftest::Ctrl& opticalflowMsg);

void spinCallback(const ros::TimerEvent& e);

void statusCallback(const std_msgs::Float32& statusMsg);

int main (int argc, char** argv)
{
	// ros init and parameters retrieve
	ros::init(argc, argv, "uav_state_controller_node");
	ros::NodeHandle nh;

	ctrlpub = nh.advertise<geometry_msgs::Quaternion>("board_ctrl",10);
	statuspub = nh.advertise<std_msgs::Float32>("uav_flight_control_status",10);
	requestpub = nh.advertise<std_msgs::Float32>("uav_flight_status_request",10);

	statussub   = nh.subscribe("uav_flight_status", 10, statusCallback);
	avoidsub    = nh.subscribe("laser_ctrl" , 10, avoidCallback);
	takeoffsub  = nh.subscribe("takeoff_ctrl" , 10, takeoffCallback);
	tracksub    = nh.subscribe("vicon_followme_ctrl" , 10, trackCallback);
	randomsub   = nh.subscribe("random_ctrl", 10, randomCallback);
	landsub     = nh.subscribe("landing_ctrl", 10, landCallback);
	boundarysub = nh.subscribe("boundary_ctrl", 10, boundaryCallback);
	opticalflowsub = nh.subscribe("opticalflow_ctrl", 10, opticalflowCallback);
 
	nh.param("freq", freq, 30.0); 
	nh.param("yaw_rate", yawRate ,0.0);
		
	stateTimer = nh.createTimer(ros::Duration(1.0/max(freq,1.0)), spinCallback);

	cout<< "uav state controller start!"<<endl;
	ros::spin();
	cout<< "uav state controller shutdown!"<<endl;
	return 0;
}

void avoidCallback(const ukftest::avoidCtrl& avoidMsg)
{
	if(avoidMsg.isBlocking == 1)
	{
		isAvoid = true;
		avoid_z = avoidMsg.ctrl.z;	//yaw_rate 
		avoid_x = avoidMsg.ctrl.x;	//pitch
		avoid_y = avoidMsg.ctrl.y;	//roll
		avoid_w = avoidMsg.ctrl.w;	//vel
	}
	else
	{
		isAvoid = false;
		avoid_z = 0;	//yaw_rate 
		avoid_x = 0;	//pitch
		avoid_y = 0;	//roll
		avoid_w = 0;	//vel
	}
}

void trackCallback(const ukftest::Ctrl& trackMsg)
{
	if(trackMsg.isCtrl == 1)
	{
		isTrack = true;
		track_z = trackMsg.ctrl.z;	//yaw_rate 
		track_x = trackMsg.ctrl.x;	//pitch
		track_y = trackMsg.ctrl.y;	//roll
		track_w = trackMsg.ctrl.w;	//vel
	}
	else
	{
		isTrack = false;
		track_z = 0;	//yaw_rate 
		track_x = 0;	//pitch
		track_y = 0;	//roll
		track_w = 0;	//vel
	}

}

void randomCallback(const ukftest::randomCtrl& randomMsg)
{
	if(randomMsg.isRandflying == 1)
	{
		isRandom = true;
		random_z = randomMsg.ctrl.z;	//yaw_rate 
		random_x = randomMsg.ctrl.x;	//pitch
		random_y = randomMsg.ctrl.y;	//roll
		random_w = randomMsg.ctrl.w;	//vel
	}
	else
	{
		isRandom = false;
		random_z = 0;	//yaw_rate 
		random_x = 0;	//pitch
		random_y = 0;	//roll
		random_w = 0;	//vel
	}

}

void boundaryCallback(const ukftest::Ctrl& boundaryMsg)
{
	if(boundaryMsg.isCtrl == 1)
	{
		isOut = true;
		boundary_z = boundaryMsg.ctrl.z;	//yaw_rate 
		boundary_x = boundaryMsg.ctrl.x;	//pitch
		boundary_y = boundaryMsg.ctrl.y;	//roll
		boundary_w = boundaryMsg.ctrl.w;	//vel
	}
	else
	{
		isOut = false;
		boundary_z = 0;	//yaw_rate 
		boundary_x = 0;	//pitch
		boundary_y = 0;	//roll
		boundary_w = 0;	//vel
	}
}

void takeoffCallback(const ukftest::Ctrl& takeoffMsg)
{
	if(takeoffMsg.isCtrl == 1)
	{
		isTakeoff = true;
		takeoff_z = takeoffMsg.ctrl.z;	//yaw_rate 
		takeoff_x = takeoffMsg.ctrl.x;	//pitch
		takeoff_y = takeoffMsg.ctrl.y;	//roll
		takeoff_w = takeoffMsg.ctrl.w;	//vel
	}
	else
	{
		isTakeoff = false;
		takeoff_z = 0;	//yaw_rate 
		takeoff_x = 0;	//pitch
		takeoff_y = 0;	//roll
		takeoff_w = 0;	//vel
	}
}

void opticalflowCallback(const ukftest::Ctrl& opticalflowMsg)
{
	if(opticalflowMsg.isCtrl == 1)
	{
		isOpticalflow = true;
		opticalflow_z = opticalflowMsg.ctrl.z;	//yaw_rate 
		opticalflow_x = opticalflowMsg.ctrl.x;	//pitch
		opticalflow_y = opticalflowMsg.ctrl.y;	//roll
		opticalflow_w = opticalflowMsg.ctrl.w;	//vel
	}
	else
	{
		isOpticalflow = false;
		opticalflow_z = 0;	//yaw_rate 
		opticalflow_x = 0;	//pitch
		opticalflow_y = 0;	//roll
		opticalflow_w = 0;	//vel
	}
}

void landCallback(const std_msgs::Float32& landMsg)
{
	if(landMsg.data == 1)
	{
		isLand = true;
	}
	else
	{
		isLand = false;
	}
}



void spinCallback(const ros::TimerEvent& e)
{
	geometry_msgs::Quaternion outMsg;
	std_msgs::Float32 statusMsg;
	std_msgs::Float32 requestMsg;
	allTimeCnt++;

	if(isAvoid)
	{
		avoidCnt++;
		outMsg.z = (int)avoid_z;         //yaw_rate  
		outMsg.x = (int)avoid_x;         //pitch
		outMsg.y = (int)avoid_y;         //roll
		outMsg.w = (int)avoid_w;         //vel
		cout<< "state: Avoiding!" <<endl;
		statusMsg.data = 1.0;
		statuspub.publish(statusMsg);
	}

	else if(isLand)
	{
		landCnt++;
		outMsg.z = (int)0;         //yaw_rate  
		outMsg.x = (int)0;         //pitch
		outMsg.y = (int)0;         //roll
		outMsg.w = (int)0;         //vel
		cout<< "state: Landing!" <<endl;
		statusMsg.data = 5.0;
		statuspub.publish(statusMsg);
		requestMsg.data = 4.0;
		requestpub.publish(requestMsg);
		
	}
	else if(isOut)
	{
		outCnt++;
		outMsg.z = (int)boundary_z;         //yaw_rate  
		outMsg.x = (int)boundary_x;         //pitch
		outMsg.y = (int)boundary_y;         //roll
		outMsg.w = (int)boundary_w;         //vel
		cout<< "state: outBoundary!" <<endl;
		statusMsg.data = 6.0;
		statuspub.publish(statusMsg);
		
	}
	else if(isTrack)
	{
		trackCnt++;
		outMsg.z = (int)track_z;         //yaw_rate  
		outMsg.x = (int)track_x;         //pitch
		outMsg.y = (int)track_y;         //roll
		outMsg.w = (int)track_w;         //vel
		cout<< "state: Tracking!" <<endl;
		statusMsg.data = 2.0;
		statuspub.publish(statusMsg);
	}
	else if(isOpticalflow)
	{
		opticalflowCnt++;
		outMsg.z = (int)opticalflow_z;         //yaw_rate  
		outMsg.x = (int)opticalflow_x;         //pitch
		outMsg.y = (int)opticalflow_y;         //roll
		outMsg.w = (int)opticalflow_w;         //vel
		cout<< "state: opticalflow!" <<endl;
		statusMsg.data = 7.0;
		statuspub.publish(statusMsg);
	}
	else if(isRandom)
	{
		randomCnt++;
		outMsg.z = (int)random_z;         //yaw_rate  
		outMsg.x = (int)random_x;         //pitch
		outMsg.y = (int)random_y;         //roll
		outMsg.w = (int)random_w;         //vel
		cout<< "state: Rand-Flying!" <<endl;
		statusMsg.data = 3.0;
		statuspub.publish(statusMsg);
	}
	else 
	{
		hoverCnt++;
		outMsg.z = (int)(yawRate);         //yaw_rate  
		outMsg.x = (int)(0);         //pitch
		outMsg.y = (int)(0);         //roll
		outMsg.w = (int)(0);         //vel
		cout<< "state: Hovering!" <<endl;
		statusMsg.data = 4.0;
		statuspub.publish(statusMsg);
	}
	ctrlpub.publish(outMsg);
	
	if(isDebug)
	{
		cout<< "yaw rate  :" << outMsg.z << endl;
		cout<< "pitch     :" << outMsg.x << endl;
		cout<< "roll      :" << outMsg.y << endl;
		cout<< "vel       :" << outMsg.w << endl;
	}
}

void statusCallback(const std_msgs::Float32& statusMsg)
{
	uav_status= statusMsg.data;
}
