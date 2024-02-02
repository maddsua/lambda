#include "../websocket.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::WSServer;

WebsocketContext::WebsocketContext(HTTPServer::ConnectionContext* connctxInit) {
	if (!connctxInit) throw std::runtime_error("ConnectionContext* cannot be a nullptr");
	this->connctx = connctxInit;
}

WebsocketContext::~WebsocketContext() {
	if (this->m_reader.valid()) {
		try { this->m_reader.get(); }
			catch (...) {}
	}
}
/*
void WebsocketContext::close(Websocket::CloseReason reason) {

}
*/
