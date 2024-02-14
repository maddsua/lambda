
#include "../../lambda.hpp"
using namespace Lambda;

const auto requestHandlerA = [](const Request& req, const Context& context) {
	return Response("Response from instance A");
};

const auto requestHandlerB = [](const Request& req, const Context& context) {
	return Response("Response from instance B");
};

int main(int argc, char const *argv[]) {

	{
		puts("Starting instance A...");
		auto serverA = LambdaInstance(requestHandlerA, {});
		std::thread([&](){
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
			serverA.shutdownn();
		}).detach();
		serverA.awaitFinished();
	}

	{
		puts("Starting instance B...");
		auto serverB = LambdaInstance(requestHandlerB, {});
		std::thread([&](){
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
			serverB.shutdownn();
		}).detach();
		serverB.awaitFinished();
	}

	return 0;
}
