#include <cstdio>
#include "rclcpp/rclcpp.hpp"

#include "opencv2/opencv.hpp"

//add include for manipulate sensor_msgs::msgs::Image type:
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"

//TODO add #include for output topic type (geometry_msgs::msg::Twist)
#include "geometry_msgs/msg/twist.hpp"

//for binarisation function:
#include "DetectionCnam_codels.hpp"

rclcpp::Node::SharedPtr ptr_node = nullptr;

rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_;
rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;

static const std::string OPENCV_WINDOW = "Image window";


void getImage(const sensor_msgs::msg::Image::SharedPtr _msg)
{
    rclcpp::Time begin = ptr_node->get_clock()->now();

    //RCLCPP_INFO(ptr_node->get_logger(),"I have received image! ;-)");

    int b;
    int g;
    int r;
    int seuil;

    //necessary for transform ros image type into opencv image type
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(_msg, sensor_msgs::image_encodings::BGR8);
        RCLCPP_INFO(ptr_node->get_logger(),"I have received image! ;-)");
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(ptr_node->get_logger(),"cv_bridge exception: %s", e.what());
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
    cv::imshow(OPENCV_WINDOW, cv_ptr->image);
    cv::waitKey(3);

    ptr_node->get_parameter("blue", b);
    ptr_node->get_parameter("green", g);
    ptr_node->get_parameter("red", r);
    ptr_node->get_parameter("seuil", seuil);
    //RCLCPP_INFO(ptr_node->get_logger(),"parameter b, g, r, seuil: %d, %d, %d, %d", b, g, r, seuil);

    //declare a CvPoint
    CvPoint point;
    geometry_msgs::msg::Twist cmd;

    //call binarisation method!
    point = binarisation(ptr_ipl_img, b, g, r, seuil);
    if (point.x != -1 || point.y != -1) {
        RCLCPP_INFO(ptr_node->get_logger(), "x = %d, y = %d", point.x, point.y);
        float cibleY = ptr_ipl_img->height*3/4;
        float cmd_x_pixel_value = 2.0 / ptr_ipl_img->width;
        float cmd_y_pixel_value = 2.0/(ptr_ipl_img->height - cibleY);
        cmd.angular.z = - ((point.x * cmd_x_pixel_value) - 1.0);
        cmd.linear.x = - ((point.y - cibleY) * cmd_y_pixel_value);

        RCLCPP_INFO(ptr_node->get_logger(),"cibleY: %f", cibleY);
        RCLCPP_INFO(ptr_node->get_logger(),"cmd.linear.x: %f",cmd.linear.x);
        RCLCPP_INFO(ptr_node->get_logger(),"cmd.angular.z: %f",cmd.angular.z);

    }
    else {
        cmd.angular.z = 0.0;
        cmd.angular.x = 0.0;
    }
    //TODO send command in output topic.
    pub_->publish(cmd);

    rclcpp::Time end = ptr_node->get_clock()->now();
    RCLCPP_INFO(ptr_node->get_logger(), "begin : %ld", begin.nanoseconds());
    RCLCPP_INFO(ptr_node->get_logger(), "end : %ld", end.nanoseconds());
    RCLCPP_INFO(ptr_node->get_logger(), "duration : %ld (ns) %ld (ms)", (end.nanoseconds() - begin.nanoseconds()), ((end.nanoseconds() - begin.nanoseconds()))/1000000);
}



int main(int argc, char ** argv)
{
    (void) argc;
    (void) argv;
    rclcpp::init(argc, argv);

    ptr_node = rclcpp::Node::make_shared("detection_cnam");

    auto sensor_qos = rclcpp::QoS(rclcpp::SensorDataQoS());

    sub_ = ptr_node->create_subscription<sensor_msgs::msg::Image>("/image", sensor_qos,getImage);
    //auto subscription = ptr_node->create_subscription<sensor_msgs::msg::Image>("topic", 10, getImage);

    //instantiate publisher topic for command output (geometry_msgs::msg::Twist):
    //must be global for visibility in callback function.
    pub_ = ptr_node->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 1);

    ptr_node->declare_parameter<std::uint8_t>("blue",  180);
    ptr_node->declare_parameter<std::uint8_t>("green", 100);
    ptr_node->declare_parameter<std::uint8_t>("red",   50);
    ptr_node->declare_parameter<std::uint8_t>("seuil", 30);

    rclcpp::spin(ptr_node);
    rclcpp::shutdown();
    return 0;
}
