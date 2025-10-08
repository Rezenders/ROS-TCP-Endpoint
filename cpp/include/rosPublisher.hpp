
#pragma once
#include "rosCommunication.hpp"
#include <memory>
#include <mutex>
#include <rclcpp/serialized_message.hpp>

class RosPublisher : public RosSender {
public:
	RosPublisher(const std::string& topic, const std::string &message_type, int queue_size = 10);

	std::string get_message_type() const override;
	void send(const RosData &message);

private:
	std::string message_type;
	int queue_size;
	rclcpp::GenericPublisher::SharedPtr publisher;
	rclcpp::SerializedMessage serialized_message_buffer;
	std::mutex serialized_message_mutex;
};
