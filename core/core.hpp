#ifndef _OCTOPUSS_CORE_
#define _OCTOPUSS_CORE_

#include <vector>
#include <string>

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
			public:
				URL() {};
				URL(const std::string& href);

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

				Body(const std::string& content) {
					this->internalContent = std::vector<uint8_t>(content.begin(), content.end());
				};

				Body(const std::vector<uint8_t>& content) {
					this->internalContent = content;
				};

				Body& operator = (const std::vector<uint8_t>& right) {
					this->internalContent = right;
					return *this;
				}

				Body& operator = (const std::string& right) {
					this->internalContent = std::vector<uint8_t>(right.begin(), right.end());
					return *this;
				}

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
	}

#endif
