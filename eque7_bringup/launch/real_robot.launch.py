import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition, UnlessCondition
from launch_ros.actions import Node


def generate_launch_description():

    # ---------------- Hardware Interface ----------------
    hardware_interface = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_firmware"),
            "launch",
            "hardware_interface.launch.py"
        ),
    )

    # ---------------- Controller ----------------
    controller = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_controller"),
            "launch",
            "controller.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "False"
        }.items(),
    )
    
    # ---------------- Joystick ----------------
    joystick = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_controller"),
            "launch",
            "joystick.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "False"
        }.items()
    )

    # ---------------- Imu ----------------

    imu = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_firmware"),
            "launch",
            "imu.launch.py"
        ),
    )

    # ---------------- EKF ----------------
    ekf = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_localization"),
            "launch",
            "local_localization.launch.py"
        ),
    )

    return LaunchDescription([
        hardware_interface,
        controller,
        joystick,
        imu,
        ekf,
    ])