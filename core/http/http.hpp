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
			KVContainer(KVContainer&& other);
			KVContainer(const std::initializer_list<KVpair>& init);

			KVContainer& operator=(const KVContainer& other) noexcept;
			KVContainer& operator=(KVContainer&& other) noexcept;

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

	class BodyBuffer {

		private:
			std::vector<uint8_t> m_data;

		public:

			/**
			 * Creates HTTP Body object
			*/
			BodyBuffer() {}
			BodyBuffer(const BodyBuffer& other);
			BodyBuffer(const char* content);
			BodyBuffer(const std::string& content);
			BodyBuffer(const std::vector<uint8_t>& content);

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

	class Status {
		private:
			uint32_t m_code;
			std::string m_text;
		
		public:
			Status();
			Status(uint32_t code);
			Status(uint32_t code, const std::string& text);

			uint32_t code() const noexcept;
			const std::string& text() const noexcept;
	};

	struct Response {
		Status status;
		Headers headers;
		Cookies setCookies;
		BodyBuffer body;

		Response() {}
		Response(
			const Status& statusinit
		) : status(statusinit) {}
		Response(
			const Status& statusinit,
			const Headers& headersinit
		) : status(statusinit), headers(headersinit) {}
		Response(
			const Status& statusinit,
			const BodyBuffer& body
		) : status(statusinit), body(body) {}
		Response(
			const BodyBuffer& bodyinit
		) : body(bodyinit) {}
		Response(
			const Status& statusinit,
			const Headers& headersinit,
			const BodyBuffer& body
		) : status(statusinit), headers(headersinit), body(body) {}
	};

	struct Request {
		URL url;
		Method method;
		Headers headers;
		Cookies cookies;
		BodyBuffer body;

		Request() {}
		Request(
			const URL& urlinit
		) : url(urlinit) {}
		Request(
			const URL& urlinit,
			const Method& methodInit
		) : url(urlinit), method(methodInit) {}
		Request(
			const URL& urlinit,
			const Headers& headersinit
		) : url(urlinit), headers(headersinit) {}
		Request(
			const URL& urlinit,
			const Method& methodInit,
			const BodyBuffer& bodyinit
		) : url(urlinit), method(methodInit), body(bodyinit) {}
		Request(
			const URL& urlinit,
			const Method& methodInit,
			const Headers& headersinit,
			BodyBuffer& bodyinit
		) : url(urlinit), method(methodInit), headers(headersinit), body(bodyinit) {}
	};
};

#endif
