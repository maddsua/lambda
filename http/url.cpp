#include "./http.hpp"

using namespace Lambda::HTTP;

const std::string URL::href() {

	auto href = this->host.size() ? this->protocol + "://" + this->host : std::string();

	if (this->port != "80")
		href += ":" + this->port;
	
	href += this->pathname;

	if (this->searchParams.length())
		href += "?" + this->searchParams.stringify();

	return href;
}

void URL::setHref(const std::string& href) {

	auto hrefNormalized = stringToLowerCase(stringTrim(href));

	auto indexofStart = 0;

	// get protocol
	auto indexofProtocol = hrefNormalized.find("://");
	if (indexofProtocol != std::string::npos) {
		indexofProtocol > 0 ? this->protocol = hrefNormalized.substr(0, indexofProtocol) : "http";
		indexofStart = indexofProtocol + 3;
	}

	//	hostname and port
	auto indexofHost = hrefNormalized.find_first_of('/', indexofStart);
	auto indexofPort = hrefNormalized.find_first_of(":", indexofStart);
	if (indexofHost != std::string::npos) {
		auto indexofHostEnd = indexofPort != std::string::npos ? indexofPort : indexofHost;
		this->host = hrefNormalized.substr(indexofStart, indexofHostEnd - indexofStart);
		indexofStart = indexofHost;
	}

	//	just port
	if (indexofPort != std::string::npos && indexofHost != std::string::npos) {
		this->port = hrefNormalized.substr(indexofPort + 1, (indexofHost + 1) - indexofPort);
	}

	//	path
	auto indexofSearchQuery = hrefNormalized.find_first_of('?');
	auto pathStart = indexofHost == std::string::npos ? 0 : indexofHost;
	auto pathEnd = indexofSearchQuery == std::string::npos ? hrefNormalized.size() : indexofSearchQuery;
	this->pathname = hrefNormalized.substr(pathStart, pathEnd - pathStart);

	//	search query
	if (indexofSearchQuery != std::string::npos) {
		this->searchParams.fromHref(hrefNormalized.substr(indexofSearchQuery + 1));
	}

	refresh();
}

void URL::refresh() {

	if (!this->protocol.size())
		this->protocol = "http";

	if (!this->port.size())
		this->port = "80";

	if (this->pathname.at(0) != '/')
		this->pathname = std::string("/").append(this->pathname);

}
