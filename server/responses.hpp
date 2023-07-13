#ifndef __LAMBDA_SERVER_RESPONSES__
#define __LAMBDA_SERVER_RESPONSES__

#include "../http/http.hpp"

namespace Lambda::Responses {

	HTTP::Response serviceResponse(uint16_t httpStatus, const std::string& text);

};

#endif