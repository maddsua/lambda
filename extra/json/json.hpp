#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_JSON__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_JSON__

#include <string>
#include <vector>
#include <unordered_map>

namespace Lambda::JSON {

	class Property;
	typedef std::unordered_map<std::string, Property> Map;
	typedef std::vector<Property> Array;

	enum struct JSTypes {
		Undefined = -1,
		Null = 0,
		String = 1,
		Boolean = 2,
		Integer = 3,
		Float = 4,
		Array = 5,
		Map = 6
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
