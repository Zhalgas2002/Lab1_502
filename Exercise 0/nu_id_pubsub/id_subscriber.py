import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32


class NUIdSubscriber(Node):
    def __init__(self):
        super().__init__('zhalgas_bolatbayev_id_subscriber')

        self.subscription = self.create_subscription(
            Int32,
            'bolatbayev',  # topic = surname
            self.listener_callback,
            10
        )

    def listener_callback(self, msg):
        self.get_logger().info(f'Received: {msg.data}')


def main(args=None):
    rclpy.init(args=args)
    node = NUIdSubscriber()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
