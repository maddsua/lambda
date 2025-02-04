#ifndef  __LAMBDA_JSON__
#define  __LAMBDA_JSON__

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cstdint>
#include <stdexcept>

namespace Lambda::JSON {

	enum struct Type {
		Undefined = -1,
		Null = 0,
		String = 1,
		Boolean = 2,
		Integer = 3,
		Float = 4,
		Array = 5,
		Object = 6
	};

	class Property;
	typedef std::map<std::string, Property> Object;
	typedef std::vector<Property> Array;

	std::string stringify(const Property& value);
	std::string stringify(const std::string& value);
	std::string stringify(const char* value);
	std::string stringify(uint16_t value);
	std::string stringify(int16_t value);
	std::string stringify(uint32_t value);
	std::string stringify(int32_t value);
	std::string stringify(uint64_t value);
	std::string stringify(int64_t value);
	std::string stringify(float value);
	std::string stringify(double value);
	std::string stringify(long double value);
	std::string stringify(Object value);
	std::string stringify(Array value);
	
	Property parse(const std::string& data);

	class Property {
		private:
			Type m_type;
			std::variant<long double, int64_t, bool, std::string, Array, Object> m_data;

		public:

			Property() {
				this->m_type = Type::Undefined;
			}

			Property(std::nullptr_t _nullval) {
				this->m_type = Type::Null;
			}

			Property(long double value) {
				this->m_type = Type::Float;
				this->m_data = value;
			}

			Property(double value) {
				this->m_type = Type::Float;
				this->m_data = static_cast<long double>(value);
			}

			Property(float value) {
				this->m_type = Type::Float;
				this->m_data = static_cast<long double>(value);
			}

			Property(int64_t value) {
				this->m_type = Type::Integer;
				this->m_data = value;
			}

			Property(uint64_t value) {
				this->m_type = Type::Integer;
				this->m_data = static_cast<int64_t>(value);
			}

			Property(int32_t value) {
				this->m_type = Type::Integer;
				this->m_data = static_cast<int64_t>(value);
			}

			Property(uint32_t value) {
				this->m_type = Type::Integer;
				this->m_data = static_cast<int64_t>(value);
			}

			Property(int16_t value) {
				this->m_type = Type::Integer;
				this->m_data = static_cast<int64_t>(value);
			}

			Property(uint16_t value) {
				this->m_type = Type::Integer;
				this->m_data = static_cast<int64_t>(value);
			}

			Property(bool value) {
				this->m_type = Type::Boolean;
				this->m_data = value;
			}

			Property(const std::string& value) {
				this->m_type = Type::String;
				this->m_data = value;
			}

			Property(const char* value) {
				this->m_type = Type::String;
				this->m_data = std::string(value);
			}

			Property(const Array& value) {
				this->m_type = Type::Array;
				this->m_data = value;
			}

			Property(const Object& value) {
				this->m_type = Type::Object;
				this->m_data = value;
			}

			Property(const Property& other) {
				this->m_type = other.m_type;
				this->m_data = other.m_data;
			}

			Property(Property&& other) {
				this->m_type = other.m_type;
				this->m_data = other.m_data;
			}

			Property operator=(const Property& other) {
				this->m_type = other.m_type;
				this->m_data = other.m_data;
				return *this;
			}

			Type type() const noexcept {
				return this->m_type;
			}

			long double as_float() const {

				if (this->m_type != Type::Float) {
					throw std::runtime_error("TypeError: value is not a float");
				}

				return std::get<long double>(this->m_data);
			}

			int64_t as_int() const {

				if (this->m_type != Type::Integer) {
					throw std::runtime_error("TypeError: value is not an integer");
				}

				return std::get<int64_t>(this->m_data);
			}

			bool as_bool() const {
				
				if (this->m_type != Type::Boolean) {
					throw std::runtime_error("TypeError: value is not a boolean");
				}

				return std::get<bool>(this->m_data);
			}

			const std::string& as_string() const {
				
				if (this->m_type != Type::String) {
					throw std::runtime_error("TypeError: value is not a string");
				}

				return std::get<std::string>(this->m_data);
			}

			const Array& as_array() const {
				
				if (this->m_type != Type::Array) {
					throw std::runtime_error("TypeError: value is not an array");
				}

				return std::get<Array>(this->m_data);
			}

			const std::map<std::string, Property>& as_map() const {
				
				if (this->m_type != Type::Object) {
					throw std::runtime_error("TypeError: value is not a map");
				}

				return std::get<Object>(this->m_data);
			}
	};
};

#endif
