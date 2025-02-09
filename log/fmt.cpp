#include "./log.hpp"

#include <iostream>
#include <mutex>

using namespace Lambda;
using namespace Lambda::Log;

std::mutex sync_stdout;
std::mutex sync_stderr;

void Log::log(const std::string& message) {

	std::lock_guard ctx_lock (sync_stdout);

	std::cout.write(message.c_str(), message.size());
	if (!message.ends_with('\n')) {
		std::cout.write("\n", 1);
	}

	std::cout.flush();
}

void Log::err(const std::string& message) {

	std::lock_guard ctx_lock (sync_stderr);

	std::cerr.write(message.c_str(), message.size());
	if (!message.ends_with('\n')) {
		std::cerr.write("\n", 1);
	}

	std::cerr.flush();
}

std::string format_message(const std::string& format, std::initializer_list<Overload> params) {

	std::string buff;

	auto acc_size = format.size();
	for (const auto& item : params) {
		acc_size += item.value.size();
	}

	buff.reserve(acc_size);

	size_t next_param = 0;
	auto token_start = 0;
	auto has_next_end = format.size() - 1;

	for (size_t idx = 0; idx < format.size(); idx++) {
		
		if (format[idx] == '{' && idx < has_next_end && format[idx + 1] == '}') {

			if (next_param >= params.size()) {
				continue;
			}

			buff.insert(buff.end(), format.begin() + token_start, format.begin() + idx);

			auto value = (params.begin() + next_param)->value;
			buff.insert(buff.end(), value.begin(), value.end());

			idx++;
			next_param++;
			token_start = idx + 1;
			continue;
		}
	}

	buff.insert(buff.end(), format.begin() + token_start, format.end());
	
	return buff;
}

void Log::log(const std::string& format, std::initializer_list<Overload> params) {
	Log::log(format_message(format, params));
}

void Log::err(const std::string& format, std::initializer_list<Overload> params) {
	Log::err(format_message(format, params));
}
