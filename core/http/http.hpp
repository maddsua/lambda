#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP__

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>

namespace Lambda::HTTP {

	typedef std::pair<std::string, std::string> KVpair;
	typedef std::pair<const std::string, std::vector<std::string>> MultiValueKVpair;

	/**
	 * This vector-based kv container provides base for Headers, URLSearchParams and Cookie classes
	*/
	class KVContainer {
		protected:
			std::unordered_map<std::string, std::vector<std::string>> m_data;

		public:
			KVContainer() {};
			KVContainer(const KVContainer& other);
			KVContainer(const std::initializer_list<KVpair>& init);

			std::string get(const std::string& key) const;
			bool has(const std::string& key) const;
			void set(const std::string& key, const std::string value);
			void del(const std::string& key);
			std::vector<std::string> getAll(const std::string& key) const;
			void append(const std::string& key, const std::string value);
			std::vector<KVpair> entries() const;
			size_t size() const noexcept;
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

	class Cookies : public KVContainer {
		public:
			Cookies() {};
			Cookies(const std::string& cookies);

			std::string stringify() const;
	};

	class Body {

		private:
			std::vector<uint8_t> m_data;

		public:

			/**
			 * Creates HTTP Body object
			*/
			Body() {}
			Body(const Body& other);
			Body(const char* content);
			Body(const std::string& content);
			Body(const std::vector<uint8_t>& content);

			operator std::string () const;

			/**
			 * Returns body text reoresentation
			*/
			std::string text() const;
			
			/**
			 * Returns raw byte buffer
			*/
			const std::vector<uint8_t>& buffer() const;

			/**
			 * Returns body buffer size
			*/
			size_t size() const;
	};

	enum struct Methods {
		GET, POST, PUT, DEL, HEAD, OPTIONS, TRACE, PATCH, CONNECT
	};

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
		Cookies cookies;
		Body body;

		Request() {}
		Request(const URL& urlinit) : url(urlinit) {}
		Request(const URL& urlinit, const Headers& headersinit) : url(urlinit), headers(headersinit) {}
		Request(const URL& urlinit, const Body& bodyinit) : url(urlinit), body(bodyinit) {}
		Request(const URL& urlinit, const Headers& headersinit, Body& bodyinit) : url(urlinit), headers(headersinit), body(bodyinit) {}

		Response upgrageToWebsocket() const noexcept;
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
			Status(int code, const std::string& text);

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
		Cookies setCookies;
		Body body;

		Response() {}
		Response(const Status& statusinit);
		Response(const Body& bodyinit);
		Response(const Headers& headersinit);
		Response(const Headers& headersinit, const Body& bodyinit);
		Response(const Status& statusinit, const Headers& headersinit);
		Response(const Status& statusinit, const Body& bodyinit);
		Response(const Status& statusinit, const Headers& headersinit, const Body& body);
		Response(const Body& body, const Headers& headersinit, const Status& statusinit);
	};
};

#endif
