//
// Created by Aleksey Timin on 11/21/19.
//
#include <system_error>

#ifdef _WIN32
	#include <winsock2.h>
#else // Linux and MacOS
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

#include "UDPBoundSocket.h"

namespace eipScanner {
namespace sockets {


	UDPBoundSocket::UDPBoundSocket(std::string host, int port)
		: UDPBoundSocket(EndPoint(host, port)) {

	}
	UDPBoundSocket::UDPBoundSocket(EndPoint endPoint)
		: UDPSocket(std::move(endPoint)) {
		int on = 1;
		int err = setsockopt(_sockedFd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

#ifdef _WIN32
		if (err == SOCKET_ERROR) {
			throw std::system_error(WSAGetLastError(), std::generic_category());
		}
#else // Linux and MacOS
		if (err < 0) {
			throw std::system_error(errno, std::generic_category());
		}
#endif

		auto addr = _remoteEndPoint.getAddr();
		addr.sin_addr.s_addr = INADDR_ANY;
		err = bind(_sockedFd, (struct sockaddr*)&addr, sizeof(addr));

#ifdef _WIN32
		if (err == SOCKET_ERROR) {
			throw std::system_error(WSAGetLastError(), std::generic_category());
		}
#else // Linux and MacOS
		if (err < 0) {
			throw std::system_error(errno, std::generic_category());
		}
#endif

	}

	sockets::UDPBoundSocket::~UDPBoundSocket() = default;
}
}