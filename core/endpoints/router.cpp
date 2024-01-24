#include "../polyfill/polyfill.hpp"
#include "./router.hpp"

using namespace Lambda;
using namespace Lambda::Endpoints;

const std::initializer_list<char> eraseAfterPoints = { '?', '#' };

Router::Router(const RouteContext& routerInit) {
	this->staticHandler = routerInit;
}

Router::Router(const HandlerFunction& routerInit) {
	this->staticHandler = {
		routerInit
	};
}

Router::Router(const std::initializer_list<std::pair<std::string, RouteContext>>& routerInit) {

	for (const auto& item : routerInit) {

		auto keyNormalized = Strings::toLowerCase(item.first);
		if (keyNormalized.ends_with('/')) {
			keyNormalized.erase(keyNormalized.size() - 1);
		}

		if (!keyNormalized.starts_with('/')) {
			keyNormalized.insert(keyNormalized.begin(), '/');
		}

		if (keyNormalized.ends_with("/*")) {
			keyNormalized.erase(keyNormalized.size() - 2);
		}

		this->m_router[keyNormalized] = item.second;
	}
}

Router& Router::operator= (const Router& other) noexcept {
	this->m_router = other.m_router;
	this->staticHandler = other.staticHandler;
	return *this;
}
Router& Router::operator= (Router&& other) noexcept {
	this->m_router = std::move(other.m_router);
	this->staticHandler = other.staticHandler;
	return *this;
}

std::optional<RouteContext> Router::match(const std::string& pathname) const {

	if (this->staticHandler.has_value()) {
		return this->staticHandler;
	}

	auto routename = pathname;
	for (const auto& breakpoint : eraseAfterPoints) {
		auto idx = routename.find(breakpoint);
		if (idx != std::string::npos) {
			if (idx > 0) routename.erase(idx - 1);
				else routename = '/';
		}
	}

	if (!pathname.starts_with('/')) {
		routename.insert(routename.begin(), '/');
	}

	if (pathname.ends_with('/')) {
		routename.erase(routename.size() - 1);
	}

	auto directMatch = this->m_router.find(pathname);
	if (directMatch != this->m_router.end()) {
		return directMatch->second;
	}

	return std::nullopt;
}
