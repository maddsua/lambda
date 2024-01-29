#include <vector>

#include "./templates.hpp"

using namespace Lambda;

std::string HTML::renderTemplate(const std::string& templateSource, const TemplateProps& props) {

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
