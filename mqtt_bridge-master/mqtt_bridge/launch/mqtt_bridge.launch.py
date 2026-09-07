from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_dir = get_package_share_directory('mqtt_bridge')
    default_config = os.path.join(pkg_dir, 'config', 'mqtt_config.yaml')
    default_vehicle_config = os.path.join(
        pkg_dir, 'config', 'vehicle_identity.yaml')

    config_file_arg = DeclareLaunchArgument(
        'config_file',
        default_value=default_config,
        description='MQTT配置文件路径'
    )
    vehicle_config_file_arg = DeclareLaunchArgument(
        'vehicle_config_file',
        default_value=default_vehicle_config,
        description='共享车辆身份配置文件路径'
    )

    mqtt_bridge_node = Node(
        package='mqtt_bridge',
        executable='mqtt_bridge_node',
        name='mqtt_bridge',
        output='screen',
        parameters=[
            LaunchConfiguration('vehicle_config_file'),
            {'config_file': LaunchConfiguration('config_file')}
        ],
        emulate_tty=True
    )

    return LaunchDescription([
        config_file_arg,
        vehicle_config_file_arg,
        mqtt_bridge_node
    ])
