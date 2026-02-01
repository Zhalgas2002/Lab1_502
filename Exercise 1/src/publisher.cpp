#include <chrono>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

using namespace std::chrono_literals;

class NuIdPublisher : public rclcpp::Node
{
public:
  NuIdPublisher()
  : Node("zhalgas_bolatbayev_talker"), digits_{2,0,1,9,7,7,8,8,3}, idx_(0)
  {
    // Topic name = surname
    publisher_ = this->create_publisher<std_msgs::msg::Int32>("bolatbayev", 10);

    // Rate parameter: default 1 Hz, can set to 50 via --ros-args -p rate_hz:=50
    this->declare_parameter<double>("rate_hz", 1.0);
    const double rate_hz = this->get_parameter("rate_hz").as_double();
    const auto period = std::chrono::duration<double>(1.0 / rate_hz);

    timer_ = this->create_wall_timer(
      std::chrono::duration_cast<std::chrono::nanoseconds>(period),
      std::bind(&NuIdPublisher::on_timer, this)
    );

    RCLCPP_INFO(this->get_logger(), "Publishing NU ID digits on topic '/bolatbayev' at %.2f Hz", rate_hz);
  }

private:
  void on_timer()
  {
    std_msgs::msg::Int32 msg;
    msg.data = digits_[idx_];
    publisher_->publish(msg);

    RCLCPP_INFO(this->get_logger(), "Publishing: %d", msg.data);

    idx_ = (idx_ + 1) % digits_.size();
  }

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::vector<int> digits_;
  std::size_t idx_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NuIdPublisher>());
  rclcpp::shutdown();
  return 0;
}
