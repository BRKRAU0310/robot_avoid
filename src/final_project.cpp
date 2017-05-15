#include <ros/ros.h>
#include <tf/tf.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/transform_listener.h>
#include <actionlib/server/simple_action_server.h>
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/PoseArray.h"
#include <cmath>
#include <stdio.h>
#include <curl/curl.h>

#include "json.hpp"
#include <boost/thread/thread.hpp>
#include <stdlib.h>
#include <iostream>

#include <vector>
#include <iostream>


using json=nlohmann::json;
using namespace std;
CURLcode res;
string data;
double myX;
double myY;
double otherBotX;
double otherBotY;


size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up){
	
	data = "";
	for(int c=0;c<size*nmemb;c++){
		data.push_back(buf[c]);
	}

	return size*nmemb;
}



void help_Curl(){

	CURL *curl;
	//CURLcode res;

	curl = curl_easy_init();
	ros::Rate curl_rate(1.0f /4);
	bool once = true;
	
	while(ros::ok() && once){

	ros::spinOnce();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, "http://nixons-head.csres.utexas.edu:7978/json");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		//cout << endl<< data <<endl;


		curl_easy_cleanup(curl);




	}
	once = false;
	curl_rate.sleep();
	}

	

}

 typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void getCoords(){

	ros::NodeHandle n;
	help_Curl();

	std::string::iterator it = data.begin();
	auto j3 = json::parse(data);

	std::vector <std::pair<double,double>>pos;

	for(json::iterator it = j3.begin(); it != j3.end();++it){
	json obj = *it;

	string xtemp = obj["X"];
	string ytemp = obj["Y"];
	
	double otherX = atof(xtemp.c_str());
	double otherY = atof(ytemp.c_str());
	
	
	pos.push_back({otherX,otherY});
	//pos.push_back({obj["X"], obj["Y"]});

	}

	//cout <<"size: " << pos.size()<<endl;

/*	for(int i=0; i<pos.size();i++){
	cout <<"X: " << pos.at(i).first << " Y: " << pos.at(i).second<<endl;
	
	}*/
	
	//set the x and y coords of both bots
	//first bot
	myX = pos.at(0).first;
	myY = pos.at(0).second;
	//second bot
	otherBotX = pos.at(1).first;
	otherBotY = pos.at(1).second;
	




}

int main(int argc, char **argv)
{
  	ros::init(argc, argv, "client");
/*	ros::NodeHandle n;
	help_Curl();

	std::string::iterator it = data.begin();
	auto j3 = json::parse(data);

	std::vector <std::pair<double,double>>pos;

	for(json::iterator it = j3.begin(); it != j3.end();++it){
	json obj = *it;

	string xtemp = obj["X"];
	string ytemp = obj["Y"];
	
	double otherX = atof(xtemp.c_str());
	double otherY = atof(ytemp.c_str());
	
	
	pos.push_back({otherX,otherY});
	//pos.push_back({obj["X"], obj["Y"]});

	}

	cout <<"size: " << pos.size()<<endl;

	for(int i=0; i<pos.size();i++){
	cout <<"X: " << pos.at(i).first << "Y: " << pos.at(i).second<<endl;
	
	}
	
	//set the x and y coords of both bots
	//first bot
	double myX = pos.at(0).first;
	double myY = pos.at(0).second;
	//second bot
	double otherBotX = pos.at(1).first;
	double otherBotY = pos.at(1).second; */
	
	//does the above code that is commented out
	getCoords();

	MoveBaseClient ac("move_base", true);


	//while(!ac.waitForServer(ros::Duration(5.0))){
	//ROS_INFO("Waiting for the move_base action server to come up");}
 
  move_base_msgs::MoveBaseGoal goal;

  int buffer = 1;
  int range = 10;
  bool same_Area = false;
	

 if(std::abs(myX-otherBotX) <=range && std::abs(myY-otherBotY)<=range){
	same_Area = true;
   }
  
  
  while (ros::ok() && same_Area)
  {
    
    //set the header
	goal.target_pose.header.frame_id = "/base_link";
    goal.target_pose.header.stamp = ros::Time::now();

	//moves the robot
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(0);
	goal.target_pose.pose.position.x = buffer;

	//changes x position first
	ac.sendGoal(goal);
	ac.waitForResult();

	goal.target_pose.pose.position.y = buffer;
	goal.target_pose.pose.position.z = 0.0;
	
	//updates x and y coordinates of both robots
	getCoords();
    

    ac.sendGoal(goal);
    ac.waitForResult();

    same_Area = std::abs(myX-otherBotX) <=buffer && std::abs(myY-otherBotY)<=buffer;

  }
  return 0;

}
