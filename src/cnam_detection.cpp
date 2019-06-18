#include "ros/ros.h"
#include "std_msgs/String.h"

//add include for manipulate sensor_msgs::Image type:
#include "sensor_msgs/Image.h"
#include <cv_bridge/cv_bridge.h>
#include "opencv2/opencv.hpp"

//TODO add #include for output topic type (geometry_msgs/Twist)

//for binarisation function:
#include "DetectionCnam_codels.hpp"

static const std::string OPENCV_WINDOW = "Image window";

//TODO declare publisher topic for command output:


void cnam_image_Callback(const sensor_msgs::Image::ConstPtr& msg)
{
  //necessary for transform ros image type into opencv image type
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    ROS_INFO("I have received image! ;-)");
    IplImage _ipl_img=cv_ptr->image;
    IplImage *ptr_ipl_img= &_ipl_img;
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }

  //TODO  For see OpenCV Image:
  //  // Update GUI Window
  //  cv::imshow(OPENCV_WINDOW, cv_ptr->image);
  //  cv::waitKey(3);

  //TODO declare a CvPoint
  //TODO call binarisation method!
  //TODO print with ROS_INFO coordinate points.x points.y
  //TODO calculate command to send
  //TODO send command in output topic.

}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "cnam_detection");
  ros::NodeHandle nh;

  //TODO  //  For see OpenCV Image:
  //  cv::namedWindow(OPENCV_WINDOW);

  ros::Subscriber sub = nh.subscribe("cnam_image", 1, cnam_image_Callback);

  //instantiate publisher topic for command output (geometry_msgs/Twist):
  //must be global for visibility in callback function.

  ros::spin();

  //cv::destroyWindow(OPENCV_WINDOW);
  return 0;
}
