#include "http.hpp"

#ifndef _maddsua_http_lambda
#define _maddsua_http_lambda

namespace maddsua {

	struct lambdaEvent {
		std::string method;
		std::string httpversion;
		std::string path;
		std::vector <datapair> searchQuery;
		std::vector <datapair> headers;
		std::string body;
	};
	struct lambdaResponse {
		uint16_t statusCode;
		std::vector <datapair> headers;
		std::string body;
	};
	struct lambdalog {
		std::string type;
		std::string time;
		std::string text;
	};


	class lambda {

		public:
			lambda() {
				wsaData = {0};
				ListenSocket = INVALID_SOCKET;
				handlerDispatched = true;
				running = false;
			}
			lambda(const lambda & obj) {
				//	a copy constructor so IntelliSense would shut the f up
			}
			lambda(lambda && obj) {
				//	a move constructor for the same purpose
				//	you can't actually use them due to std::threads not being copyable
				//	and copying ov moving the server does not make any sence to me, bc it's a "running function" and not a data object,
				//	so I can't think of scenario wnen you need to copy it
			}
			~lambda() {
				close();
			}

			actionResult init(const char* port, std::function <lambdaResponse(lambdaEvent)> lambda);
			void close();

			inline bool logsAvail() { return serverlog.size(); }
			inline std::vector <lambdalog> logs() {
				auto temp = serverlog;
				serverlog.erase(serverlog.begin(), serverlog.end());
				return temp;
			}

		private:
			WSADATA wsaData;
			SOCKET ListenSocket;

			bool running;
			std::thread worker;
			void connectManager();

			std::function<lambdaResponse(lambdaEvent)> callback;
			bool handlerDispatched;
			void handler();
			std::vector <lambdalog> serverlog;
	};

}

#endif