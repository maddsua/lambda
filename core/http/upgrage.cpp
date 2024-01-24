#include "./http.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;
using namespace Lambda::Crypto;
using namespace Lambda::Encoding;
using namespace Lambda::Strings;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

Response Request::upgrageToWebsocket() const noexcept {

	auto headerUpgrade = toLowerCase(this->headers.get("Upgrade"));
	auto headerWsKey = this->headers.get("Sec-WebSocket-Key");

	if (headerUpgrade != "websocket" || !headerWsKey.size()) {
		return Response(400, "Websocket initialization aborted: no valid handshake headers present");
	}

	auto combinedKey = headerWsKey + wsMagicString;

	auto keyHash = Crypto::SHA1().update(std::vector<uint8_t>(combinedKey.begin(), combinedKey.end())).digest();

	return Response(101, HTTP::Headers({
		{ "Upgrade", "websocket" },
		{ "Connection", "Upgrade" },
		{ "Sec-WebSocket-Accept", Encoding::toBase64(keyHash) }
	}));
}
