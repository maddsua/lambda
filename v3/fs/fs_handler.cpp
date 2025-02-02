#include "./fs.hpp"
#include "../server/server.hpp"

#include <filesystem>

using namespace Lambda;

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

FileServer::FileServer(FileServerReader* reader) {

	if (!reader) {
		std::logic_error("FileServerReader reader is null");
	}

	this->m_reader = std::unique_ptr<FileServerReader>(reader);
}

//	todo: return noice error pages

void FileServer::handle_request(Request& req, ResponseWriter& wrt) {

	switch (req.method) {

		case Method::HEAD: break;
		case Method::GET: break;

		case Method::OPTIONS: {
			wrt.header().set("allow", "OPTIONS, GET, HEAD");
			wrt.write_header(Status::NoContent);
		} return;

		default: {
			wrt.write_header(Status::MethodNotAllowed);
		} return;
	}

	//	flatten request path (remove segments like "./", "/../")
	auto flattened = flatten_path(req.url.path);
	if (flattened != req.url.path) {
		wrt.header().set("location", flattened);	
		wrt.write_header(Status::PermanentRedirect);
		return;
	}

	//	apply directory redirect
	auto fs_file_path = req.url.path;
	if (fs_file_path.ends_with('/')) {
		fs_file_path.append("index.html");
	}

	auto file_hit = this->m_reader->open(fs_file_path);
	if (!file_hit) {
		
		auto notfound_msg = "path '" + req.url.path + "' not found";
		
		if (req.method != Method::HEAD) {
			wrt.header().set("content-type", "text/plain");
			wrt.header().set("content-length", std::to_string(notfound_msg.size()));
		}

		wrt.write_header(Status::NotFound);

		if (req.method != Method::HEAD) {
			wrt.write(notfound_msg);
		}
		
		return;
	}

	//	return redirect for directories
	if (file_hit->type() == ServedFile::Type::Directory) {

		if (!req.url.path.ends_with('/')) {
			req.url.path.push_back('/');
		}

		wrt.header().set("location", req.url.to_string());	
		wrt.write_header(Status::Found);
		return;
	}

	wrt.header().set("content-type", Fs::infer_mimetype(file_hit->name()));
	wrt.header().set("last-modified", Date(file_hit->modified()).to_utc_string());
	wrt.header().set("content-length", std::to_string(file_hit->size()));
	wrt.write_header(Status::OK);

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
