#ifndef __LAMBDA_HTTP_UTILS__
#define __LAMBDA_HTTP_UTILS__

#include <string>
#include <map>
#include <vector>
#include <functional>

namespace Lambda::HTTP {

	typedef std::vector<std::string> MultiValue;
	typedef std::vector<std::pair<std::string, std::string>> Entries;

	class Values {
		protected:
			std::map<std::string, MultiValue> m_entries;
			std::string m_format_key(const std::string& key) const noexcept;

		public:
			Values() = default;
			Values(const Values& other);
			Values(Values&& other);

			Values& operator=(const Values& other) noexcept;
			Values& operator=(Values&& other) noexcept;

			bool has(const std::string& key) const noexcept;
			std::string get(const std::string& key) const noexcept;
			MultiValue get_all(const std::string& key) const noexcept;
			void set(const std::string& key, const std::string& value) noexcept;
			void append(const std::string& key, const std::string& value) noexcept;
			void del(const std::string& key) noexcept;
			Entries entries() const noexcept;
			size_t size() const noexcept;
	};

	typedef std::vector<uint8_t> Buffer;
	typedef std::function<size_t(int status, const Values& header)> HeaderWriterCallback;
	typedef std::function<size_t(const Buffer& data)> WriterCallback;

	const size_t LengthUnknown = -1;
	typedef std::function<Buffer(size_t size)> ReaderCallback;

};

#endif
