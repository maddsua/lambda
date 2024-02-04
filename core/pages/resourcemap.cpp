#include "./pages.hpp"

extern char _binary_core_pages_resources_servicepage_html_start;
extern char _binary_core_pages_resources_servicepage_html_end;

using namespace Lambda::Pages;

const std::string Templates::servicePage = std::string(
	&_binary_core_pages_resources_servicepage_html_start,
	&_binary_core_pages_resources_servicepage_html_end - &_binary_core_pages_resources_servicepage_html_start
);
