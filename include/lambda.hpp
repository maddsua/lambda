#ifndef _maddsua_http_lambda
#define _maddsua_http_lambda

#include <stdint.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <time.h>

#include <string>
#include <vector>
#include <thread>
#include <functional>

#define HTTPLAMBDA_CHUNK_SIZE	(8192)

namespace maddsuahttp {

	struct datapair {
		std::string name;
		std::string value;
	};

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

	struct actionResult {
		bool success;
		std::string cause;
		std::string info;
	};

	struct httpRequest {
		std::vector <std::string> startLineArgs;
		std::vector <datapair> headers;
		std::string body;
	};

	struct logentry {
		std::string type;
		std::string time;
		std::string text;
	};

	void toLowerCase(std::string* text);
	std::string toLowerCase(std::string text);
	void toUpperCase(std::string* text);
	std::string toUpperCase(std::string text);
	void toTitleCase(std::string* text);
	std::string toTitleCase(std::string text);

	void trim(std::string* text);
	std::string trim(std::string text);
	std::vector <std::string> splitBy(const char* source, const char* token);

	std::string findHeader(const char* headerName, std::vector <datapair>* headers);
	inline std::string findSearchQuery(const char* headerName, std::vector <datapair>* headers) {
		return findHeader(headerName, headers);
	}
	std::string findMimeType(const char* extension);
	std::string formattedTime(time_t epoch_time);

	std::vector <datapair> searchQueryParams(std::string* url);

	std::string _findHttpCode(const uint16_t statusCode);
	httpRequest _getData(SOCKET* client);
	actionResult _sendData(SOCKET* client, std::string startline, std::vector <datapair>* headers, std::string* body);

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
			inline std::vector <logentry> logs() {
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
			std::vector <logentry> serverlog;
	};

}

#endif