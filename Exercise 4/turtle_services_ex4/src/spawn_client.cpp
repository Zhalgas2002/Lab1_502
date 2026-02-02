#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "turtlesim/srv/spawn.hpp"

using namespace std::chrono_literals;

class SpawnClientNode : public rclcpp::Node
{
public:
  SpawnClientNode() : Node("spawn_client_ex4")
  {
    client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");

    timer_ = this->create_wall_timer(
      500ms,
      std::bind(&SpawnClientNode::try_spawn, this)
    );

    RCLCPP_INFO(this->get_logger(), "Spawn client started");
  }

private:
  void try_spawn()
  {
    if (spawned_) {
      timer_->cancel();
      return;
    }

    if (!client_->wait_for_service(200ms)) {
      RCLCPP_INFO(this->get_logger(), "Waiting for /spawn service...");
      return;
    }

    auto request = std::make_shared<turtlesim::srv::Spawn::Request>();
    request->x = 1.0;
    request->y = 5.0;
    request->theta = 0.0;
    request->name = "Turtle_Zhalgas";

    // Call only once
    timer_->cancel();

    client_->async_send_request(
      request,
      [this](rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future) {
        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "Spawned turtle name: %s", response->name.c_str());
        spawned_ = true;
      }
    );
  }

  rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool spawned_{false};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SpawnClientNode>());
  rclcpp::shutdown();
  return 0;
}
