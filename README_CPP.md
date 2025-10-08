# ROS TCP Endpoint

This is a C++ version of the ROS2 ROS-TCP-Endpoint node which was originally written in Python.

## Requirements

It requires ROS2 Jazzy (or maybe later) to build since we're using the GenericClient class which was added in Jazzy.

If you need to build the node for an older ROS2 version, you have to write your own GenericClient class which could probably be inspired of what is done in GenericService and in Jazzy's GenericClient class (see rclcpp/generic_client.hpp).

## Building

clone this reposository in the src folder of your ROS2 colcon workspace. Then at the root of your workspace, run the command :
```colcon build --merge-install```

To create a TCP-Endpoint node, remember to source your built node and you can use the sample ```endpoint.py``` launch file in ```launch``` folder