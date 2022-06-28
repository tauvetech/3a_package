#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int32.h"

//add include for manipulate sensor_msgs::Image type:
#include "sensor_msgs/Image.h"
#include <cv_bridge/cv_bridge.h>
#include "opencv2/opencv.hpp"

//TODO add #include for output topic type (geometry_msgs/Twist)
#include "geometry_msgs/Twist.h"

//for binarisation function:
#include "DetectionCnam_codels.hpp"

static const std::string OPENCV_WINDOW = "Image window";

//TODO declare publisher topic for command output:
ros::Publisher pub;

int32_t _r;
int32_t _g;
int32_t _b;
int32_t _seuil;

ros::NodeHandle *nh;

void cnam_image_Callback(const sensor_msgs::Image::ConstPtr& msg)
{
  ros::Time begin = ros::Time::now();
  //usleep(10000);
//  ros::Time end = ros::Time::now();

  //necessary for transform ros image type into opencv image type
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    ROS_INFO("I have received image! ;-)");
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }
#if CV_VERSION_MAJOR == 4
  IplImage _ipl_img=cvIplImage(cv_ptr->image);
#else
  IplImage _ipl_img=cv_ptr->image;
#endif
  IplImage *ptr_ipl_img= &_ipl_img;


  //For see OpenCV Image:
  //
  // Update GUI Window
  //  cv::imshow(OPENCV_WINDOW, cv_ptr->image);
  //  cv::waitKey(3);

  geometry_msgs::Twist cmd;

  //declare a CvPoint
  CvPoint coord;

  nh->getParam("my_r", _r);
  nh->getParam("my_g", _g);
  nh->getParam("my_b", _b);
  nh->getParam("my_seuil", _seuil);

  //call binarisation method!
  coord = binarisation(ptr_ipl_img, _b, _g, _r,_seuil);

  if (coord.x > -1)
  {

  //print with ROS_INFO coordinate points.x points.y
  ROS_INFO("coord X: %d",coord.x);
  ROS_INFO("coord Y: %d",coord.y);

  ROS_INFO("_r: %d", _r);
  ROS_INFO("_g: %d", _g);
  ROS_INFO("_b: %d", _b);
  ROS_INFO("_seuil: %d", _seuil);


  //TODO calculate command to send:

  float cibleY = ptr_ipl_img->height * 3 / 4;

  float cmd_x_pixel_value= 2.0 / ptr_ipl_img->width;
  float cmd_y_pixel_value= 2.0 / (ptr_ipl_img->height - cibleY);

  cmd.angular.z= - ((coord.x * cmd_x_pixel_value) -1.0);
  cmd.linear.x = - ((coord.y - cibleY) * cmd_y_pixel_value);

//  //cmd.angular.z=1.0;
//  if (coord.y > cibleY)
//    cmd.linear.x = -0.5;
//  else
//    cmd.linear.x = 0.5;

  //ROS_INFO("cibleY: %f", cibleY);
  ROS_INFO("cmd.linear.x: %f",cmd.linear.x);
  ROS_INFO("cmd.angular.z: %f",cmd.angular.z);
  }
  else
  {
    cmd.angular.z=0.0;
    cmd.linear.x=0.0;
  }
  //TODO send command in output topic.
  pub.publish(cmd);
  ros::Time end = ros::Time::now();

  ROS_INFO("begin: %d",begin.toNSec());
  ROS_INFO("end: %d",end.toNSec());
  ROS_INFO("duration: %d", (end.toNSec() - begin.toNSec()));

}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "cnam_detection");

  nh = new ros::NodeHandle;

  //TODO  //  For see OpenCV Image:
  //  cv::namedWindow(OPENCV_WINDOW);

  ros::Subscriber sub = nh->subscribe("cnam_input_image", 1, cnam_image_Callback);

  //instantiate publisher topic for command output (geometry_msgs/Twist):
  //must be global for visibility in callback function.
  pub = nh->advertise<geometry_msgs::Twist>("cnam_output_command", 1);

  nh->setParam("my_r", 213);
  nh->setParam("my_g", 103);
  nh->setParam("my_b", 4);
  nh->setParam("my_seuil", 40);

  ros::spin();

  //cv::destroyWindow(OPENCV_WINDOW);
  return 0;
}
