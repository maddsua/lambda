#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP__

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <map>

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
			KVContainer() = default;
			KVContainer(const KVContainer& other);
			KVContainer(KVContainer&& other);
			KVContainer(const std::initializer_list<KVpair>& init);

			KVContainer& operator=(const KVContainer& other) noexcept;
			KVContainer& operator=(KVContainer&& other) noexcept;

			std::string get(const std::string& key) const;
			bool has(const std::string& key) const;
			void set(const std::string& key, const std::string& value);
			void del(const std::string& key);
			std::vector<std::string> getAll(const std::string& key) const;
			void append(const std::string& key, const std::string& value);
			std::vector<KVpair> entries() const;
			size_t size() const noexcept;
	};

	typedef KVContainer Headers;

	class URLSearchParams : public KVContainer {
		public:
			URLSearchParams() = default;
			URLSearchParams(const std::string& URLString);

			std::string stringify() const;
	};

	struct URL {	
		private:
			void parse(const std::string& href);

		public:
			URL() = default;
			URL(const std::string& href);
			URL(const char* href);

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

	class Cookies {
		protected:

			struct SetParam {
				std::string key;
				std::string value;

				SetParam(const std::string& init);
				SetParam(const char* init);
				SetParam(std::initializer_list<std::string> init);
			};

			struct SetItem {
				std::string value;
				std::initializer_list<SetParam> props;
			};

			std::unordered_map<std::string, std::string> m_data;
			std::map<std::string, SetItem> m_set_queue;

		public:
			Cookies() = default;
			Cookies(const std::string& cookiestring);
			Cookies(const Cookies& other);
			Cookies(Cookies&& other);

			Cookies& operator=(const Cookies& other) noexcept;
			Cookies& operator=(Cookies&& other) noexcept;

			std::string get(const std::string& key) const;
			bool has(const std::string& key) const;
			void set(const std::string& key, const std::string& value);
			void set(const std::string& key, const std::string& value, const std::initializer_list<SetParam>& props);
			void del(const std::string& key);
			std::vector<KVpair> entries() const;
			size_t size() const noexcept;
			std::vector<std::string> serialize() const;
	};

	class BodyBuffer {
		private:
			std::vector<uint8_t> m_data;

		public:

			/**
			 * Creates HTTP Body object
			*/
			BodyBuffer() = default;
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

			enum struct Type {
				Unknown, Info, Success, Redirect, ClientError, ServerError
			};

			Status();
			Status(uint32_t code);
			Status(uint32_t code, const std::string& text);

			uint32_t code() const noexcept;
			const std::string& text() const noexcept;

			Type type() const noexcept;
	};

	struct Response {
		Status status;
		Headers headers;
		BodyBuffer body;

		Response() = default;
		Response(const Status& statusinit);
		Response(const Status& statusinit, const Headers& headersinit);
		Response(const Status& statusinit, const BodyBuffer& body);
		Response(const Status& statusinit, const Headers& headersinit, const BodyBuffer& body);
		Response(const BodyBuffer& bodyinit);

		void setCookies(const Cookies& cookies);
		void setCookies(const std::initializer_list<KVpair>& cookies);
	};

	struct Request {
		URL url;
		Method method;
		Headers headers;
		BodyBuffer body;

		Request() = default;
		Request(const std::string& urlinit);
		Request(const std::string& urlinit, const Method& methodInit);
		Request(const std::string& urlinit, const Headers& headersinit);
		Request(const std::string& urlinit, const Method& methodInit, const BodyBuffer& bodyinit);
		Request(const std::string& urlinit, const Method& methodInit, const Headers& headersinit, BodyBuffer& bodyinit);

		Cookies getCookies() const;
	};
};

#endif
