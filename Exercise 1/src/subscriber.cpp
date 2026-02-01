#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

class NuIdSubscriber : public rclcpp::Node
{
public:
  NuIdSubscriber()
  : Node("zhalgas_bolatbayev_listener")
  {
    subscription_ = this->create_subscription<std_msgs::msg::Int32>(
      "bolatbayev",
      10,
      std::bind(&NuIdSubscriber::callback, this, std::placeholders::_1)
    );

    RCLCPP_INFO(this->get_logger(), "Subscribing to topic '/bolatbayev'");
  }

private:
  void callback(const std_msgs::msg::Int32 & msg) const
  {
    RCLCPP_INFO(this->get_logger(), "I heard: %d", msg.data);
  }

  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NuIdSubscriber>());
  rclcpp::shutdown();
  return 0;
}
