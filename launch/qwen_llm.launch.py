# Copyright (c) 2024，D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from launch import LaunchDescription
from launch_ros.actions import Node

from launch.actions import IncludeLaunchDescription, SetEnvironmentVariable, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python import get_package_share_directory
from launch.substitutions import TextSubstitution, LaunchConfiguration
from launch.conditions import IfCondition
from launch_ros.substitutions import FindPackageShare
from launch.substitutions import PathJoinSubstitution, PythonExpression

def generate_launch_description():
    pkg_path = FindPackageShare("qwen_llm")

    config_file = PathJoinSubstitution([
        pkg_path,
        "config"
    ])

    fc_call_node = Node(
        package='gesture_legs_control',
        executable='function_call_control',
        output='screen',
        arguments=['--ros-args', '--log-level', 'info'],
        condition=IfCondition(LaunchConfiguration('enable_function_call')),
    )

    cute_words = PythonExpression([
        "'",
        LaunchConfiguration('cute_words'),
        "' if '",
        LaunchConfiguration('cute_words'),
        "' != '' else (",
        "'你好，请问有什么可以帮助你的吗' if '",
        LaunchConfiguration('language_type'),
        "' == 'zh' else 'Hello, may I help you with anything'"
        ")"
    ])

    system_prompt_file = PythonExpression([
        "('",  # 开始引号
        LaunchConfiguration('system_prompt_file'),
        "' if '",
        LaunchConfiguration('system_prompt_file'),
        "' != '' else '",
        PathJoinSubstitution([config_file, "system_prompt.txt"]),
        "' if '",
        LaunchConfiguration('language_type'),
        "' == 'zh' else '",
        PathJoinSubstitution([config_file, "system_prompt_en.txt"]),
        "')"
    ])

    return LaunchDescription([
        SetEnvironmentVariable(
            'RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp'
        ),
        DeclareLaunchArgument(
            'cute_words',
            default_value="",
            description='cute words'),
        DeclareLaunchArgument(
            'enable_function_call',
            default_value='False',
            description='enable function call'),
        DeclareLaunchArgument(
            'wait_for_audio',
            default_value='True',
            description='wait for audio'),
        DeclareLaunchArgument(
            'language_type',
            default_value='zh',
            description='language type'),
        DeclareLaunchArgument(
            'system_prompt_file',
            default_value='',
            description='prompt file'),
        # 启动手势操控节点    
        Node(
            package='gesture_interaction',
            executable='function_call_control',
            output='screen',
            arguments=['--ros-args', '--log-level', 'info'],
            condition=IfCondition(LaunchConfiguration('enable_function_call')),
        ),
        # 启动音频采集pkg
        Node(
            package='qwen_llm',
            executable='qwen_llm',
            output='screen',
            parameters=[
                {"llm_model_path": "/dev/shm/qwen2.5-1.5b-instruct-q5_k_m.gguf"},
                {"cute_words": cute_words},
                {"system_prompt_file_": system_prompt_file},
                {"system_prompt_function_call_file": PathJoinSubstitution([config_file, "system_prompt_function_call.txt"])},
                {"enable_function_call": LaunchConfiguration('enable_function_call')},
                {"wait_for_audio": LaunchConfiguration('wait_for_audio')},
                {"language_type": LaunchConfiguration('language_type')},
            ],
            arguments=['--ros-args', '--log-level', 'warn']
        )
    ])