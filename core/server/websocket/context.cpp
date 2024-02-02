#include "../server.hpp"
#include "./transport.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::WSServer::Transport;

WebsocketContext::WebsocketContext(ContextInit init) : conn(init.conn) {

	this->m_reader = std::async([&]() {

		std::vector<uint8_t> buffer = init.connbuff;

		while (this->conn.active() && !this->m_stopped) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

	});
}

WebsocketContext::~WebsocketContext() {
	this->m_stopped = true;
	if (this->m_reader.valid()) {
		try { this->m_reader.get(); }
			catch (...) {}
	}
}


void WebsocketContext::close(Websocket::CloseReason reason) {

	this->m_stopped = true;

}
