#ifndef _OCTOPUSS_CORE_
#define _OCTOPUSS_CORE_

#include <vector>
#include <string>
#include <cstring>

	/**
	 * Extends standard std::string methods
	*/
	namespace Strings {

		//	Set all string characters to lower case
		//	AaA -> aaa
		void toLowerCase(std::string& str);
		std::string toLowerCase(const std::string& str);

		//	Set all string characters to upper case
		//	aAa -> AAA
		void toUpperCase(std::string& str);
		std::string toUpperCase(const std::string& str);

		//	Capitalize first character in each word and leave others in lower case
		//	aa-bB -> Aa-Bb
		void toTittleCase(std::string& str);
		std::string toTittleCase(const std::string& str);

		//	Checks whether string includes substring
		bool includes(const std::string& str, const std::string& substr);
		bool includes(const std::string& str, const std::vector <std::string>& substrs);

		//	Checks whether string ends with substring
		bool endsWith(const std::string& str, const std::string& substr);

		//	Checks whether string starts with substring
		bool startsWith(const std::string& str, const std::string& substr);

		//	Remove trailing and preceiding whitespace characters (\\r\\n\\t\\s)
		void trim(std::string& str);
		std::string trim(const std::string& str);

		//	Split string into array of substrings separated by token
		std::vector<std::string> split(const std::string& str, const std::string& token);
	};

	namespace HTTP {

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
				std::vector<KVpair> internalContent;
				void delNormalized(const std::string& keyNormalized);

			public:
				KVContainer() {};
				KVContainer(const std::vector<KVpair>& entries);

				std::string get(const std::string& key) const;
				bool has(const std::string& key) const;
				void set(const std::string& key, const std::string value);
				void del(const std::string& key);
				const std::vector<KVpair>& entries() const;
		};

		class Headers : public KVContainer {
			public:
				Headers() {};

				std::vector<std::string> getAll(const std::string& key) const;
				void append(const std::string& key, const std::string value);
		};

		class URLSearchParams : public Headers {
			public:
				URLSearchParams() {};
				URLSearchParams(const std::string& URLString);

				std::string stringify() const;
		};

		class URL {	
			private:
				void parse(const std::string& href);

			public:
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
				 * Returns raw bite buffer
				*/
				const std::vector<uint8_t>& buffer() const {
					return this->internalContent;
				}
		};

		class Method {
			private:
				std::string value;
				void apply(const std::string& method);
			
			public:
				Method() {
					this->value = "GET";
				};
				Method(const std::string& method) {
					this->apply(method);
				}
				Method(const char* method) {
					this->apply(method);
				}

				operator std::string () const {
					return this->value;
				}
		};

		class Request {
			public:
				Request(const URL& url) : url(url) {}
				Request(const URL& url, const Headers& headers) : url(url), headers(headers) {}
				Request(const URL& url, const Body& body) : url(url), body(body) {}
				Request(const URL& url, const Headers& headers, Body& body) : url(url), headers(headers), body(body) {}

				URL url;
				Method method;
				Headers headers;
				Body body;
		};

		class Status {
			private:
				std::string internalText;
				int internalCode;
			
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

		class Response {
			Response(const Status& status) : status(status) {}
			Response(const Status& status, const Headers& headers) : status(status), headers(headers) {}
			Response(const Status& status, const Body& body) : status(status), body(body) {}
			Response(const Status& status, const Headers& headers, Body& body) : status(status), headers(headers), body(body) {}
			
			Status status;
			Headers headers;
			Body body;
		};
	}

	class Date {
		private:
			time_t internalTime;
		
		public:
			Date() {}
			Date(time_t epoch) : internalTime(epoch) {}

			std::string getDate();
			uint32_t getDay();
			uint32_t getYear();
			uint32_t getHours();
			uint32_t getMinutes();
			uint32_t getMonth();
			uint32_t getSeconds();
			time_t getTime() {
				return this->internalTime;
			}
			std::string toUTCString();
			std::string toHumanReadableTime();
	};

#endif
