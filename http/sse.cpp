#include "./http.hpp"

using namespace Lambda;

SSEWriter::SSEWriter(Request& req, ResponseWriter& writer) : m_writer(writer), m_ctx(req.ctx) {

	writer.header().set("connection", "keep-alive");
	writer.header().set("cache-control", "no-cache");
	writer.header().set("content-type", "text/event-stream");

	writer.write_header(Status::OK);
}

SSEWriter::~SSEWriter() {
	try { this->close(); }
		catch(...) { }
}

size_t SSEWriter::push(const SSEevent& msg) {

	if (this->m_closed) {
		throw std::runtime_error("SSE writer closed");
	}

	if (this->m_ctx.done()) {
		this->m_closed = true;
		return 0;
	}

	HTTP::Buffer buffer;

	static const char field_token[] = ": ";
	static const char line_break[] = "\r\n";

	auto field = [&](const std::string& name, const std::string& value) {
		buffer.insert(buffer.end(), name.begin(), name.end());
		buffer.insert(buffer.end(), field_token, field_token + sizeof(field_token) - 1);
		buffer.insert(buffer.end(), value.begin(), value.end());
		buffer.insert(buffer.end(), line_break, line_break + sizeof(line_break) - 1);
	};

	if (msg.event.has_value()) {
		field("event", msg.event.value());
	}

	if (msg.id.has_value()) {
		field("id", msg.id.value());
	}

	field("data", msg.data);

	if (msg.retry.has_value()) {
		field("retry", std::to_string(msg.retry.value()));
	}

	buffer.insert(buffer.end(), line_break, line_break + sizeof(line_break) - 1);

	return this->m_writer.write(buffer);
}

size_t SSEWriter::close() {

	size_t bytes_written = 0;

	if (this->is_open()) {
		bytes_written = this->push({ .event = "close" });
	}

	this->m_ctx.cancel();
	this->m_closed = true;

	return bytes_written;
}

bool SSEWriter::is_open() const noexcept {
	return !this->m_ctx.done() && !this->m_closed;
}
