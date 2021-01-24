//
// Created by Aleksey Timin on 11/16/19.
//

#include <system_error>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else // Linux and MacOS
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

#include <fcntl.h>

#include "utils/Logger.h"
#include "TCPSocket.h"


namespace eipScanner {
	namespace sockets {
		using eipScanner::utils::Logger;
		using eipScanner::utils::LogLevel;

		TCPSocket::TCPSocket(std::string host, int port)
				: TCPSocket(EndPoint(host, port)) {
		}

		TCPSocket::TCPSocket(EndPoint endPoint, std::chrono::milliseconds connTimeout)
				: BaseSocket(std::move(endPoint)) {
			_sockedFd = socket(AF_INET, SOCK_STREAM, 0);
			if (_sockedFd < 0) {
				throw std::system_error(errno, std::generic_category());
			}
			// Set non-blocking
#ifdef  _WIN32
			u_long mode = 1;  // 1 to enable non-blocking socket
			auto arg = ioctlsocket(_sockedFd, FIONBIO, &mode);
			if (arg < 0) {
				throw std::system_error(errno, std::generic_category());
			}
#else // Linux and MacOS
			auto arg = fcntl(_sockedFd, F_GETFL, NULL);
			if (arg < 0) {
				throw std::system_error(errno, std::generic_category());
			}

			arg |= O_NONBLOCK;
			if (fcntl(_sockedFd, F_SETFL, arg) < 0) {
				throw std::system_error(errno, std::generic_category());
			}
#endif

			Logger(LogLevel::DEBUG) << "Opened socket fd=" << _sockedFd;

			Logger(LogLevel::DEBUG) << "Connecting to " << _remoteEndPoint.toString();
			auto addr = _remoteEndPoint.getAddr();
			auto res = connect(_sockedFd, (struct sockaddr *) &addr, sizeof(addr));
			if (res < 0) {
				if (errno == EINPROGRESS) {
					do {
						fd_set myset;
						auto tv = makePortableInterval(connTimeout);

						FD_ZERO(&myset);
						FD_SET(_sockedFd, &myset);
						res = ::select(((int) _sockedFd) + 1, NULL, &myset, NULL, &tv);

						if (res < 0 && errno != EINTR) {
							throw std::system_error(errno, std::generic_category());
						} else if (res > 0) {
							// Socket selected for write
							int err;
							socklen_t lon = sizeof(int);
#ifdef _WIN32
							if (getsockopt(_sockedFd, SOL_SOCKET, SO_ERROR, (char*)(&err), &lon) < 0) {
#else // Linux and MacOS
							if (getsockopt(_sockedFd, SOL_SOCKET, SO_ERROR, (void*)(&err), &lon) < 0) {
#endif
								throw std::system_error(errno, std::generic_category());
							}
							// Check the value returned...
							if (err) {
								throw std::system_error(err, std::generic_category());
							}
							break;
						} else {
							throw std::system_error(ETIMEDOUT, std::generic_category());
						}
					} while (1);
				} else {
					throw std::system_error(errno, std::generic_category());
				}
			}
			// Set to blocking mode again...
#ifdef  _WIN32
			mode = 0;  // 0 to disable non-blocking socket
			arg = ioctlsocket(_sockedFd, FIONBIO, &mode);
			if (arg < 0) {
				throw std::system_error(errno, std::generic_category());
			}
#else // Linux and MacOS
			if ((arg = fcntl(_sockedFd, F_GETFL, NULL)) < 0) {
				throw std::system_error(errno, std::generic_category());
			}
			arg &= (~O_NONBLOCK);
			if (fcntl(_sockedFd, F_SETFL, arg) < 0) {
				throw std::system_error(errno, std::generic_category());
			}
#endif
		}


		TCPSocket::TCPSocket(EndPoint endPoint)
				: TCPSocket(std::move(endPoint), std::chrono::seconds(1)) {

		}

		TCPSocket::~TCPSocket() {
			Logger(LogLevel::DEBUG) << "Close socket fd=" << _sockedFd;
#ifdef  _WIN32
			shutdown(_sockedFd, SD_BOTH);
			closesocket(_sockedFd);
#else // Linux and MacOS
			shutdown(_sockedFd, SHUT_RDWR);
			close(_sockedFd);
#endif
		}

		void TCPSocket::Send(const std::vector<uint8_t> &data) const {
			Logger(LogLevel::TRACE) << "Send " << data.size() << " bytes from TCP socket #" << _sockedFd << ".";

			int count = send(_sockedFd, reinterpret_cast<const char*>(data.data()), data.size(), 0);
			if (count < data.size()) {
				throw std::system_error(errno, std::generic_category());
			}
		}

		std::vector<uint8_t> TCPSocket::Receive(size_t size) const {
			std::vector<uint8_t> recvBuffer(size);

			int count = 0;
			while (size > count) {
				auto len = recv(_sockedFd, reinterpret_cast<char*>(recvBuffer.data()) + count, size - count, 0);
				count += len;
				if (len < 0) {
					throw std::system_error(errno, std::generic_category());
				}

				if (len == 0) {
					break;
				}
			}

			if (size != count) {
				Logger(LogLevel::WARNING) << "Received from " << _remoteEndPoint.toString()
										  << " " << count << " of " << size;
			}

			return recvBuffer;
		}
	}
}