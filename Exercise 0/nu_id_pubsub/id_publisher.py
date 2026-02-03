import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32


class NUIdPublisher(Node):
    def __init__(self):
        super().__init__('zhalgas_bolatbayev_id_publisher')

        # NU ID: 201977883 -> publish digits one by one in a loop
        self.nu_id_digits = [2, 0, 1, 9, 7, 7, 8, 8, 3]
        self.index = 0

        # Topic name = your surname
        self.publisher_ = self.create_publisher(Int32, 'bolatbayev', 10)

        # 1 Hz (change to 0.02 for 50 Hz in step 7)
        self.timer = self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        msg = Int32()
        msg.data = self.nu_id_digits[self.index]
        self.publisher_.publish(msg)
        self.get_logger().info(f'Publishing: {msg.data}')

        self.index = (self.index + 1) % len(self.nu_id_digits)


def main(args=None):
    rclpy.init(args=args)
    node = NUIdPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
