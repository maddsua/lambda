#include "./websocket.hpp"
#include "../../websocket/proto.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::WSServer;

void WebsocketContext::sendMessage(const Websocket::Message& msg) {
	auto writeBuff = serializeMessage(msg);
	this->connctx->conn.write(writeBuff);
}
