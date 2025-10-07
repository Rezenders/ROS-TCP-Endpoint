#pragma once

#ifdef _WIN32
#  include <WinSock2.h>
#  include <WS2tcpip.h>
using socket_len_t = int;
inline int last_socket_error() {
    return WSAGetLastError();
}
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <cerrno>

using SOCKET = int;
constexpr SOCKET INVALID_SOCKET = -1;
constexpr int SOCKET_ERROR = -1;
using socket_len_t = socklen_t;

inline int closesocket(SOCKET socket) {
    return ::close(socket);
}
inline int last_socket_error() {
    return errno;
}
#ifndef WSAECONNABORTED
#define WSAECONNABORTED ECONNABORTED
#endif
#ifndef WSAECONNRESET
#define WSAECONNRESET ECONNRESET
#endif
#endif
