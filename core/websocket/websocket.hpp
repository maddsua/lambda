#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET__

#include <string>
#include <vector>
#include <cstdint>

namespace Lambda::Websocket {

	class Message {
		private:
			std::vector<uint8_t> m_buffer;
			bool m_binary;

		public:
			Message(const std::string& init);
			Message(const std::vector<uint8_t>& init);
			const std::vector<uint8_t>& data() const noexcept;
			std::string text() const;
			bool isBinary() const noexcept;
	};

};

#endif
