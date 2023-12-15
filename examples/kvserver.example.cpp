#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	auto storage = Lambda::Storage::LocalStorage();

	auto handler = [&](const Request& req, const Context& context) {

		//	get record key
		auto key = req.url.searchParams.get("record");
		if (!key.size()) return HTTP::Response(HTTP::Status(400), "no record key provided");

		switch (req.method) {

			case HTTP::Methods::GET: {
				auto value = storage.getItem(key);
				if (!value.size()) return HTTP::Response(HTTP::Status(404), "object not found");
				return HTTP::Response(value);
			} break;

			case HTTP::Methods::POST: {
				storage.setItem(key, req.body.text());
				return HTTP::Response(204, "created");
			} break;

			case HTTP::Methods::DELETE: {
				storage.removeItem(key);
				return HTTP::Response(200, "deleted");
			} break;
			
			default: break;
		}

		return HTTP::Response(HTTP::Status(400), "bad request");
	};

	HttpServerConfig initparams;
	auto server = HttpServer(handler, initparams);
	server.awaitFinished();

	return 0;
}
