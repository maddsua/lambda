#include "./fs.hpp"
#include "../http/http_utils.hpp"
#include "../base64/base64.hpp"
#include "../log/log.hpp"

#include <filesystem>
#include <cstring>

using namespace Lambda;

const size_t max_oneshot_response_size = 1024 * 1024;

std::string flatten_path(const std::string& path);
std::string hash_content(const HTTP::Buffer& data);
void log_access(const Request& req, Status status);

struct Range {
	size_t begin;
	size_t end;
};

std::optional<Range> get_range(const Headers& headers, size_t file_size);

//	generated fn
std::string render_404_page(const std::string& requeted_url);

void FileServer::handle(Request& req, ResponseWriter& wrt) {

	switch (req.method) {

		case Method::HEAD: break;
		case Method::GET: break;

		case Method::OPTIONS: {

			wrt.header().set("allow", "OPTIONS, GET, HEAD");
			wrt.write_header(Status::NoContent);

			if (this->opts.debug) {
				log_access(req, Status::NoContent);
			}

		} return;

		default: {
	
			wrt.write_header(Status::MethodNotAllowed);

			if (this->opts.debug) {
				log_access(req, Status::MethodNotAllowed);
			}

		} return;
	}

	//	flatten request path (remove segments like "./", "/../")
	auto flattened = flatten_path(req.url.path);
	if (flattened != req.url.path) {

		wrt.header().set("location", flattened);	
		wrt.write_header(Status::PermanentRedirect);

		if (this->opts.debug) {
			log_access(req, Status::PermanentRedirect);
		}
		
		return;
	}

	//	apply directory redirect
	auto fs_file_path = req.url.path;
	if (fs_file_path.ends_with('/')) {
		fs_file_path.append("index.html");
	}

	auto file_hit = this->m_reader.open(fs_file_path);
	if (!file_hit) {

		if (req.method != Method::HEAD) {
			wrt.header().set("content-type", this->opts.html_error_pages ? "text/html" : "text/plain");
		}

		wrt.write_header(Status::NotFound);

		if (req.method != Method::HEAD) {
			wrt.write(this->opts.html_error_pages ? render_404_page(req.url.path) : ("path '" + req.url.path + "' not found"));
		}

		if (this->opts.debug) {
			log_access(req, Status::NotFound);
		}
		
		return;
	}

	//	return redirect for directories
	if (file_hit->type() == ServedFile::Type::Directory) {

		if (!req.url.path.ends_with('/')) {
			req.url.path.push_back('/');
		}

		wrt.header().set("location", req.url.href());
		wrt.write_header(Status::Found);

		if (this->opts.debug) {
			log_access(req, Status::Found);
		}

		return;
	}

	wrt.header().set("cache-control", "max-age=604800, must-revalidate");
	wrt.header().set("accept-ranges", "bytes");
	wrt.header().set("vary", "range");

	auto last_modified = Date(file_hit->modified()).to_utc_string();
	if (req.headers.get("if-modified-since") == last_modified) {

		wrt.write_header(Status::NotModified);

		if (this->opts.debug) {
			log_access(req, Status::NotModified);
		}

		return;
	}

	wrt.header().set("last-modified", last_modified);

	auto range_opt = get_range(req.headers, file_hit->size());
	if (range_opt.has_value()) {

		auto range = range_opt.value();

		if (range.end > file_hit->size() || range.begin >= range.end) {
			wrt.write_header(Status::RequestedRangeNotSatisfiable);
			return;
		}

		auto partial_content = file_hit->content(range.begin, range.end);
		auto etag = hash_content(partial_content);
	
		if (req.headers.get("if-none-match") == etag) {

			wrt.write_header(Status::NotModified);

			if (this->opts.debug) {
				log_access(req, Status::NotModified);
			}

			return;
		}
	
		wrt.header().set("etag", etag);
		wrt.header().set("content-type", Fs::infer_mimetype(file_hit->name()));
		wrt.header().set("content-length", std::to_string(partial_content.size()));
		wrt.write_header(Status::PartialContent);
		wrt.write(partial_content);

		return;
	}

	HTTP::Buffer complete_content;
	auto stream_response = file_hit->size() >= max_oneshot_response_size;

	if (!stream_response) {

		complete_content = file_hit->content();
		auto etag = hash_content(complete_content);
	
		if (req.headers.get("if-none-match") == etag) {

			wrt.write_header(Status::NotModified);

			if (this->opts.debug) {
				log_access(req, Status::NotModified);
			}

			return;
		}
	
		wrt.header().set("etag", etag);
	}

	wrt.header().set("content-type", Fs::infer_mimetype(file_hit->name()));
	wrt.header().set("content-length", std::to_string(file_hit->size()));

	wrt.write_header(Status::OK);

	if (this->opts.debug) {
		log_access(req, Status::OK);
	}

	if (req.method == Method::HEAD || !file_hit->size()) {
		return;
	}

	//	if file size is a bit too large, read and write it in chunks
	if (stream_response) {

		for (size_t written = 0; written < file_hit->size(); ) {

			auto chunk = file_hit->content();
			if (chunk.empty()) {
				break;
			}

			written += chunk.size();
			wrt.write(chunk);		
		}

		return;
	}

	wrt.write(complete_content);
}

