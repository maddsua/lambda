#include "../core.hpp"
#include <stdexcept>

using namespace HTTP;
using namespace Strings;

/**
 * Just a memo of how URL schema looks
 * https://user:password@example.com:443/document?search=query#fragment
*/

void URL::parse(const std::string& href) {

	try {

		size_t docStart = 0;
		size_t cursor = 0;

		//	parse server address and stuff
		if (!href.starts_with('/')) {

			//	alright so let's start with getting url schema. that's the "http" thing
			cursor = href.find("://");
			if (cursor == std::string::npos) {
				throw std::runtime_error("Protocol not specified");
			}

			this->protocol = href.substr(0, cursor);
			cursor += 3;

			//	find separator between hostname and stuff and document path and it's stuff
			docStart = href.find_first_of('/', cursor);

			auto addrString = href.substr(cursor, docStart - cursor);

			if (Strings::includes(addrString, std::vector<std::string>({ "?", "#" }))) {
				throw std::runtime_error("Hostname invalid");
			}

			// get http auth sorted out
			cursor = addrString.find('@');
			if (cursor != std::string::npos) {

				auto credentails = addrString.substr(0, cursor);
				const auto credSep = credentails.find(':');
				if (credSep != std::string::npos) {
					this->username = credentails.substr(0, credSep);
					this->password = credentails.substr(credSep + 1);
				}

				addrString = addrString.substr(cursor + 1);
			}

			this->host = addrString;

			//	get hostname and port
			cursor = addrString.find(':');
			if (cursor != std::string::npos) {
				this->port = addrString.substr(cursor + 1);
				this->hostname = addrString.substr(0, cursor);
			}
			else {
				this->hostname = addrString;
			}

			if (!this->hostname.size()) throw std::runtime_error("Host name undefined");
		}

		if (docStart == std::string::npos) {
			this->pathname = '/';
			return;
		}

		auto urlDocPart = href.substr(docStart);

		//	get document fragment aka hash
		cursor = urlDocPart.find_first_of('#');
		if (cursor != std::string::npos) {
			this->hash = urlDocPart.substr(cursor);
			urlDocPart = urlDocPart.substr(0, cursor);
		}

		//	now get search query
		cursor = urlDocPart.find_first_of('?');
		if (cursor != std::string::npos) {
			this->searchParams = URLSearchParams(urlDocPart.substr(cursor));
			urlDocPart = urlDocPart.substr(0, cursor);
		}

		this->pathname = urlDocPart;

	} catch(const std::exception& e) {
		throw std::runtime_error("Failed to parse URL: " + std::string(e.what()));
	} catch(...) {
		throw std::runtime_error("Failed to parse URL: Unhandled error");
	}
}

std::string URL::href() const {

	try {

		std::string temp;

		//	add protocol
		if (this->protocol.size()) {
			temp = this->protocol + "://";
		} else {
			temp = "http://";
		}

		//	add http auth
		if (this->username.size() && this->password.size()) {
			temp += this->username + ":" + this->password + "@";
		}

		//	add host name
		if (!this->hostname.size()) throw std::runtime_error("Host name undefined");
		temp += hostname;

		//	add port
		if (this->port.size()) temp += ":" + this->port;

		//	add pathname
		if (!this->pathname.size()) throw std::runtime_error("Path name undefined");
		temp += this->pathname;

		//	add search query
		if (this->searchParams.entries().size()) {
			temp += "?" + this->searchParams.stringify();
		}

		//	add fragment
		if (this->hash.size() > 1) {
			temp += this->hash;
		}

		return temp;

	} catch(const std::exception& e) {
		throw std::runtime_error("Failed to serialize URL: " + std::string(e.what()));
	} catch(...) {
		throw std::runtime_error("Failed to serialize URL: Unhandled error");
	}
}
