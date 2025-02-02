#include "./fs.hpp"

#include <filesystem>

using namespace Lambda;

std::string flatten_path(const std::string& path);
void log_access(const Request& req, Status status);

//	generated fn
std::string render_404_page(const std::string& requeted_url);

FileServer::FileServer(FileServerReader* reader) {

	if (!reader) {
		std::logic_error("FileServerReader reader is null");
	}

	this->m_reader = std::unique_ptr<FileServerReader>(reader);
}

void FileServer::handle_request(Request& req, ResponseWriter& wrt) {

	switch (req.method) {

		case Method::HEAD: break;
		case Method::GET: break;

		case Method::OPTIONS: {

			wrt.header().set("allow", "OPTIONS, GET, HEAD");
			wrt.write_header(Status::NoContent);

			if (this->debug) {
				log_access(req, Status::NoContent);
			}

		} return;

		default: {
	
			wrt.write_header(Status::MethodNotAllowed);

			if (this->debug) {
				log_access(req, Status::MethodNotAllowed);
			}

		} return;
	}

	//	flatten request path (remove segments like "./", "/../")
	auto flattened = flatten_path(req.url.path);
	if (flattened != req.url.path) {

		wrt.header().set("location", flattened);	
		wrt.write_header(Status::PermanentRedirect);

		if (this->debug) {
			log_access(req, Status::PermanentRedirect);
		}
		
		return;
	}

	//	apply directory redirect
	auto fs_file_path = req.url.path;
	if (fs_file_path.ends_with('/')) {
		fs_file_path.append("index.html");
	}

	auto file_hit = this->m_reader->open(fs_file_path);
	if (!file_hit) {

		auto response_body = this->html_error_pages ? render_404_page(req.url.path) : ("path '" + req.url.path + "' not found");

		if (req.method != Method::HEAD) {
			wrt.header().set("content-type", this->html_error_pages ? "text/html" : "text/plain");
			wrt.header().set("content-length", std::to_string(response_body.size()));
		}

		wrt.write_header(Status::NotFound);

		if (req.method != Method::HEAD) {
			wrt.write(response_body);
		}

		if (this->debug) {
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

		if (this->debug) {
			log_access(req, Status::Found);
		}

		return;
	}

	//	todo: add caching/etag support

	wrt.header().set("content-type", Fs::infer_mimetype(file_hit->name()));
	wrt.header().set("last-modified", Date(file_hit->modified()).to_utc_string());
	wrt.header().set("content-length", std::to_string(file_hit->size()));
	wrt.write_header(Status::OK);

	if (this->debug) {
		log_access(req, Status::OK);
	}

	if (req.method == Method::HEAD || !file_hit->size()) {
		return;
	}

	//	todo: support chunked reads

	wrt.write(file_hit->content());
}

HandlerFn FileServer::handler() noexcept {
	return [&](Request& req, ResponseWriter& wrt) -> void {
		this->handle_request(req, wrt);
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

void log_access(const Request& req, Status status) {

	auto unwrap_method = [](Method method) {
		switch (method) {
			case Method::GET: return "GET";
			case Method::POST: return "POST";
			case Method::PATCH: return "PATCH";
			case Method::PUT: return "PUT";
			case Method::DEL: return "DEL";
			case Method::OPTIONS: return "OPTIONS";
			case Method::HEAD: return "HEAD";
			case Method::TRACE: return "CONNECT";
			case Method::CONNECT: return "CONNECT";
			default: return "NULL";
		}
	};

	fprintf(stdout, "%s DEBUG Lambda::Fileserver %s %s %s -> %i\n",
		Date().to_log_string().c_str(),
		req.remote_addr.hostname.c_str(),
		unwrap_method(req.method),
		req.url.path.c_str(),
		static_cast<std::underlying_type_t<Status>>(status)
	);
}

//	generated fn
std::string render_404_page(const std::string& requeted_url) {
	return (
		"<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>404 | Not found</title><style> * { box-sizing: border-box; margin: 0; padding: 0; } body { display: flex; flex-direction: column; height: 100vh; align-items: center; justify-content: center; font-family: sans-serif; color: black; background-color: white; } @media (prefers-color-scheme: dark) { body { color: whitesmoke; background-color: #171717; } } .message { display: flex; flex-direction: row; gap: 1.25rem; align-items: center; justify-content: flex-start; flex-shrink: 0; } .status-code { font-weight: 400; font-size: 6rem; } .content { display: flex; flex-direction: column; gap: 0.625rem; align-items: flex-start; justify-content: flex-start; flex-shrink: 0; max-width: 20rem; } .message-title { font-weight: 400; font-size: 2.25rem; } .message-content { font-weight: 400; font-size: 1rem; } </style></head><body><div class=\"message\"><div class=\"status-code\">404</div><div class=\"content\"><div class=\"message-title\">Not found</div><div class=\"message-content\">The requested URL '" + requeted_url + "' doesn't point to any valid resources</div></div></div></body></html>"
	);
}
