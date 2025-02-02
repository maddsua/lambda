#include "./fs.hpp"
#include "../server/server.hpp"

#include <filesystem>

using namespace Lambda;

StaticServer::StaticServer(const std::string& root) {

	//	first of all check if destination exists
	if (!std::filesystem::exists(root)) {
		throw std::runtime_error("StaticServer root path '" + root + "' doesn't exist");
	}

	//	check if we got a directory or a file there
	if (std::filesystem::is_directory(root)) {
		this->m_reader_ptr = std::unique_ptr<Lambda::FsServeReader>(new FsStaticReader(root));
		return;
	}

	//	todo: add tar support

	throw std::runtime_error("StaticServer root path '" + root + "' cannot be served");
}

//	todo: return noice error pages
//	todo: fix file dates

void StaticServer::handle(Request& req, ResponseWriter& wrt) {

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

	auto file_hit = this->m_reader_ptr->open(req.url.path);

	//	return 404 if not found
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

	auto content = file_hit->content();

	wrt.header().set("content-type", Fs::infer_mimetype(file_hit->name));
	wrt.header().set("last-modified", Date(file_hit->modified).to_utc_string());

	if (req.method == Method::HEAD || !file_hit->size) {
		wrt.header().set("content-length", std::to_string(file_hit->size));
		wrt.write_header(Status::OK);
		return;
	}

	wrt.header().set("content-length", std::to_string(content.size()));
	wrt.write_header(Status::OK);

	wrt.write(content);
}

HandlerFn StaticServer::handler() noexcept {
	return [&](Request& req, ResponseWriter& wrt) -> void {
		this->handle(req, wrt);
	};
}
