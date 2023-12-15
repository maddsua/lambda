#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP__

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>

namespace Lambda::HTTP {

	template<typename T, typename U>
	struct Record {
		T key;
		U value;
	};

	typedef Record<std::string, std::string> KVpair;

	/**
	 * This vector-based kv container provides base for Headers, URLSearchParams and Cookie classes
	*/
	class KVContainer {
		protected:
			std::unordered_map<std::string, std::vector<std::string>> data;

		public:
			KVContainer() {};
			KVContainer(const std::vector<KVpair>& entries);

			std::string get(const std::string& key) const;
			bool has(const std::string& key) const;
			void set(const std::string& key, const std::string value);
			void del(const std::string& key);
			std::vector<std::string> getAll(const std::string& key) const;
			void append(const std::string& key, const std::string value);
			std::vector<KVpair> entries() const;
	};

	typedef KVContainer Headers;

	class URLSearchParams : public KVContainer {
		public:
			URLSearchParams() {};
			URLSearchParams(const std::string& URLString);

			std::string stringify() const;
	};

	class URL {	
		private:
			void parse(const std::string& href);

		public:
			URL() {}
			URL(const std::string& href) {
				this->parse(href);
			}
			URL(const char* href) {
				this->parse(href);
			}

			std::string protocol;
			std::string username;
			std::string password;
			std::string host;
			std::string hostname;
			std::string port;
			std::string pathname;
			std::string hash;
			URLSearchParams searchParams;

			std::string href() const;
	};

	class Cookie : public KVContainer {
		public:
			Cookie() {};
			Cookie(const std::string& cookies);

			std::string stringify() const;
	};

	class Body {

		private:
			std::vector<uint8_t> internalContent;

		public:

			/**
			 * Creates HTTP Body object
			*/
			Body() {};

			Body(const char* content) {
				this->internalContent = std::vector<uint8_t>(content, content + strlen(content));
			};
			Body(const std::string& content) {
				this->internalContent = std::vector<uint8_t>(content.begin(), content.end());
			};
			Body(const std::vector<uint8_t>& content) {
				this->internalContent = content;
			};

			operator std::string () const {
				return this->text();
			}

			/**
			 * Returns body text reoresentation
			*/
			std::string text() const {
				return std::string(this->internalContent.begin(), this->internalContent.end());
			}
			
			/**
			 * Returns raw byte buffer
			*/
			const std::vector<uint8_t>& buffer() const {
				return this->internalContent;
			}

			/**
			 * Returns body buffer size
			*/
			size_t size() const {
				return this->internalContent.size();
			}
	};

	enum struct Methods { GET, POST, PUT, DELETE, HEAD, OPTIONS, TRACE, PATCH, CONNECT };

	class Method {
		private:
			Methods value;
			void apply(const std::string& method);

		public:
			Method();
			Method(const std::string& method);
			Method(const char* method);
			operator std::string () const;
			operator Methods () const noexcept;
	};

	struct Request {
		URL url;
		Method method;
		Headers headers;
		Body body;

		Request() {}
		Request(const URL& urlinit) : url(urlinit) {}
		Request(const URL& urlinit, const Headers& headersinit) : url(urlinit), headers(headersinit) {}
		Request(const URL& urlinit, const Body& bodyinit) : url(urlinit), body(bodyinit) {}
		Request(const URL& urlinit, const Headers& headersinit, Body& bodyinit) : url(urlinit), headers(headersinit), body(bodyinit) {}
	};

	class Status {
		private:
			int internalCode;
			std::string internalText;
		
		public:
			Status() {
				this->internalCode = 200;
				this->internalText = "OK";
			}
			Status(int code);
			Status(int code, const std::string& text) : internalCode(code), internalText(text) {}

			int code() const {
				return this->internalCode;
			}
			const std::string& text() const {
				return this->internalText;
			}
	};

	struct Response {
		Status status;
		Headers headers;
		Body body;

		Response() {}
		Response(const Status& statusinit) : status(statusinit) {}
		Response(const Body& bodyinit) : body(bodyinit) {}
		Response(const Headers& headersinit, const Body& bodyinit) : headers(headersinit), body(bodyinit) {}
		Response(const Status& statusinit, const Headers& headersinit) : status(statusinit), headers(headersinit) {}
		Response(const Status& statusinit, const Body& bodyinit) : status(statusinit), body(bodyinit) {}
		Response(const Status& statusinit, const Headers& headersinit, const Body& body) : status(statusinit), headers(headersinit), body(body) {}
	};
};

#endif
