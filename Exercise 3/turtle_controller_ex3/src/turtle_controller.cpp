#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"

class TurtleControllerEx3 : public rclcpp::Node
{
public:
  TurtleControllerEx3() : Node("turtle_controller_ex3")
  {
    vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

    pose_sub_ = this->create_subscription<turtlesim::msg::Pose>(
      "/turtle1/pose",
      10,
      std::bind(&TurtleControllerEx3::pose_cb, this, std::placeholders::_1)
    );

    RCLCPP_INFO(this->get_logger(), "Exercise 3 controller running");
  }

private:
  void pose_cb(const turtlesim::msg::Pose::SharedPtr msg)
  {
    RCLCPP_INFO(this->get_logger(), "Pose: x=%.2f y=%.2f theta=%.2f", msg->x, msg->y, msg->theta);

    geometry_msgs::msg::Twist cmd;
    cmd.linear.x = 1.0;   // forward
    cmd.angular.z = 1.0;  // turn
    vel_pub_->publish(cmd);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr vel_pub_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurtleControllerEx3>());
  rclcpp::shutdown();
  return 0;
}
