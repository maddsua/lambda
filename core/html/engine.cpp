#include <vector>

#include "./templates.hpp"

using namespace Lambda;

std::string HTML::renderTemplate(const std::string& templateSource, const TemplateProps& props) {

	auto result = templateSource;

	for (const auto& item : props) {
		auto templateVariable = "{{" + item.first + "}}";
		auto matchIdx = std::string::npos;
		while ((matchIdx = result.find(templateVariable)) != std::string::npos) {
			result.replace(matchIdx, templateVariable.size(), item.second);
		}
	}

	return result;
}
