#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "turtlesim/msg/pose.hpp"
#include "turtlesim/srv/kill.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/srv/teleport_absolute.hpp"

static double norm_angle(double a)
{
  while (a > M_PI) a -= 2.0 * M_PI;
  while (a < -M_PI) a += 2.0 * M_PI;
  return a;
}

class TrajectoryController : public rclcpp::Node
{
public:
  TrajectoryController()
  : Node("trajectory_controller_ex5")
  {
    this->declare_parameter<std::string>("turtle_name", "turtle_Zhalgas");

    turtle_name_ = this->get_parameter("turtle_name").as_string();
    pose_topic_ = "/" + turtle_name_ + "/pose";
    cmd_topic_  = "/" + turtle_name_ + "/cmd_vel";
    teleport_srv_ = "/" + turtle_name_ + "/teleport_absolute";

    cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(cmd_topic_, 10);
    pose_sub_ = this->create_subscription<turtlesim::msg::Pose>(
      pose_topic_, 10, std::bind(&TrajectoryController::pose_cb, this, std::placeholders::_1));

    kill_client_ = this->create_client<turtlesim::srv::Kill>("/kill");
    spawn_client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");
    teleport_client_ = this->create_client<turtlesim::srv::TeleportAbsolute>(teleport_srv_);

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(50),
      std::bind(&TrajectoryController::step, this));

    // Define corners (turtlesim is roughly 0..11)
    corners_ = {
      {1.0, 1.0},   // bottom-left
      {10.0, 1.0},  // bottom-right
      {10.0, 10.0}, // top-right
      {1.0, 10.0}   // top-left
    };

    triangle_ = {
      corners_[0],         // bottom-left
      corners_[1],         // bottom-right
      {5.5, 10.0},         // top-middle
      corners_[0]          // diagonal back to start
    };

    RCLCPP_INFO(this->get_logger(), "Exercise 5 controller started (turtle: %s)", turtle_name_.c_str());
    phase_ = Phase::INIT;
  }

private:
  struct Pt { double x; double y; };

  enum class Phase {
    INIT,
    KILL_TURTLE1,
    SPAWN_CENTER,
    TELEPORT_TO_CORNER,
    SQUARE_SETUP,
    SQUARE_RUN,
    TRI_SETUP,
    TRI_RUN,
    DONE
  };

  void pose_cb(const turtlesim::msg::Pose::SharedPtr msg)
  {
    pose_ = *msg;
    have_pose_ = true;
  }

  void stop()
  {
    geometry_msgs::msg::Twist cmd;
    cmd_pub_->publish(cmd);
  }

  void set_target(const Pt& p)
  {
    target_ = p;
    target_set_ = true;
  }

  // Simple feedback controller: rotate toward target, then move forward until close
  bool drive_to_target(double pos_tol = 0.15, double ang_tol = 0.15)
  {
    if (!have_pose_ || !target_set_) return false;

    const double dx = target_.x - pose_.x;
    const double dy = target_.y - pose_.y;
    const double dist = std::sqrt(dx*dx + dy*dy);

    const double desired = std::atan2(dy, dx);
    const double err = norm_angle(desired - pose_.theta);

    geometry_msgs::msg::Twist cmd;

    if (std::fabs(err) > ang_tol) {
      cmd.angular.z = (err > 0 ? 1.5 : -1.5);
      cmd.linear.x = 0.0;
    } else if (dist > pos_tol) {
      cmd.linear.x = 1.5;
      cmd.angular.z = 0.0;
    } else {
      stop();
      return true; // reached
    }

    cmd_pub_->publish(cmd);
    return false;
  }

  void step()
  {
    // Wait for turtlesim pose stream after spawn
    switch (phase_) {
      case Phase::INIT:
        phase_ = Phase::KILL_TURTLE1;
        break;

      case Phase::KILL_TURTLE1: {
        if (!kill_client_->wait_for_service(std::chrono::milliseconds(200))) return;
        auto req = std::make_shared<turtlesim::srv::Kill::Request>();
        req->name = "turtle1";
        kill_client_->async_send_request(req);
        RCLCPP_INFO(this->get_logger(), "Requested: kill turtle1");
        phase_ = Phase::SPAWN_CENTER;
        break;
      }

      case Phase::SPAWN_CENTER: {
        if (!spawn_client_->wait_for_service(std::chrono::milliseconds(200))) return;
        auto req = std::make_shared<turtlesim::srv::Spawn::Request>();
        req->x = 5.5; req->y = 5.5; req->theta = 0.0;
        req->name = turtle_name_;
        spawn_client_->async_send_request(req);
        RCLCPP_INFO(this->get_logger(), "Requested: spawn %s at center", turtle_name_.c_str());
        phase_ = Phase::TELEPORT_TO_CORNER;
        break;
      }

      case Phase::TELEPORT_TO_CORNER: {
        if (!teleport_client_->wait_for_service(std::chrono::milliseconds(200))) return;
        auto req = std::make_shared<turtlesim::srv::TeleportAbsolute::Request>();
        req->x = corners_[0].x; req->y = corners_[0].y; req->theta = 0.0;
        teleport_client_->async_send_request(req);
        RCLCPP_INFO(this->get_logger(), "Teleported to first corner (start square)");
        phase_ = Phase::SQUARE_SETUP;
        break;
      }

      case Phase::SQUARE_SETUP:
        square_idx_ = 1; // next corner
        set_target(corners_[square_idx_]);
        phase_ = Phase::SQUARE_RUN;
        break;

      case Phase::SQUARE_RUN: {
        // corners: 0 -> 1 -> 2 -> 3 -> 0
        if (drive_to_target()) {
          square_idx_++;
          if (square_idx_ == 4) {
            set_target(corners_[0]); // back to start
            square_idx_++;
          } else if (square_idx_ > 5) {
            phase_ = Phase::TRI_SETUP;
            break;
          } else if (square_idx_ == 5) {
            // just arrived back at corner 0
            phase_ = Phase::TRI_SETUP;
            break;
          } else {
            set_target(corners_[square_idx_]);
          }
        }
        break;
      }

      case Phase::TRI_SETUP:
        tri_idx_ = 1;
        set_target(triangle_[tri_idx_]);
        RCLCPP_INFO(this->get_logger(), "Starting triangle trajectory");
        phase_ = Phase::TRI_RUN;
        break;

      case Phase::TRI_RUN:
        if (drive_to_target()) {
          tri_idx_++;
          if (tri_idx_ >= (int)triangle_.size()) {
            phase_ = Phase::DONE;
          } else {
            set_target(triangle_[tri_idx_]);
          }
        }
        break;

      case Phase::DONE:
        stop();
        RCLCPP_INFO(this->get_logger(), "DONE: square + triangle completed");
        timer_->cancel();
        break;
    }
  }

  // ROS
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;
  rclcpp::Client<turtlesim::srv::Kill>::SharedPtr kill_client_;
  rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_client_;
  rclcpp::Client<turtlesim::srv::TeleportAbsolute>::SharedPtr teleport_client_;
  rclcpp::TimerBase::SharedPtr timer_;

  // State
  std::string turtle_name_;
  std::string pose_topic_;
  std::string cmd_topic_;
  std::string teleport_srv_;

  turtlesim::msg::Pose pose_;
  bool have_pose_{false};

  std::vector<Pt> corners_;
  std::vector<Pt> triangle_;
  Pt target_{};
  bool target_set_{false};

  Phase phase_{Phase::INIT};
  int square_idx_{0};
  int tri_idx_{0};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TrajectoryController>());
  rclcpp::shutdown();
  return 0;
}
