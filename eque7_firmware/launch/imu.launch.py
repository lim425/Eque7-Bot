import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node


def generate_launch_description():


    # ---------------- IMU Driver ----------------
    imu_driver = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("ros2_mpu6050"),
            "launch",
            "ros2_mpu6050.launch.py"
        ),
    )

    # ---------------- Madgwick Filter ----------------
    madgwick_filter = Node(
        package='imu_filter_madgwick',
        executable='imu_filter_madgwick_node',
        name='imu_filter',
        output='screen',
        parameters=[{
            'use_mag': False,         
            'publish_tf': False,     
            'world_frame': 'enu',
            'fixed_frame': 'odom', 
        }],
        remappings=[
            # Subscribe
            ('/imu/data_raw', '/imu/raw'),
            # Publish 
            ('/imu/data', '/imu/filtered')
        ]
    )


    
    return LaunchDescription([
        imu_driver,
        madgwick_filter
    ])