//	An example KV-cache server (redis on a budget)

#include <cstdio>

#include "../lambda.hpp"

#include <unordered_map>
#include <mutex>

using namespace Lambda;

int main() {

	std::unordered_map<std::string, std::string> kv;
	std::mutex lock;

	auto handler = [&](Request& req, ResponseWriter& wrt) -> void {

		auto key = req.url.search.get("r");
		if (key.empty()) {
			wrt.write_header(Status::BadRequest);
			wrt.write("no record key provided");
			return;
		}

		switch (req.method) {

			case Method::GET: {

				std::lock_guard _scope_lock(lock);

				auto record = kv.find(key);
				if (record == kv.end()) {
					wrt.write_header(Status::NotFound);
					wrt.write("object not found");
					return;
				}

				wrt.write(record->second);

			} break;

			case Method::POST: {
				std::lock_guard _scope_lock(lock);
				kv[key] = req.body.text();
				wrt.write_header(Status::Created);
			} break;

			case Method::DEL: {
				std::lock_guard _scope_lock(lock);
				kv.erase(key);
				wrt.write_header(Status::Accepted);
			} break;
			
			default: {
				wrt.write_header(Status::MethodNotAllowed);
				wrt.write("method not allowed");
			} break;
		}
	};

	auto server = Server(handler, { .debug = true });
	server.serve();

	return 0;
}
