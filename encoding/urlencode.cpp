#include "./encoding.hpp"
#include <map>

using namespace Lambda;

typedef std::map<char, std::string> URLEncodeMap;
const auto URLEncodeTable = URLEncodeMap({
	{'\"', "%22"},
	{'\'', "%27"},
	{'\\', "%5C"},
	{'/', "%2F"},
	{'>', "%3E"},
	{'<', "%3C"},
	{' ', "%20"},
	{'%', "%25"},
	{'{', "%7B"},
	{'}', "%7D"},
	{'|', "%7C"},
	{'^', "%5E"},
	{'`', "%60"},
	{':', "%3A"},
	{'\?', "%3F"},
	{'#', "%23"},
	{'[', "%5B"},
	{']', "%5D"},
	{'@', "%40"},
	{'!', "%21"},
	{'$', "%24"},
	{'&', "%26"},
	{'(', "%28"},
	{')', "%29"},
	{'*', "%2A"},
	{'+', "%2B"},
	{',', "%2C"},
	{';', "%3B"},
	{'=', "%3D"}
});

std::string Encoding::encodeURIComponent(const std::string& str) {
	auto result = std::string();
	for (auto c : str) {
		if (URLEncodeTable.find(c) != URLEncodeTable.end()) {
			result.append(URLEncodeTable.at(c));
			continue;
		}
		result.push_back(c);
	}
	return std::move(result);
}
