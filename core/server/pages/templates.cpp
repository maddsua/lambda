#include "../internal.hpp"

extern char _binary_core_server_pages_resources_servicepage_html_start;
extern char _binary_core_server_pages_resources_servicepage_html_end;

using namespace Lambda::Server;
using namespace Lambda::Server::Pages;
using namespace Lambda::Server::Pages::Templates;

const std::string& Templates::servicePage() noexcept {

	static const auto pageTemplate = std::string(
		&_binary_core_server_pages_resources_servicepage_html_start,
		&_binary_core_server_pages_resources_servicepage_html_end - &_binary_core_server_pages_resources_servicepage_html_start
	);

	return pageTemplate;
}
