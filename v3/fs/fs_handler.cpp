#include "./fs.hpp"
#include "../server/server.hpp"

#include <filesystem>

using namespace Lambda;

FileServer::FileServer(FileServerReader* reader) {

	if (!reader) {
		std::runtime_error("FileServerReader reader is null");
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

	//	try directory index file redirect
	if (req.url.path.ends_with('/')) {
		req.url.path.append("index.html");
	}

	auto handle_404 = [&]() {
		
		auto notfound_msg = "path '" + req.url.path + "' not found";
		
		if (req.method != Method::HEAD) {
			wrt.header().set("content-type", "text/plain");
			wrt.header().set("content-length", std::to_string(notfound_msg.size()));
		}

		wrt.write_header(Status::NotFound);

		if (req.method != Method::HEAD) {
			wrt.write(notfound_msg);
		}
	};

	//	todo: fix
	auto file_hit = this->m_reader->open(req.url.path);
	if (!file_hit) {
		handle_404();
		return;
	}

	//	more directory redirects
	switch (file_hit->type()) {
		
		case ServedFile::Type::Directory: {

			if (!req.url.path.ends_with('/')) {
				req.url.path.push_back('/');
			}

			req.url.path.append("index.html");

			//	todo: return redirect instead

			file_hit = this->m_reader->open(req.url.path);
			if (!file_hit || file_hit->type() != ServedFile::Type::File) {
				handle_404();
				return;
			}

		} break;

		case ServedFile::Type::File: break;

		default: {
			handle_404();
		} return;
	}

	wrt.header().set("content-type", Fs::infer_mimetype(file_hit->name()));
	wrt.header().set("last-modified", Date(file_hit->modified()).to_utc_string());
	wrt.header().set("content-length", std::to_string(file_hit->size()));
	wrt.write_header(Status::OK);

	if (req.method == Method::HEAD || !file_hit->size()) {
		return;
	}

	wrt.write(file_hit->content());
}

HandlerFn FileServer::handler() noexcept {
	return [&](Request& req, ResponseWriter& wrt) -> void {
		this->handle_request(req, wrt);
	};
}
