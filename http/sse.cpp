#include "./http.hpp"

using namespace Lambda;

SSEWriter::SSEWriter(ResponseWriter& writer) : m_writer(writer) {

	writer.header().set("connection", "keep-alive");
	writer.header().set("cache-control", "no-cache");
	writer.header().set("content-type", "text/event-stream");

	writer.write_header(Status::OK);
}

SSEWriter::~SSEWriter() {
	try { this->close(); }
		catch(...) { }
}

size_t SSEWriter::write(const SSEevent& msg) {

	if (!this->is_writable()) {
		throw std::runtime_error("SSE writer closed");
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

bool SSEWriter::is_writable() const noexcept {
	return this->m_ok && this->m_writer.writable();
}

size_t SSEWriter::close() {

	if (!this->is_writable()) {
		return 0;
	}

	return this->write({ .event = "close" });
}
