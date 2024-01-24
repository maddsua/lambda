#include "../polyfill/polyfill.hpp"
#include "./router.hpp"

using namespace Lambda;
using namespace Lambda::Server::Handlers;

const std::initializer_list<char> eraseAfterPoints = { '?', '#' };

std::optional<RouteContext> matchRoute(const Router& router, const std::string& pathname) {

	auto routename = pathname;
	for (const auto& breakpoint : eraseAfterPoints) {
		auto idx = routename.find(breakpoint);
		if (idx != std::string::npos) {
			if (idx > 0) routename.erase(idx - 1);
				else routename = '/';
		}
	}

	if (!pathname.starts_with('/')) {
		routename = '/' + pathname;
	}

	if (pathname.ends_with('/')) {
		routename.erase(routename.size() - 1);
	}

	auto directMatch = router.find(pathname);
	if (directMatch != router.end()) {
		return directMatch->second;
	}

	return;
}
