#include "./html.hpp"

extern char _binary_core_html_resources_servicepage_html_start;
extern char _binary_core_html_resources_servicepage_html_end;

using namespace Lambda;
using namespace Lambda::HTML;

const std::string Templates::servicePage = std::string(
	&_binary_core_html_resources_servicepage_html_start,
	&_binary_core_html_resources_servicepage_html_end - &_binary_core_html_resources_servicepage_html_start
);
