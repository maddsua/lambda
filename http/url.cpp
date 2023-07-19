#include "./http.hpp"

using namespace Lambda::HTTP;

std::string URL::href() const {

	auto href = this->host.size() ? this->protocol + "://" + this->host : std::string();

	if (this->port != "80")
		href += ":" + this->port;
	
	href += this->pathname;

	if (this->searchParams.length())
		href += "?" + this->searchParams.stringify();

	return href;
}

std::string URL::toHttpPath() const {
	
	auto path = this->pathname;

	if (this->searchParams.length())
		path += "?" + this->searchParams.stringify();

	return path;
}

URL::URL(const std::string href) {
	setHref(href);
}

void URL::setHref(const std::string& href) {

	auto hrefNormalized = stringToLowerCase(stringTrim(href));

	auto posStart = 0;

	// get protocol
	auto posProtocol = hrefNormalized.find("://");
	if (posProtocol != std::string::npos) {
		this->protocol = posProtocol > 0 ? hrefNormalized.substr(0, posProtocol) : "http";
		//hrefNormalized = hrefNormalized.substr(indexofProtocol + 3);
		posStart = posProtocol + 3;
	}

	//	hostname and port
	auto posHost = hrefNormalized.find_first_of('/', posStart);
	auto posPort = hrefNormalized.find_first_of(':', posStart);
	auto posHostEnd = (posHost != std::string::npos) ? posHost : posPort;

	this->host = posHost == posStart ? "localhost" : (posHostEnd == std::string::npos) ? hrefNormalized.substr(posStart) : hrefNormalized.substr(posStart, posHostEnd - posStart);

	this->port = posPort == std::string::npos ? "80" : (posHost == std::string::npos ? hrefNormalized.substr(posPort + 1) : hrefNormalized.substr(posPort + 1, (posHost + 1) - posPort));

	//	path
	auto indexofSearchQuery = hrefNormalized.find_first_of('?');
	auto pathStart = posHost == std::string::npos ? posStart : posHost;
	auto pathEnd = indexofSearchQuery == std::string::npos ? hrefNormalized.size() : indexofSearchQuery;
	this->pathname = posHost == std::string::npos ? "/" : hrefNormalized.substr(pathStart, pathEnd - pathStart);

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

bool URL::isWWW() const {
	return !(this->host == "localhost" || this->host == "127.0.0.1" || !this->host.size()) && this->protocol.starts_with("http");
}
