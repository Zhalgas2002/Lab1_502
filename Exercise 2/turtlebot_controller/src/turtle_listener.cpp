#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"

class TurtlebotSubscriber : public rclcpp::Node
{
public:
  TurtlebotSubscriber() : Node("turtlebot_subscriber")
  {
    sub_ = this->create_subscription<turtlesim::msg::Pose>(
      "/turtle1/pose",
      10,
      std::bind(&TurtlebotSubscriber::turtleCallback, this, std::placeholders::_1)
    );
  }

private:
  void turtleCallback(const turtlesim::msg::Pose::SharedPtr msg) const
  {
    RCLCPP_INFO(
      this->get_logger(),
      "Turtle subscriber@[%.3f, %.3f, %.3f]",
      msg->x, msg->y, msg->theta
    );
  }

  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr sub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurtlebotSubscriber>());
  rclcpp::shutdown();
  return 0;
}

