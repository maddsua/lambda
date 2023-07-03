#include "./http.hpp"

using namespace Lambda::HTTP;

std::string URL::href() {

	auto href = this->host.size() ? this->protocol + "://" + this->host : std::string();

	if (this->port != "80")
		href += ":" + this->port;
	
	href += this->pathname;

	if (this->searchParams.length())
		href += "?" + this->searchParams.stringify();

	return href;
}

std::string URL::toHttpPath() {
	
	auto path = this->pathname;

	if (this->searchParams.length())
		path += "?" + this->searchParams.stringify();

	return path;
}

void URL::setHref(const std::string& href) {

	auto hrefNormalized = stringToLowerCase(stringTrim(href));

	auto indexofStart = 0;

	// get protocol
	auto indexofProtocol = hrefNormalized.find("://");
	if (indexofProtocol != std::string::npos) {
		this->protocol = indexofProtocol > 0 ? hrefNormalized.substr(0, indexofProtocol) : "http";
		//hrefNormalized = hrefNormalized.substr(indexofProtocol + 3);
		indexofStart = indexofProtocol + 3;
	}

	//	hostname and port
	auto indexofHost = hrefNormalized.find_first_of('/', indexofStart);
	auto indexofPort = hrefNormalized.find_first_of(":", indexofStart);

	this->host = indexofHost == std::string::npos ? hrefNormalized.substr(indexofStart) : hrefNormalized.substr(indexofStart, indexofPort == std::string::npos ? indexofHost : indexofPort);

	this->port = indexofPort == std::string::npos ? "80" : (indexofHost == std::string::npos ? hrefNormalized.substr(indexofPort + 1) : hrefNormalized.substr(indexofPort + 1, (indexofHost + 1) - indexofPort));

	//	path
	auto indexofSearchQuery = hrefNormalized.find_first_of('?');
	auto pathStart = indexofHost == std::string::npos ? indexofStart : indexofHost;
	auto pathEnd = indexofSearchQuery == std::string::npos ? hrefNormalized.size() : indexofSearchQuery;
	this->pathname = indexofHost == std::string::npos ? "/" : hrefNormalized.substr(pathStart, pathEnd - pathStart);

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
