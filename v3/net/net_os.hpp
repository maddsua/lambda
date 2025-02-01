#ifndef __LIB_MADDSUA_LAMBDA_NET_UTILS_PRIVATE__
#define __LIB_MADDSUA_LAMBDA_NET_UTILS_PRIVATE__

	#include <stdexcept>

	#ifdef _WIN32

		#define WIN32_LEAN_AND_MEAN

		#include <winsock2.h>
		#include <ws2tcpip.h>
		#include <stdexcept>

		#define LAMBDA_INVALID_SOCKET (INVALID_SOCKET)
		#define LAMBDA_SD_BOTH (SD_BOTH)

		#define lambda_os_errno() (static_cast<int>(GetLastError()))
		#define lambda_close_sock(sock_handle) (closesocket(sock_handle))

		inline bool lambda_wsa_wake_up() {

			static bool initDone = false;

			if (initDone) {
				return false;
			}

			initDone = true;

			WSADATA initdata;
			if (WSAStartup(MAKEWORD(2,2), &initdata) != 0) {
				throw std::runtime_error("WSA initialization failed with code " + std::to_string(lambda_os_errno()));
			}

			return true;
		}

		inline int lambda_shutdown_sock(SockHandle sock_handle, int how) noexcept {
			return shutdown(sock_handle, how);
		}

		inline bool lambda_is_error_disconnect(int code) noexcept {
			return code == WSAEINTR || code == WSAECONNRESET;
		}

		inline bool lambda_is_error_timeout(int code) noexcept {
			return code == WSAETIMEDOUT;
		}

	#else

		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netdb.h>
		#include <unistd.h>
		#include <arpa/inet.h>
		#include <cerrno>

		#define LAMBDA_INVALID_SOCKET (-1)
		#define LAMBDA_SD_BOTH (SHUT_RDWR)

		#define lambda_os_errno() (static_cast<int>(errno))

		inline int lambda_close_sock(int fd) noexcept {
			return close(fd);
		}

		inline int lambda_shutdown_sock(int fd, int how) noexcept {
			return shutdown(fd, how);
		}

		inline bool lambda_is_error_disconnect(int code) noexcept {
			return code == ECONNRESET;
		}

		inline bool lambda_is_error_timeout(int code) noexcept {
			return code == ETIMEDOUT || code == EWOULDBLOCK;
		}

	#endif

#endif
