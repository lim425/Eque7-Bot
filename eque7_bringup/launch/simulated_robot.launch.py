import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition, UnlessCondition
from launch_ros.actions import Node


def generate_launch_description():

    # ---------------- Gazebo Simulation ----------------
    gazebo = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_description"),
            "launch",
            "gazebo.launch.py"
        ),
    )
    
    # ---------------- Controller ----------------
    controller = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_controller"),
            "launch",
            "controller.launch.py"
        ),
    )
    
    # ---------------- Joystick ----------------
    joystick = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("eque7_controller"),
            "launch",
            "joystick.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "True"
        }.items()
    )

    # ---------------- EKF ----------------
    # ekf = IncludeLaunchDescription(
    #     os.path.join(
    #         get_package_share_directory("eque7_localization"),
    #         "launch",
    #         "local_localization.launch.py"
    #     ),
    # )
    
    return LaunchDescription([
        gazebo,
        controller,
        joystick,
        # ekf
    ])