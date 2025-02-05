#include "./http.hpp"

using namespace Lambda;

HTTP::Buffer SSEevent::to_buffer() const {

	HTTP::Buffer buffer;

	static const char field_token[] = ": ";
	static const char line_break[] = "\r\n";

	auto field = [&](const std::string& name, const std::string& value) {
		buffer.insert(buffer.end(), name.begin(), name.end());
		buffer.insert(buffer.end(), field_token, field_token + sizeof(field_token) - 1);
		buffer.insert(buffer.end(), value.begin(), value.end());
		buffer.insert(buffer.end(), line_break, line_break + sizeof(line_break) - 1);
	};

	if (this->event.has_value()) {
		field("event", this->event.value());
	}

	if (this->id.has_value()) {
		field("id", this->id.value());
	}

	field("data", this->data);

	if (this->retry.has_value()) {
		field("retry", std::to_string(this->retry.value()));
	}

	buffer.insert(buffer.end(), line_break, line_break + sizeof(line_break) - 1);

	return buffer;
}

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

	if (!this->writable()) {
		throw std::runtime_error("SSE writer closed");
	}

	return this->m_writer.write(msg.to_buffer());
}

bool SSEWriter::writable() const noexcept {
	return this->m_ok && this->m_writer.writable();
}

size_t SSEWriter::close() {

	if (!this->writable()) {
		return 0;
	}

	return this->write({ .event = "close" });
}
