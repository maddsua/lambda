#ifndef _maddsua_http
#define _maddsua_http

#include <stdint.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <time.h>

#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <regex>

#define HTTPLAMBDA_CHUNK_SIZE	(8192)
#define MADDSUAHTTP_ATTEMPTS	(3)

namespace maddsuaHTTP {

	struct datapair {
		std::string name;
		std::string value;
	};
	struct fetchResult {
		std::string errors;
		uint16_t statusCode;
		std::string statusText;
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

	bool socketsReady();

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
	inline bool headerExists(const char* headerName, std::vector <datapair>* headers) {
		return findHeader(headerName, headers).size();
	}

	std::string findMimeType(const char* extension);
	std::string formattedTime(time_t epoch_time);

	std::vector <datapair> searchQueryParams(std::string* url);
	inline std::string findSearchQuery(const char* headerName, std::vector <datapair>* headers) {
		return findHeader(headerName, headers);
	}

	std::string _findHttpCode(const uint16_t statusCode);
	httpRequest _getData(SOCKET* client);
	actionResult _sendData(SOCKET* client, std::string startline, std::vector <datapair>* headers, std::string* body);

	fetchResult fetch(std::string url, std::string method, std::vector <datapair> headers, std::string body);

}

#endif