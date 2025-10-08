# ROS TCP Endpoint

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Introduction

[ROS](https://www.ros.org/) package used to create an endpoint to accept ROS messages sent from a Unity scene using the [ROS TCP Connector](https://github.com/Unity-Technologies/ROS-TCP-Connector) scripts.

Instructions and examples on how to use this ROS package can be found on the [Unity Robotics Hub](https://github.com/Unity-Technologies/Unity-Robotics-Hub/blob/master/tutorials/ros_unity_integration/README.md) repository.

This is a C++ version of the ROS2 ROS-TCP-Endpoint node which was originally written in Python.

## Requirements

It requires ROS2 Jazzy (or maybe later) to build since we're using the GenericClient class which was added in Jazzy.

If you need to build the node for an older ROS2 version, you have to write your own GenericClient class which could probably be inspired of what is done in GenericService and in Jazzy's GenericClient class (see rclcpp/generic_client.hpp).

## Building

clone this reposository in the src folder of your ROS2 colcon workspace. Then at the root of your workspace, run the command :
```colcon build --merge-install```

To create a TCP-Endpoint node, remember to source your built node and you can use the sample ```endpoint.py``` launch file in ```launch``` folder

## Community and Feedback

The Unity Robotics projects are open-source and we encourage and welcome contributions.
If you wish to contribute, be sure to review our [contribution guidelines](CONTRIBUTING.md)
and [code of conduct](CODE_OF_CONDUCT.md).

## Support

For questions or discussions about Unity Robotics package installations or how to best set up and integrate your robotics projects, please create a new thread on the [Unity Robotics forum](https://forum.unity.com/forums/robotics.623/) and make sure to include as much detail as possible.

For feature requests, bugs, or other issues, please file a [GitHub issue](https://github.com/Unity-Technologies/ROS-TCP-Endpoint/issues) using the provided templates and the Robotics team will investigate as soon as possible.

For any other questions or feedback, connect directly with the
Robotics team at [unity-robotics@unity3d.com](mailto:unity-robotics@unity3d.com).

## License

[Apache License 2.0](LICENSE)