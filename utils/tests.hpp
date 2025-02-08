#ifndef __TEST_LIB_MADDSUA_LAMBDA_INTERNAL_TESTUTILS__
#define __TEST_LIB_MADDSUA_LAMBDA_INTERNAL_TESTUTILS__

	#include <functional>
	#include <cstdio>
	#include <stdexcept>

	typedef std::vector<std::pair<const char*, std::function<void()>>> TestQueue;

	inline int exec_test(const char* id, std::function<void()> callback) {
		
		try {
			callback();
			printf("--> Test '%s': OK\n", id);
		} catch(const std::exception& e) {
			fprintf(stderr, " --X Test '%s': FAILED: \n\t%s\n", id, e.what());
			return 1;
		}

		return 0;
	}

	inline int exec_queue(const TestQueue& queue) {
		
		for (auto& test : queue) {
			auto status = exec_test(test.first, test.second);
			if (status) {
				return status;
			}
		}

		printf("--> Tests completed\n");
			
		return 0;
	}

	inline void string_assert(const char* field, std::string value, std::string expected) {
		
		if (value == expected) {
			return;
		}

		throw std::logic_error("Field mismatch: [" + std::string(field) + "]\n\t\tExpected: '" + expected + "'\n\t\tReturned: '" + value + "'");
	}

#endif
