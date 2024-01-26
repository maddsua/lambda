#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_CONSOLE__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_CONSOLE__

#include <string>
#include <stdint.h>

namespace Lambda {

	class LogItem {
		private:
			std::string value;

		public:
			LogItem(const std::string& thing);
			LogItem(const char* thing);
			LogItem(bool thing);
			LogItem(char thing);
			LogItem(unsigned char thing);
			LogItem(short thing);
			LogItem(unsigned short thing);
			LogItem(int thing);
			LogItem(unsigned int thing);
			LogItem(float thing);
			LogItem(double thing);
			LogItem(long thing);
			LogItem(unsigned long thing);
			LogItem(long long thing);
			LogItem(unsigned long long thing);
			LogItem(long double thing);

			const std::string& unwrap();
	};

	class Console {
		private:
			std::string m_id;
			bool m_timestamps;
			std::string serializeEntries(const std::initializer_list<LogItem>& list) const;

		public:
			Console(const std::string& setid, bool useTimestamps);

			void log(std::initializer_list<LogItem> list) const;
			void error(std::initializer_list<LogItem> list) const;
			void warn(std::initializer_list<LogItem> list) const;
	};
};

#endif
