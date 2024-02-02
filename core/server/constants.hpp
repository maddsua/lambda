#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_CONSTANTS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_CONSTANTS__

#include <string>

namespace Lambda::Server {
	static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	static const std::string wsPingString = "ping/lambda/ws";
};

#endif
