#include "../lambda.hpp"
#include <iostream>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Encoding;
using namespace Lambda::Storage;

struct ServerPass {
	VFS* vfs = nullptr;
	KV* kvstore = nullptr;
};

const std::string kv_help_message = R"(
	Commands are: GET, SET, DEL

	Usage examples:

	SET user_2_name maddsua
	>> OK
	
	GET user_2_name
	>> maddsua

	DEL user_2_name
	>> OK
)";

void callback(Lambda::Network::HTTPServer& connection, Lambda::Context& context) {

	auto request = connection.receiveMessage();

	std::cout << "Serving \"" << request.url.pathname << "\"" << std::endl;

	//	handle regular http requests
	if (!request.url.pathname.starts_with("/ws")) {

		const auto vfs = ((ServerPass*)context.passtrough)->vfs;
		
		if (vfs == nullptr) {
			auto response = serviceResponse(500, "HTTP/500 Error: VFS unavailable");;
			connection.sendMessage(response);
			return;
		}

		//	transform request url a bit
		auto filepath = request.url.pathname;
		if (filepath.ends_with("/")) filepath.replace(filepath.size() - 1, 1, "/index.html");
		if (filepath.starts_with("/")) filepath.erase(0, 1);

		//	find file in vfs
		auto file = vfs->read(filepath);
		if (!file.size()) {
			auto response = serviceResponse(404, "Resource \"" + request.url.pathname + "\" does not exist");
			connection.sendMessage(response);
			return;
		}

		auto response = Response();
		response.body = file;

		auto pathExtIdx = filepath.find_last_of('.');
		if (pathExtIdx != std::string::npos)
			response.headers.append("content-type", getExtMimetype(filepath.substr(pathExtIdx)));
		
		connection.sendMessage(response);
		return;
	}

	//	websocket part
	std::cout << "Conntected a websocket" << std::endl;

	auto websock = connection.upgradeToWebsocket(request);

	const auto kv = ((ServerPass*)context.passtrough)->kvstore;

	while (websock.isAlive()) {

		if (!websock.availableMessage()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		auto messages = websock.getMessages();

		for (const auto& msg : messages) {

			std::cout << msg.timestamp << ": " << msg.content << std::endl;

			auto msgtokens = stringSplit(msg.content, " ");

			if (!msgtokens.size()) {
				websock.sendMessage("invalid kv command format");
				continue;
			}

			auto command = stringToLowerCase(static_cast<const std::string>(msgtokens.at(0)));

			if (command == "help") {

				websock.sendMessage(kv_help_message);

			} else if (command == "get") {

				if (msgtokens.size() < 2) {
					websock.sendMessage("second argument should me record id");
					continue;
				}

				auto recordPresent = kv->has(msgtokens.at(1));
				if (!recordPresent) {
					websock.sendMessage("[record not found]");
					continue;
				}

				websock.sendMessage(kv->get(msgtokens.at(1)).value);

			} else if (command == "set") {

				if (msgtokens.size() < 3) {
					websock.sendMessage("command should look like this: set [record_id] [value]");
					continue;
				}
				
				kv->set(msgtokens.at(1), msgtokens.at(2));

				websock.sendMessage("ok");

			} else if (command == "del") {

				if (msgtokens.size() < 2) {
					websock.sendMessage("second argument should me record id");
					continue;
				}

				auto recordPresent = kv->has(msgtokens.at(1));
				if (!recordPresent) {
					websock.sendMessage("[record wasn't even present]");
					continue;
				}

				kv->del(msgtokens.at(1));
				
				websock.sendMessage("ok");
				
			} else {
				websock.sendMessage("unknown command");
			}
		}
	}

	std::cout << "Websocket disconnected" << std::endl;
}

int main() {

	const int port = 8080;

	auto server = Lambda::Server(port);

	server.flags.compressionUseBrotli = true;
	server.flags.compressionUseGzip = true;

	ServerPass passthough;
	passthough.kvstore = new KV();
	server.enablePasstrough(&passthough);

	auto vfs = new VFS();
	auto vfsload = vfs->loadSnapshot("examples/content/demo.dist.tar.gz");
	if (vfsload.isError()) std::cout << "Failed to load VFS: " << vfsload.what() << std::endl;
	else passthough.vfs = vfs;

	std::cout << "Server started at http://localhost:" + std::to_string(port) << std::endl;

	server.setServerCallback(&callback);

	while (server.isAlive()) {
		
		if (server.hasNewLogs()) {
			puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return 0;
}
