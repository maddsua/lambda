#include "../internal.hpp"

using namespace Lambda::Server;
using namespace Lambda::Server::Pages;

std::string Pages::renderTemplate(const std::string& templateSource, const TemplateProps& props) {

	auto result = templateSource;

	for (const auto& item : props) {
		auto templateLiteral = "{{" + item.first + "}}";
		auto matchIdx = std::string::npos;
		while ((matchIdx = result.find(templateLiteral)) != std::string::npos) {
			result.replace(matchIdx, templateLiteral.size(), item.second);
		}
	}

	return result;
}
