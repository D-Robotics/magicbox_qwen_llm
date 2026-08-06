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
from launch.substitutions import PathJoinSubstitution

def generate_launch_description():

    enable_function_call_arg = DeclareLaunchArgument(
        "enable_function_call", default_value="False"
    )
    wait_for_audio_call_arg = DeclareLaunchArgument(
        "wait_for_audio", default_value="True"
    )
    llm_model_path_arg = DeclareLaunchArgument(
        "llm_model_path", default_value="/userdata/magicbox/config/qwen2.5-1.5b-instruct-q5_k_m.gguf"
    )
    pkg_path = FindPackageShare("qwen_llm")

    config_file = PathJoinSubstitution([
        pkg_path,
        "config"
    ])
    print(config_file)
    qwen_llm_node = Node(
        package='qwen_llm',
        executable='qwen_llm',
        output='screen',
        parameters=[
            {"llm_model_path": LaunchConfiguration('llm_model_path')},
            {"cute_words": "你好，请问有什么能够帮助您的？"},
            {"system_prompt_file_": PathJoinSubstitution([config_file, "system_prompt.txt"])},
            {"system_prompt_function_call_file": PathJoinSubstitution([config_file, "system_prompt_function_call.txt"])},
            {"enable_function_call": LaunchConfiguration('enable_function_call')},
            {"wait_for_audio": LaunchConfiguration('wait_for_audio')},
        ],
        arguments=['--ros-args', '--log-level', 'warn']
    )

    fc_call_node = Node(
        package='gesture_interaction',
        executable='function_call_control',
        output='screen',
        arguments=['--ros-args', '--log-level', 'info'],
        condition=IfCondition(LaunchConfiguration('enable_function_call')),
    )

    return LaunchDescription([
        SetEnvironmentVariable(
            'RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp'
        ),
        enable_function_call_arg,
        wait_for_audio_call_arg,
        llm_model_path_arg,
        fc_call_node,
        qwen_llm_node
    ])