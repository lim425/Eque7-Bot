from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    # Use simulation time (Gazebo)
    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="true",
    )

    use_sim_time = LaunchConfiguration("use_sim_time")

    # Joint state broadcaster (REQUIRED)
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager",
        ],
        parameters=[{"use_sim_time": use_sim_time}],
    )

    # Main diff drive controller
    diff_drive_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "eque7_controller",
            "--controller-manager",
            "/controller_manager",
        ],
        parameters=[{"use_sim_time": use_sim_time}],
    )

    return LaunchDescription([
        use_sim_time_arg,
        joint_state_broadcaster_spawner,
        diff_drive_controller_spawner,
    ])
