//
// Created by Aleksey Timin on 11/18/19.
//

#ifndef EIPSCANNER_SOCKETS_BASESOCKET_H
#define EIPSCANNER_SOCKETS_BASESOCKET_H

#include <vector>
#include <cstdint>
#include <string>
#include <chrono>
#include <functional>
#include <memory>

#include "EndPoint.h"

#ifdef _WIN32
	typedef SOCKET _SOCKET;
#else // Linux and MacOS
	typedef int _SOCKET;
#endif

namespace eipScanner {
namespace sockets {
	class BaseSocket {
	public:
		using BeginReceiveHandler = std::function<void(BaseSocket&)>;
		using SPtr = std::shared_ptr<BaseSocket>;
		using UPtr = std::unique_ptr<BaseSocket>;

		explicit BaseSocket(EndPoint endPoint);
		BaseSocket(std::string host, int port);
		virtual ~BaseSocket();

		virtual void Send(const std::vector<uint8_t>& data) const = 0;
		virtual std::vector<uint8_t> Receive(size_t size) const = 0;
		void setBeginReceiveHandler(BeginReceiveHandler handler);


		const std::chrono::milliseconds &getRecvTimeout() const;
		void setRecvTimeout(const std::chrono::milliseconds &recvTimeout);

		_SOCKET getSocketFd() const;

		const EndPoint &getRemoteEndPoint() const;

		static void select(std::vector<BaseSocket::SPtr> sockets, std::chrono::milliseconds timeout);

	protected:
		void BeginReceive();

		_SOCKET _sockedFd;
		EndPoint _remoteEndPoint;

		std::chrono::milliseconds _recvTimeout;
		BeginReceiveHandler _beginReceiveHandler;

		static timeval makePortableInterval(const std::chrono::milliseconds &recvTimeout);
	};
}
}


#endif  // EIPSCANNER_SOCKETS_BASESOCKET_H
