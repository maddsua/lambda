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

	// Encode binary data to hex string
	std::string hexEncode(std::string data);
	std::string hexEncode(std::vector<uint8_t> data);

	/**
	 * JSON array constructor class
	*/
	class JSON_Array;
	
	/**
	 * JSON object constructor class
	*/
	class JSON_Object {
		private:
			std::map<std::string, std::string> data;

		public:

			/**
			 * Add string property
			*/
			void addString(const std::string& key, const std::string& value);

			/**
			 * Add integer (int64) property
			*/
			void addInt(const std::string& key, const int64_t value);

			/**
			 * Add integer (int32) property
			*/
			void addInt(const std::string& key, const int32_t value);

			/**
			 * Add float (float32) property
			*/
			void addFlt(const std::string& key, const float value);

			/**
			 * Add double (float64) property
			*/
			void addFlt(const std::string& key, const double value);

			/**
			 * Add bool (boolean) property
			*/
			void addBool(const std::string& key, const bool value);

			/**
			 * Put a null in there
			*/
			void addNull(const std::string& key);

			/**
			 * Remove property from the object (prop type does not matter, it's about to be nuked)
			*/
			void removeKey(const std::string& key);

			/**
			 * Add string array property
			 * 
			 * Example: ["one", "two", "tree 🌳"]
			*/
			void addStringArray(const std::string& key, const std::vector<std::string>& value);

			/**
			 * Add int array property
			 * 
			 * Example: [4, 8 ,6]
			*/
			void addIntArray(const std::string& key, const std::vector<int64_t> value);

			/**
			 * Add float array property
			 * 
			 * Example: [2.99, 10.49, 0.11]
			*/
			void addFltArray(const std::string& key, const std::vector<double> value);

			/**
			 * Add bool array property
			 * 
			 * Example: [true, true, false]
			*/
			void addBoolArray(const std::string& key, const std::vector<bool> value);

			/**
			 * Add JSON object propery
			 * 
			 * Now we're talking. Object nesting - yep it is.
			*/
			void addObject(const std::string& key, JSON_Object& object);

			/**
			 * Add JSON array propery
			 * 
			 * Also object nesting, just like with addObject
			 * 
			 * Array type does not matter
			*/
			void addObject(const std::string& key, JSON_Array& object);

			/**
			 * Get JSON from this
			*/
			std::string stringify();

			JSON_Object operator+=(const JSON_Object& b);
	};

	class JSON_Array {
		private:
			std::vector<std::string> data;
		
		public:

			/**
			 * Add string as an array item
			*/
			void pushString(const std::string& value);

			/**
			 * Add integer (int64) as an array item
			*/
			void pushInt(int64_t value);

			/**
			 * Add integer (int32) as an array item
			*/
			void pushInt(int32_t value);

			/**
			 * Add float (flt32) as an array item
			*/
			void pushFlt(float value);

			/**
			 * Add float (flt64) as an array item
			*/
			void pushFlt(double value);

			/**
			 * Add bool (boolean) as an array item
			*/
			void pushBool(bool value);

			/**
			 * Add JSON object as an array item
			*/
			void push(JSON_Object& object);

			/**
			 * Add JSON array as an array item
			*/
			void push(JSON_Array& object);

			/**
			 * Get JSON from this
			*/
			std::string stringify();

			JSON_Array operator+=(const JSON_Array& b);
	};

	/**
	 * JSON parser class
	*/
	class JSONParser {
		private:
			struct SearchMask {
				size_t from;
				size_t to;
			};

			std::string data;
			std::vector<SearchMask> mask;

			size_t findPropNext(const std::string& key);
			std::string selectArray(const std::string& key);

		public:

			/**
			 * Load JSON
			*/
			JSONParser(const std::string& jsonstring);

			/**
			 * Finds an integer prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			int64_t getIntProp(const std::string& key);

			/**
			 * Finds a float prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			long double getFltProp(const std::string& key);

			/**
			 * Finds a string prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			std::string getStringProp(const std::string& key);

			/**
			 * Finds a bool prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			bool getBoolProp(const std::string& key);

			/**
			 * Finds a JSON object prop on current object
			 * 
			 * This is the way to access nested objects and their props
			 * 
			 * Throws an error if prop is not found
			*/
			JSONParser getObjectProp(const std::string& key);

			/**
			 * Finds a integer array prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			std::vector<uint64_t> getIntArrayProp(const std::string& key);

			/**
			 * Finds a float array prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			std::vector<long double> getFltArrayProp(const std::string& key);

			/**
			 * Finds a string array prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			std::vector<std::string> getStringArrayProp(const std::string& key);

			/**
			 * Finds a JSON object array prop on current object
			 * 
			 * Throws an error if prop is not found
			*/
			std::vector<JSONParser> getObjectArrayProp(const std::string& key);
	};
};

#endif
