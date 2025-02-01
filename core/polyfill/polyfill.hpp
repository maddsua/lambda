#ifndef __LIB_MADDSUA_LAMBDA_CORE_STRINGS__
#define __LIB_MADDSUA_LAMBDA_CORE_STRINGS__

#include <vector>
#include <string>
#include <cstring>
#include <ctime>

namespace Lambda {

	class Date {
		private:
			time_t m_unix;
			tm m_tms;
		
		public:
			Date() noexcept;
			Date(time_t epoch) noexcept;

			std::string date() const noexcept;
			int get_date() const noexcept;
			int year() const noexcept;
			int hour() const noexcept;
			int minute() const noexcept;
			int month() const noexcept;
			int second() const noexcept;
			time_t epoch() const noexcept;
			std::string to_utc_string() const noexcept;
			std::string to_calendar_string() const noexcept;
	};

	namespace Content {
		//	Get mimetype for file extension
		std::string getExtMimetype(const std::string& mimetype);

		//	Get file extension for mimetype
		std::string getMimetypeExt(const std::string& mimetype);
	};

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
		bool includes(const std::string& str, char token);
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
		std::vector<std::string> split(const std::string& str, char separator);

		//	Combine multiple strings
		std::string join(const std::vector<std::string>& strs, const char* token);

		std::string replace(const std::string& base, const std::string& sub, const std::string& replacement);
		void replace(std::string& base, const std::string& sub, const std::string& replacement);

		void replaceAll(std::string& base, const std::string& sub, const std::string& replacement);
		std::string replaceAll(const std::string& base, const std::string& sub, const std::string& replacement);
	};
};

#endif
