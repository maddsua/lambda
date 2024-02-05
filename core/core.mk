
include core/compression/compression.mk
include core/crypto/crypto.mk
include core/encoding/encoding.mk
include core/http/http.mk
include core/json/json.mk
include core/html/html.mk
include core/network/network.mk
include core/polyfill/polyfill.mk
include core/server/server.mk
include core/utils/utils.mk
include core/websocket/websocket.mk

LIB_CORE				= core/core.a
LIB_CORE_DEPS			= $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_HTTP_DEPS) $(LIB_CORE_ENCODING_DEPS) $(LIB_CORE_NETWORK_DEPS) $(LIB_CORE_COMPRESS_DEPS) $(LIB_CORE_SERVER_DEPS) $(LIB_CORE_CRYPTO_DEPS) $(LIB_CORE_HTML_DEPS) $(LIB_CORE_JSON_DEPS) $(LIB_CORE_WEBSOCKET_DEPS) $(LIB_CORE_UTILS_DEPS)

# target object
lambda.core: $(LIB_CORE)

$(LIB_CORE): $(LIB_CORE_DEPS)
	ar rvs $(LIB_CORE) $(LIB_CORE_DEPS)
