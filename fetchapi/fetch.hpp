#ifndef __LAMBDA_FETCH__
#define __LAMBDA_FETCH__


#include "../lambda.hpp"
#include "../http/http.hpp"

namespace Lambda::Fetch {

	enum FetchConstants {
		FETCH_MAX_ATTEMPTS = 5
	};

	struct RequestOptions {
		std::string method = "GET";
		std::vector<HTTP::KVtype> headers;
		std::vector<uint8_t> body;
	};

	HTTP::Response fetch(std::string url);
	HTTP::Response fetch(std::string url, const RequestOptions& data);

};

#endif
