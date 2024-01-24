#ifndef __LIB_MADDSUA_LAMBDA_CORE_ENDPOINTS_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_ENDPOINTS_HANDLERS__

#include "../http/http.hpp"
#include "../network/network.hpp"

#include <functional>

namespace Lambda::Endpoints {

	class Console {
		public:
			Console(const std::string& setid) : context_id(setid) {}

			struct Entry {
				Entry(const std::string& thing);
				Entry(const char* thing);
				Entry(bool thing);
				Entry(char thing);
				Entry(unsigned char thing);
				Entry(short thing);
				Entry(unsigned short thing);
				Entry(int thing);
				Entry(unsigned int thing);
				Entry(float thing);
				Entry(double thing);
				Entry(long thing);
				Entry(unsigned long thing);
				Entry(long long thing);
				Entry(unsigned long long thing);
				Entry(long double thing);

				std::string value;
			};

			void log(std::initializer_list<Entry> list) const;
			void error(std::initializer_list<Entry> list) const;
			void warn(std::initializer_list<Entry> list) const;

		private:
			std::string context_id;
			std::string serializeEntries(const std::initializer_list<Entry>& list) const;
	};

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HandlerFunction;

};

#endif
