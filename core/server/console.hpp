#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_CONSOLE__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_CONSOLE__

#include <string>
#include <stdint.h>

namespace Lambda {

	class Console {
		private:
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

			std::string m_id;
			bool m_timestamps;
			std::string serializeEntries(const std::initializer_list<Console::Entry>& list) const;

		public:
			Console(const std::string& setid, bool useTimestamps);

			void log(std::initializer_list<Console::Entry> list) const;
			void error(std::initializer_list<Console::Entry> list) const;
			void warn(std::initializer_list<Console::Entry> list) const;
	};
};

#endif
