#include "../server.hpp"
#include "../handlers.hpp"
#include "../http.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::HTTPServer;
using namespace Lambda::Server::Handlers;

void Handlers::httpExtendedHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept {

}
