#include "ros/ros.h"
#include "geometry_msgs/PoseStamped.h"

#include <fetch_custom_msgs/CartesianControls.h>

#include <moveit/move_group_interface/move_group.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>

#include <moveit_msgs/DisplayRobotState.h>
#include <moveit_msgs/DisplayTrajectory.h>

#include <moveit_msgs/AttachedCollisionObject.h>
#include <moveit_msgs/CollisionObject.h>

float update_rate = 15.0;
float update_gains [3] = {0.05, 0.05, 0.05};
float control_signals [3] = {0.0, 0.0, 0.0};

static const std::string PLANNING_GROUP = "arm_with_torso";
moveit::planning_interface::MoveGroup* move_group;
moveit::planning_interface::PlanningSceneInterface* planning_scene_interface;
const robot_state::JointModelGroup* joint_model_group;

void addCollisionObjects()
{
	std::vector<moveit_msgs::CollisionObject> collision_objects;
	//front ground
	moveit_msgs::CollisionObject c_o1;
	c_o1.header.frame_id = move_group->getPlanningFrame();
	c_o1.id = "front_ground";
	shape_msgs::SolidPrimitive primitive1;
	primitive1.type = primitive1.BOX;
	primitive1.dimensions.resize(3);
	primitive1.dimensions[0] = 2;
	primitive1.dimensions[1] = 2;
	primitive1.dimensions[2] = 2;
	geometry_msgs::Pose box_pose1;
	box_pose1.orientation.w = 1.0;
	box_pose1.position.x =  1.1;
	box_pose1.position.y = 0.0;
	box_pose1.position.z =  -1.0;
	c_o1.primitives.push_back(primitive1);
	c_o1.primitive_poses.push_back(box_pose1);
	c_o1.operation = c_o1.ADD;

	collision_objects.push_back(c_o1);

	//back ground
	moveit_msgs::CollisionObject c_o2;
	c_o2.header.frame_id = move_group->getPlanningFrame();
	c_o2.id = "back_ground";
	shape_msgs::SolidPrimitive primitive2;
	primitive2.type = primitive2.BOX;
	primitive2.dimensions.resize(3);
	primitive2.dimensions[0] = 2;
	primitive2.dimensions[1] = 2;
	primitive2.dimensions[2] = 2;
	geometry_msgs::Pose box_pose2;
	box_pose2.orientation.w = 1.0;
	box_pose2.position.x =  -1.2;
	box_pose2.position.y = 0.0;
	box_pose2.position.z =  -1.0;
	c_o2.primitives.push_back(primitive2);
	c_o2.primitive_poses.push_back(box_pose2);
	c_o2.operation = c_o2.ADD;

	collision_objects.push_back(c_o2);

	//left ground
	moveit_msgs::CollisionObject c_o3;
	c_o3.header.frame_id = move_group->getPlanningFrame();
	c_o3.id = "left_ground";
	shape_msgs::SolidPrimitive primitive3;
	primitive3.type = primitive3.BOX;
	primitive3.dimensions.resize(3);
	primitive3.dimensions[0] = 2;
	primitive3.dimensions[1] = 2;
	primitive3.dimensions[2] = 2;
	geometry_msgs::Pose box_pose3;
	box_pose3.orientation.w = 1.0;
	box_pose3.position.x =  0.0;
	box_pose3.position.y = 1.2;
	box_pose3.position.z =  -1.0;
	c_o3.primitives.push_back(primitive3);
	c_o3.primitive_poses.push_back(box_pose3);
	c_o3.operation = c_o3.ADD;

	collision_objects.push_back(c_o3);

	//right_ground
	moveit_msgs::CollisionObject c_o4;
	c_o4.header.frame_id = move_group->getPlanningFrame();
	c_o4.id = "front_ground";
	shape_msgs::SolidPrimitive primitive4;
	primitive4.type = primitive4.BOX;
	primitive4.dimensions.resize(3);
	primitive4.dimensions[0] = 2;
	primitive4.dimensions[1] = 2;
	primitive4.dimensions[2] = 2;
	geometry_msgs::Pose box_pose4;
	box_pose4.orientation.w = 1.0;
	box_pose4.position.x =  0.0;
	box_pose4.position.y = -1.2;
	box_pose4.position.z =  -1.0;
	c_o4.primitives.push_back(primitive4);
	c_o4.primitive_poses.push_back(box_pose4);
	c_o4.operation = c_o4.ADD;

	collision_objects.push_back(c_o4);
	
	planning_scene_interface->addCollisionObjects(collision_objects);
}

void updateControlSignal(const fetch_custom_msgs::CartesianControls::ConstPtr& msg)
{
	control_signals[0] = msg->x_axis;
	control_signals[1] = msg->y_axis;
	control_signals[2] = msg->z_axis;
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "cartesian_keyboard_controller");
	ros::NodeHandle nh;

	move_group = new moveit::planning_interface::MoveGroup(PLANNING_GROUP);
	planning_scene_interface = new moveit::planning_interface::PlanningSceneInterface();
	joint_model_group = move_group->getCurrentState()->getJointModelGroup(PLANNING_GROUP);

	addCollisionObjects();
	ros::Subscriber sub = nh.subscribe("control_signal", 1, updateControlSignal);
	ros::AsyncSpinner spinner(2);
	spinner.start();

	ros::Rate r(30); //30 hz
	moveit::planning_interface::MoveGroup::Plan t_plan;
	bool success = false;
	move_group->setPlanningTime(0.05);
	move_group->setMaxVelocityScalingFactor(0.5);
	while(ros::ok())
	{
		if(control_signals[0] == 0.0 && control_signals[1] == 0.0 && control_signals[2] == 0.0){
			;
		} else 
		{
			if(success)
			{
				//move_group->stop();
				move_group->asyncExecute(t_plan);

			}
			geometry_msgs::PoseStamped c_pose = move_group->getCurrentPose("wrist_roll_link");

			geometry_msgs::Pose target_pose;

			//copy over orientation
			target_pose.orientation.w = c_pose.pose.orientation.w;
			target_pose.orientation.x = c_pose.pose.orientation.x;
			target_pose.orientation.y = c_pose.pose.orientation.y;
			target_pose.orientation.z = c_pose.pose.orientation.z;

			//update position according to control input
			target_pose.position.x = c_pose.pose.position.x + control_signals[0]*update_gains[0];
			target_pose.position.y = c_pose.pose.position.y + control_signals[1]*update_gains[1];
			target_pose.position.z = c_pose.pose.position.z + control_signals[2]*update_gains[2];
			
			move_group->setPoseTarget(target_pose, "wrist_roll_link");
			success = move_group->plan(t_plan);
		}
		r.sleep();
		//ros::spinOnce();
	}
}