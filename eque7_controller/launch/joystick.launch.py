from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription

import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="True",
    )

    use_sim_time = LaunchConfiguration('use_sim_time')

    joy_params = os.path.join(get_package_share_directory('eque7_controller'),'config','joystick.yaml')

    joy_node = Node(
            package='joy',
            executable='joy_node',
            parameters=[joy_params, {'use_sim_time': use_sim_time}],
         )

    teleop_node = Node(
            package='teleop_twist_joy',
            executable='teleop_node',
            name='teleop_node',
            parameters=[joy_params, {'use_sim_time': use_sim_time}],
            remappings=[('/cmd_vel','/joy_vel')]
         )
    
    robot_controller_pkg = get_package_share_directory('eque7_controller')

    twist_mux_launch = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("twist_mux"),
            "launch",
            "twist_mux_launch.py"
        ),
        launch_arguments={
            "cmd_vel_out": "eque7_controller/cmd_vel_unstamped",
            "config_locks": os.path.join(robot_controller_pkg, "config", "twist_mux_locks.yaml"),
            "config_topics": os.path.join(robot_controller_pkg, "config", "twist_mux_topics.yaml"),
            "use_sim_time": LaunchConfiguration("use_sim_time"),
        }.items(),
    )
    
    # twist_stamper = Node(
    #         package='eque7_controller',
    #         executable='twist_to_twiststamped.py',
    #         name="twist_to_twiststamped"
    #      )

    return LaunchDescription([
        use_sim_time_arg,
        joy_node,
        teleop_node,
        twist_mux_launch,
        # twist_stamper       
    ])