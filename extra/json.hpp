#ifndef _LAMBDA_EXTRA_JSON_
#define _LAMBDA_EXTRA_JSON_

#include <string>
#include <vector>
#include <unordered_map>

namespace JSON {

	class Property;
	typedef std::unordered_map<std::string, Property> Map;
	typedef std::vector<Property> Array;

	enum JSTypes {
		Type_Undefined = -1,
		Type_Null = 0,
		Type_String = 1,
		Type_Boolean = 2,
		Type_Integer = 3,
		Type_Float = 4,
		Type_Array = 5,
		Type_Map = 6
	};

	class Property {
		private:
			JSTypes internalType;

			long double* internalDataFloat = nullptr;
			int32_t* internalDataInt = nullptr;
			bool* internalDataBool = nullptr;
			std::string* internalDataStr = nullptr;
			Array* internalDataArray = nullptr;
			Map* internalDataMap = nullptr;

			void clone(const Property& other);

		public:
			Property();
			Property(std::nullptr_t value);

			Property(long double value);
			Property(double value);
			Property(float value);

			Property(int32_t value);
			Property(uint32_t value);
			Property(int16_t value);
			Property(uint16_t value);

			Property(bool value);

			Property(const std::string& value);
			Property(const char* value);

			Property(const Array& value);
			Property(const Map& value);

			Property(const Property& other);
			Property(Property&& other);
			Property operator=(const Property& other);

			JSTypes type() const;
			bool isUndefined() const;
			bool isNull() const;
			bool isNil() const;

			long double asFloat() const;
			int32_t asInt() const;
			bool asBool() const;
			const std::string& asString() const;
			const Array& asArray() const;
			const Map& asMap() const;

			~Property();
	};

	std::string stringify(const Property& data);
	Property parse(const std::string& text);
};

#endif
