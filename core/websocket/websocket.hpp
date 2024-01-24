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
			Message(const std::string& init) {
				this->m_binary = false;
				this->m_buffer = std::vector<uint8_t>(init.begin(), init.end());
			}
			Message(const std::vector<uint8_t>& init) : m_buffer(init) {
				this->m_binary = true;
			}

			const std::vector<uint8_t>& data() const noexcept {
				return this->m_buffer;
			}

			std::string text() const {
				return std::string(this->m_buffer.begin(), this->m_buffer.end());
			}

			bool isBinary() const noexcept {
				return this->m_binary;
			}
	};

};

#endif
