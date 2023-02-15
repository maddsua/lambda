/*

	maddsua's
     ___       ________  _____ ______   ________  ________  ________
    |\  \     |\   __  \|\   _ \  _   \|\   __  \|\   ___ \|\   __  \
    \ \  \    \ \  \|\  \ \  \\\__\ \  \ \  \|\ /\ \  \_|\ \ \  \|\  \
     \ \  \    \ \   __  \ \  \\|__| \  \ \   __  \ \  \ \\ \ \   __  \
      \ \  \____\ \  \ \  \ \  \    \ \  \ \  \|\  \ \  \_\\ \ \  \ \  \
       \ \_______\ \__\ \__\ \__\    \ \__\ \_______\ \_______\ \__\ \__\
        \|_______|\|__|\|__|\|__|     \|__|\|_______|\|_______|\|__|\|__|

	A C++ HTTP server framework

	2023 https://github.com/maddsua/lambda
	
*/


#ifndef H_MADDSUA_LAMBDA_HTTP
#define H_MADDSUA_LAMBDA_HTTP

#include <stdint.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#include <string>
#include <vector>
#include <thread>
#include <functional>

#include "compression.hpp"

#define HTTPLAMBDA_HEADER_CHUNK		(2048)
#define HTTPLAMBDA_BODY_CHUNK		(131072)
#define MADDSUAHTTP_ATTEMPTS		(3)
#define HTTPLAMBDA_USERAGENT		"maddsua/lambda"


namespace lambda {

	/**
	 * @param name
	 * @param value
	*/
	struct datapair {
		std::string name;
		std::string value;
	};

	/**
	 * @param errors
	 * @param statusCode
	 * @param statusText
	 * @param headers
	 * @param body
	*/
	struct fetchResult {
		std::string errors;
		unsigned int statusCode;
		std::string statusText;
		std::vector <datapair> headers;
		std::string body;
	};

	/**
	 * @param success
	 * @param cause
	 * @param info
	*/
	struct actionResult {
		bool success;
		std::string cause;
		std::string info;
	};

	/**
	 * @param success
	 * @param startLineArgs
	 * @param headers
	 * @param body
	*/
	struct httpRequest {
		bool success;
		std::vector <std::string> startLineArgs;
		std::vector <datapair> headers;
		std::string body;
	};

	/**
	 * Check if WSA (Windows Sockets API) is ready to use
	*/
	bool socketsReady();

	/**
	 * Transform string to lower case (all lowercase, x-netlify)
	*/
	void toLowerCase(std::string* text);
	std::string toLowerCase(std::string text);

	/**
	 * Transform string to upper case (all capitals, X-NETLIFY)
	*/
	void toUpperCase(std::string* text);
	std::string toUpperCase(std::string text);

	/**
	 * Reset string case like this: x-netlify -> X-Netlify
	*/
	void toTitleCase(std::string* text);
	std::string toTitleCase(std::string text);

	/**
	 * Removes whitespace and newline characters frome the beginning and the end of a string
	*/
	void trim(std::string* text);
	std::string trim(std::string text);

	/**
	 * Split string into vector of strings by the token (character or substring)
	*/
	std::vector <std::string> splitBy(std::string source, std::string token);

	/**
	 * Returns true if a string contains the substring. Just like in javascript.
	 * And yes, it could be done as a class method. I don't feel like creating a class on top of another class
	 * Compare datastring.includes("substring") to the includes(&datastring, "substring").
	 * It's the same length
	*/
	inline bool includes(std::string* findIn, std::string substring) {
		return findIn->find(substring) != std::string::npos;
	}
	inline bool includes(std::string findIn, std::string substring) {
		return includes(&findIn, substring);
	}
	inline bool includes(std::string* findIn, std::vector <std::string> substrings) {
		for (auto substring : substrings) {
			if (findIn->find(substring) != std::string::npos) return true;
		}
		return false;
	}
	inline bool includes(std::string findIn, std::vector <std::string> substrings) {
		return includes(&findIn, substrings);
	}

	/**
	 * True, if a string starts with a substring
	*/
	inline bool startsWith(std::string* text, std::string substring) {
		return text->find(substring) == 0;
	}
	inline bool startsWith(std::string text, std::string substring) {
		return startsWith(&text, substring);
	}

	/**
	 * True, if a string ends with a substring
	*/
	inline bool endsWith(std::string* text, std::string substring) {
		return text->find(substring) == text->size() - substring.size();
	}
	inline bool endsWith(std::string text, std::string substring) {
		return endsWith(&text, substring);
	}


	/**
	 * Looks for specific header in a vector of headers and returns it's value, or an empty string, if it was not found
	*/
	std::string findHeader(std::string headerName, std::vector <datapair>* headers);

	/**
	 * Returns true if a key exists in the provided vector of key-value pairs (headers or search query params)
	*/
	inline bool keyExists(std::string headerName, std::vector <datapair>* headers) {
		return findHeader(headerName, headers).size();
	}


	/**
	 * Add a header, if there is no other with such a name
	 * Returns false if such a header already exists
	*/
	inline bool addHeader(datapair header, std::vector <datapair>* headers) {
		if (keyExists(header.name, headers)) return false;
		headers->push_back({header.name, header.value});
		return true;
	}

	/**
	 * Add a header, replacing if exists
	*/
	void insertHeader(std::string header, std::string value, std::vector <datapair>* headers);

	/**
	 * Returns mimetype for specified file extension.
	 * Defaults to application/octet-stream
	*/
	std::string mimetype(std::string extension);

	/**
	 * Get time in the format: Wed, 06 Jan 2023 07:28:00 GMT
	*/
	std::string httpTime(time_t epoch_time);

	/**
	 * Get the current time in the format: Wed, 06 Jan 2023 07:28:00 GMT
	*/
	inline std::string httpTimeNow() {
		return httpTime(time(nullptr));
	}

	/**
	 * Extracts search query params as name-value pairs
	*/
	std::vector <datapair> getSearchQuery(std::string* url);

	/**
	 * Looks for specific search query and returns it's value, or an empty string, if it was not found
	*/
	inline std::string findQuery(std::string queryName, std::vector <datapair>* queries) {
		return findHeader(queryName, queries);
	}

	/**
	 * Finds http status text for the provided status code.
	 * (403) => "403 Forbidden"
	 * If not found, defaults to "200 OK"
	*/
	std::string httpStatusString(const unsigned int statusCode);

	/**
	 * Receive http data from a socket in one go
	*/
	httpRequest socketGetHTTP(SOCKET* client);

	/**
	 * Send http data to a socket in one go
	*/
	actionResult socketSendHTTP(SOCKET* client, std::string startline, std::vector <datapair>* headers, std::string* body);

	/**
	 * The same as javascript's fetch(). Sends request to a URL and returns server response
	*/
	fetchResult fetch(std::string url, std::string method, std::vector <datapair> headers, std::string body);

}

#endif