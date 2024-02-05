#include "./html.hpp"

#include <optional>

using namespace Lambda;
using namespace Lambda::HTML;

std::optional<std::string> getTemplateVariableFromSlice(const std::string& source, size_t startpos, size_t endpost);

std::string HTML::renderTemplate(const std::string& templateSource, const TemplateProps& props) {

	auto result = templateSource;

	size_t searchStart = 0;
	size_t templateOpen = std::string::npos;

	while ((templateOpen = result.find("{{", searchStart)) != std::string::npos) {

		auto templateClose = result.find("}}", templateOpen);
		if (templateClose == std::string::npos) break;

		auto templateVar = getTemplateVariableFromSlice(result, templateOpen + 2, templateClose);
		if (!templateVar.has_value()) {
			searchStart = templateClose + 2;
			continue;
		}

		auto valueItr = props.find(templateVar.value());
		auto templContent = valueItr != props.end() ? valueItr->second : "";
		result.replace(templateOpen, templateClose + 2 - templateOpen, templContent);
		searchStart = templateOpen + templContent.size();
	}

	return result;
}

std::optional<std::string> getTemplateVariableFromSlice(const std::string& source, size_t startpos, size_t endpost) {

	static auto isVarname = [](char chr) {
		return chr == '_' || isalnum(chr);
	};

	static auto isWhitespace = [](char chr) {
		return chr == ' ' || chr == '\t';
	};

	if (startpos > source.size() || endpost > source.size()) {
		return std::nullopt;
	}

	enum struct State {
		Prepadd, Variable, Postpadd
	};

	auto state = State::Prepadd;

	size_t varnameStart = std::string::npos;
	size_t varnameEnd = std::string::npos;

	for (size_t i = startpos; i < endpost; i++) {

		auto item = source.at(i);

		switch (state) {

			case State::Prepadd: {

				if (isVarname(item)) {
					varnameStart = i;
					state = State::Variable;
				} else if (!isWhitespace(item)) {
					return std::nullopt;
				}
					
			} break;

			case State::Variable: {

				if (isWhitespace(item)) {
					varnameEnd = i;
					state = State::Postpadd;
				} else if (!isVarname(item)) {
					return std::nullopt;
				}

			} break;

			case State::Postpadd: {

				if (!isWhitespace(item)) {
					return std::nullopt;
				}

			} break;
			
			default: break;
		}
	}

	if (varnameStart == std::string::npos) {
		return std::nullopt;
	}

	auto varnameLength = ((varnameEnd != std::string::npos ? varnameEnd : endpost) - varnameStart);
	return source.substr(varnameStart, varnameLength);
}
