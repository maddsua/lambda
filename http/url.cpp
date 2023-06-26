#include "http.hpp"

HTTP::URL::URL() {
	//	and whe don't do a thing here
}

HTTP::URL::URL(const std::string href) {
	setHref(href);
}

const std::string HTTP::URL::href() {

	auto href = this->host.size() ? this->protocol + "://" + this->host : std::string();

	if (this->port != "80")
		href += ":" + this->port;
	
	href += this->pathname;

	if (this->searchParams.length())
		href += "?" + this->searchParams.stringify();

	return href;
}

void HTTP::URL::setHref(const std::string& href) {

	auto indexofStart = 0;

	// get protocol
	auto indexofProtocol = href.find("://");
	if (indexofProtocol != std::string::npos) {
		indexofProtocol > 0 ? this->protocol = href.substr(0, indexofProtocol) : "http";
		indexofStart = indexofProtocol + 3;
	}

	//	hostname and port
	auto indexofHost = href.find_first_of('/', indexofStart);
	auto indexofPort = href.find_first_of(":", indexofStart);
	if (indexofHost != std::string::npos) {
		auto indexofHostEnd = indexofPort != std::string::npos ? indexofPort : indexofHost;
		this->host = href.substr(indexofStart, indexofHostEnd - indexofStart);
		indexofStart = indexofHost;
	}

	//	just port
	if (indexofPort != std::string::npos && indexofHost != std::string::npos) {
		this->port = href.substr(indexofPort + 1, (indexofHost + 1) - indexofPort);
	}

	//	path
	auto indexofSearchQuery = href.find_first_of('?');
	auto pathStart = indexofHost == std::string::npos ? 0 : indexofHost;
	auto pathEnd = indexofSearchQuery == std::string::npos ? href.size() : indexofSearchQuery;
	this->pathname = href.substr(pathStart, pathEnd - pathStart);

	//	search query
	if (indexofSearchQuery != std::string::npos) {
		this->searchParams = href.substr(indexofSearchQuery + 1);
	}

	refresh();
}

void HTTP::URL::refresh() {

	if (!this->protocol.size())
		this->protocol = "http";

	if (!this->port.size())
		this->port = "80";

	if (this->pathname.at(0) != '/')
		this->pathname = std::string("/").append(this->pathname);

}