HandlerFn FileServer::handler_fn() {
	return [&](Request& req, ResponseWriter& wrt) -> void {
		this->handle(req, wrt);
	};
}

std::string flatten_path(const std::string& path) {

	auto normalized = std::filesystem::path(path).lexically_normal();
	if (normalized.preferred_separator == '/') {
		return normalized.string();
	}

	auto normalized_string = normalized.string();

	for (auto& rune : normalized_string) {
		if (rune == normalized.preferred_separator) {
			rune = '/';
		}
	}

	return normalized_string;
}

std::string hash_content(const HTTP::Buffer& data) {

	auto std_hash = std::hash<std::string>{}(std::string(data.begin(), data.end()));
	auto hash_buff = HTTP::Buffer(sizeof(std_hash));
	std::memcpy(hash_buff.data(), &std_hash, sizeof(std_hash));

	return Encoding::Base64::encode(hash_buff);
}

void log_access(const Request& req, Status status) {
	Log::err("{} DEBUG Lambda::Fs {} {} {} -> {}", {
		Date().to_log_string(),
		req.remote_addr.hostname,
		HTTP::method_to_string(req.method),
		req.url.path,
		static_cast<std::underlying_type_t<Status>>(status)
	});
}

std::optional<Range> get_range(const Headers& headers, size_t file_size) {

	auto range_header = headers.get("range");
	if (range_header.empty()) {
		return std::nullopt;
	}

	static const std::string prefix = "bytes=";

	if (!range_header.starts_with(prefix)) {
		return std::nullopt;
	}

	auto range_begin = prefix.size();
	auto range_end = range_header.size();

	auto range_token = std::string::npos;
	for (size_t idx = range_begin; idx < range_end; idx++) {
		if (range_header[idx] == '-') {
			range_token = idx;
			break;
		}
	}
	
	if (range_token == std::string::npos) {
		return std::nullopt;
	}

	try {
		return Range {
			.begin = range_token - range_begin > 0 ? std::stoul(range_header.substr(range_begin, range_token - range_begin)) : 0,
			.end = range_end - range_token > 1 ? std::stoul(range_header.substr(range_token + 1)) : file_size
		};
	} catch(...) {
		return std::nullopt;
	}
}

//	generated fn
std::string render_404_page(const std::string& requeted_url) {
	return (
		"<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>404 | Not found</title><style> * { box-sizing: border-box; margin: 0; padding: 0; } body { display: flex; flex-direction: column; height: 100vh; align-items: center; justify-content: center; font-family: sans-serif; color: black; background-color: white; } @media (prefers-color-scheme: dark) { body { color: whitesmoke; background-color: #171717; } } .message { display: flex; flex-direction: row; gap: 1.25rem; align-items: center; justify-content: flex-start; flex-shrink: 0; } .status-code { font-weight: 400; font-size: 6rem; } .content { display: flex; flex-direction: column; gap: 0.625rem; align-items: flex-start; justify-content: flex-start; flex-shrink: 0; max-width: 20rem; } .message-title { font-weight: 400; font-size: 2.25rem; } .message-content { font-weight: 400; font-size: 1rem; } </style></head><body><div class=\"message\"><div class=\"status-code\">404</div><div class=\"content\"><div class=\"message-title\">Not found</div><div class=\"message-content\">The requested URL '" + requeted_url + "' doesn't point to any valid resources</div></div></div></body></html>"
	);
}
