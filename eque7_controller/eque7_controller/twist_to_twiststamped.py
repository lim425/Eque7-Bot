#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, TwistStamped

class TwistToTwistStamped(Node):
    def __init__(self):
        super().__init__("twist_to_twiststamped")
        # Subscribe to Twist messages
        self.sub_ = self.create_subscription(
            Twist, "cmd_vel", self.twist_callback, 10)
        # Publish TwistStamped messages
        self.pub_ = self.create_publisher(
            TwistStamped, "eque7_controller/cmd_vel", 10)
        self.get_logger().info("Converting Twist to TwistStamped")

    def twist_callback(self, msg):
        twist_stamped = TwistStamped()
        twist_stamped.header.stamp = self.get_clock().now().to_msg()
        twist_stamped.header.frame_id = "base_footprint"
        twist_stamped.twist = msg
        self.pub_.publish(twist_stamped)

def main():
    rclpy.init()
    node = TwistToTwistStamped()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()