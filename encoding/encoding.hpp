#ifndef __LAMBDA_ENCODING__
#define __LAMBDA_ENCODING__

#include <string>
#include <vector>
#include <map>

namespace Lambda::Encoding {

	//	URL-encode string
	std::string encodeURIComponent(const std::string& str);

	//	Decode base64 string
	std::string b64Decode(const std::string& data);
	//	Encode base64 string
	std::string b64Encode(const std::string& data);
	//	Validate base64 string
	bool b64Validate(const std::string& encoded);

	std::string hexEncode(std::string data);
	std::string hexEncode(std::vector<uint8_t> data);

	class JSON_Array;
	
	class JSON_Object {
		private:
			std::map<std::string, std::string> data;

		public:
			void addString(const std::string& key, const std::string& value);
			void addInt(const std::string& key, const int64_t value);
			void addInt(const std::string& key, const int32_t value);
			void addFlt(const std::string& key, const float value);
			void addFlt(const std::string& key, const double value);
			void addBool(const std::string& key, const bool value);
			void addNull(const std::string& key);
			void removeKey(const std::string& key);

			void addStringArray(const std::string& key, const std::vector<std::string>& value);
			void addIntArray(const std::string& key, const std::vector<int64_t> value);
			void addFltArray(const std::string& key, const std::vector<double> value);
			void addBoolArray(const std::string& key, const std::vector<bool> value);

			void addObject(const std::string& key, JSON_Object& object);
			void addObject(const std::string& key, JSON_Array& object);

			std::string stringify();

			JSON_Object operator+=(const JSON_Object& b);
	};

	class JSON_Array {
		private:
			std::vector<std::string> data;
		
		public:
			void pushString(const std::string& value);
			void pushInt(int64_t value);
			void pushInt(int32_t value);
			void pushFlt(float value);
			void pushFlt(double value);
			void pushBool(bool value);
			void push(JSON_Object& object);
			void push(JSON_Array& object);

			//void remove(size_t index);
			//void remove(size_t index, size_t elements);

			std::string stringify();

			JSON_Array operator+=(const JSON_Array& b);
	};

	class JSONParser {
		private:
			struct SearchMask {
				size_t from;
				size_t to;
			};

			std::string data;
			std::vector<SearchMask> mask;

			std::string selectArray(const std::string& key);

		public:
			JSONParser(const std::string& jsonstring);
			size_t findPropNext(const std::string& key);

			int64_t getIntProp(const std::string& key);
			long double getFltProp(const std::string& key);
			std::string getStringProp(const std::string& key);
			JSONParser getObjectProp(const std::string& key);
			bool getBoolProp(const std::string& key);

			std::vector<uint64_t> getIntArrayProp(const std::string& key);
			std::vector<long double> getFltArrayProp(const std::string& key);
			std::vector<std::string> getStringArrayProp(const std::string& key);
			std::vector<JSONParser> getObjectArrayProp(const std::string& key);
	};
};

#endif
