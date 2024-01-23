#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	auto storage = Lambda::Storage::LocalStorage();

	auto handler = [&](const Request& req, const Context& context) {

		auto console = context.console;

		//	get record key
		auto key = req.url.searchParams.get("record");
		if (!key.size()) return HTTP::Response(HTTP::Status(400), "no record key provided");

		switch (req.method) {

			case HTTP::Methods::GET: {
				return storage.hasItem(key) ?
					HTTP::Response(storage.getItem(key)) :
					HTTP::Response(HTTP::Status(404), "object not found");
			} break;

			case HTTP::Methods::POST: {
				storage.setItem(key, req.body.text());
				return HTTP::Response(202, "created");
			} break;

			case HTTP::Methods::DEL: {
				storage.removeItem(key);
				return HTTP::Response(200, "deleted");
			} break;
			
			default: break;
		}

		return HTTP::Response(HTTP::Status(400), "bad request");
	};

	ServerConfig initparams;
	initparams.loglevel.logRequests = true;
	auto server = Server(handler, initparams);
	server.awaitFinished();

	return 0;
}
