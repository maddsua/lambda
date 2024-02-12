#include "./sse.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda;
using namespace Lambda::Network;
using namespace Lambda::SSE;

Writer::Writer(HTTP::Transport::TransportContextV1& httpCtx)
	: m_conn(httpCtx.tcpconn()) {

	httpCtx.flags.autocompress = false;
	httpCtx.flags.forceContentLength = false;

	const auto hasOrigin = true;	//	to be fixed with transport update

	auto upgradeResponse = HTTP::Response(200, {
		{ "connection", "keep-alive" },
		{ "cache-control", "no-cache" },
		{ "content-type", "text/event-stream; charset=UTF-8" },
		{ "pragma", "no-cache" },
	});

	if (hasOrigin) {
		upgradeResponse.headers.set("Access-Control-Allow-Origin", "*");
	}

	httpCtx.respond(upgradeResponse);
}

void Writer::push(const EventMessage& event) {

	if (!this->m_conn.active()) {
		throw Lambda::Error("SSE listener disconnected");
	}

	std::vector<std::pair<std::string, std::string>> messageFields;

	if (event.event.has_value()) {
		messageFields.push_back({ "event", event.event.value() });
	}

	if (event.id.has_value()) {
		messageFields.push_back({ "id", event.id.value() });
	}

	messageFields.push_back({ "data", event.data });

	if (event.retry.has_value()) {
		messageFields.push_back({ "retry", std::to_string(event.retry.value()) });
	}

	static const std::string lineSeparator = "\r\n";
	static const std::string fieldSeparator = ": ";

	std::vector<uint8_t> serializedMessage;

	for (const auto& item : messageFields) {
		serializedMessage.insert(serializedMessage.end(), item.first.begin(), item.first.end());
		serializedMessage.insert(serializedMessage.end(), fieldSeparator.begin(), fieldSeparator.end());
		serializedMessage.insert(serializedMessage.end(), item.second.begin(), item.second.end());
		serializedMessage.insert(serializedMessage.end(), lineSeparator.begin(), lineSeparator.end());
	}

	serializedMessage.insert(serializedMessage.end(), lineSeparator.begin(), lineSeparator.end());

	try {
		this->m_conn.write(serializedMessage);
	} catch(...) {
		this->m_conn.end();
	}
}

bool Writer::connected() const noexcept {
	return this->m_conn.active();
}

void Writer::close() {
	this->push({ "", "close" });
	this->m_conn.end();
}
