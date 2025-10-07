#include "tcpServerNode.hpp"
#include "clientThread.hpp"

using namespace std::chrono_literals;

const std::string TcpServerNode::rosIPParameter = "ROS_IP";
const std::string TcpServerNode::rosPortParameter = "ROS_TCP_PORT";

TcpServerNode::TcpServerNode(const std::string &name) : rclcpp::Node(name) {}

bool TcpServerNode::init(int connections, const std::string &tcp_ip, int tcp_port) {
	std::string param_ip = declare_parameter<std::string>(rosIPParameter, "0.0.0.0");
	int param_port = static_cast<int>(declare_parameter<int>(rosPortParameter, 10000));
    
    if (!tcp_ip.empty()) {
		RCUTILS_LOG_INFO("Using ROS_IP override from constructor: %s", tcp_ip.c_str());
        this->tcp_ip = tcp_ip;
    } else {
        this->tcp_ip = param_ip;
    }
    
    if (tcp_port != 0) {
		RCUTILS_LOG_INFO("Using ROS_TCP_PORT override from constructor: %d", tcp_port);
        this->tcp_port = tcp_port;
    } else {
        this->tcp_port = param_port;
    }
    
    this->connections = connections;

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        RCUTILS_LOG_ERROR("WSAStartup failed: %d", iResult);
        return false;
    }
#endif

	return true;
}

void TcpServerNode::shutdown() {
    for (std::shared_ptr<ClientThread> client : client_threads) {
        client->halt();
    }
    for (std::shared_ptr<ClientThread> client : client_threads) {
        client->wait();
    }
    client_threads.clear();
    if (server_thread.joinable()) {
        stop_server_thread = true;
        server_thread.join();
    }
    executor.reset();
#ifdef _WIN32
    WSACleanup();
#endif
}

void TcpServerNode::start() {
    stop_server_thread = false;
    server_thread = std::thread(&TcpServerNode::listen_loop, this);
}

void TcpServerNode::spin_executor() {
    /*
        Since rclcpp.spin() is a blocking call the server needed a way
        to spin all of the relevant nodes at the same time.

        MultiThreadedExecutor allows us to set the number of threads
        needed as well as the nodes that need to be spun.    */
    executor = rclcpp::executors::MultiThreadedExecutor::make_unique(rclcpp::ExecutorOptions(), 0);
    executor->add_node(shared_from_this());
    executor->spin();
}

void TcpServerNode::register_node(std::shared_ptr<RosNode> node) {
    if (node) {
        executor->add_node(std::reinterpret_pointer_cast<rclcpp::Node>(node));
    }
}

void TcpServerNode::unregister_node(std::shared_ptr<RosNode> old_node) {
    if (old_node) {
        executor->remove_node(std::reinterpret_pointer_cast<rclcpp::Node>(old_node));
    }
}

void TcpServerNode::log_debug(const char* msg, ...) {
    const std::lock_guard<std::mutex> log_lock(log_mutex);
    bool ok = false;
    std::va_list args;
    va_start(args, msg);
    ok = get_log_string(msg, args);
    va_end(args);
    if (ok) RCUTILS_LOG_DEBUG(log_buffer);
}

void TcpServerNode::log_info(const char *msg, ...) {
    const std::lock_guard<std::mutex> log_lock(log_mutex);
    bool ok = false;
    std::va_list args;
    va_start(args, msg);
    ok = get_log_string(msg, args);
    va_end(args);
    if (ok) RCUTILS_LOG_INFO(log_buffer);
}

void TcpServerNode::log_warning(const char* msg, ...) {
    const std::lock_guard<std::mutex> log_lock(log_mutex);
    bool ok = false;
    std::va_list args;
    va_start(args, msg);
    ok = get_log_string(msg, args);
    va_end(args);
    if (ok) RCUTILS_LOG_WARN(log_buffer);
}

void TcpServerNode::log_error(const char *msg, ...) {
    const std::lock_guard<std::mutex> log_lock(log_mutex);
    bool ok = false;
    std::va_list args;
    va_start(args, msg);
    ok = get_log_string(msg, args);
    va_end(args);
    if (ok) RCUTILS_LOG_ERROR(log_buffer);
}


void TcpServerNode::listen_loop() {
    RCUTILS_LOG_INFO("Starting server on %s:%d", tcp_ip.c_str(), tcp_port);

    tcp_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_server == INVALID_SOCKET) {
        RCUTILS_LOG_ERROR("Unable to create socket : %d", last_socket_error());
        return;
    }
    int reuse_addr = 1;
    if (SOCKET_ERROR == setsockopt(tcp_server, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse_addr), sizeof(reuse_addr))) {
        RCUTILS_LOG_ERROR("setsockopt failed : %d", last_socket_error());
        closesocket(tcp_server);
        tcp_server = INVALID_SOCKET;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (1 != inet_pton(AF_INET, tcp_ip.c_str(), &addr.sin_addr)) {
        RCUTILS_LOG_ERROR("inet_pton failed with ip '%s' : %d", tcp_ip.c_str(), last_socket_error());
        closesocket(tcp_server);
        tcp_server = INVALID_SOCKET;
        return;
    }
    addr.sin_port = htons(tcp_port);
    if (SOCKET_ERROR == bind(tcp_server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
        RCUTILS_LOG_ERROR("bind failed : %d", last_socket_error());
        closesocket(tcp_server);
        tcp_server = INVALID_SOCKET;
        return;
    }

    while (!stop_server_thread) {
        if (SOCKET_ERROR == listen(tcp_server, connections)) {
            RCUTILS_LOG_ERROR("listen failed : %d", last_socket_error());
            closesocket(tcp_server);
            tcp_server = INVALID_SOCKET;
            return;
        }

        sockaddr_in client_addr{};
        socket_len_t client_addr_len = static_cast<socket_len_t>(sizeof(client_addr));
        SOCKET client_socket = accept(tcp_server, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            RCUTILS_LOG_ERROR("unable to accept incoming connection request : %d", last_socket_error());
        } else {
            std::shared_ptr<ClientThread> clientThread = std::make_shared<ClientThread>(this, client_socket, client_addr);
            {
                const std::lock_guard<std::mutex> clients_lock(client_threads_mutex);
                // remove finished threads
                for (auto& iter = client_threads.begin(); iter != client_threads.end(); iter++) {
                    if ((*iter)->is_finished()) {
                        (*iter)->wait();
                        iter = client_threads.erase(iter);
                    }
                }
                client_threads.insert(clientThread);
            }
            clientThread->start();
        }
    }

    closesocket(tcp_server);
    tcp_server = INVALID_SOCKET;
}


bool TcpServerNode::get_log_string(const char* format, std::va_list args) {
    return vsnprintf(log_buffer, sizeof(log_buffer), format, args) > 0;
}
