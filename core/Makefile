
include core/compression/Makefile
include core/crypto/Makefile
include core/encoding/Makefile
include core/error/Makefile
include core/html/Makefile
include core/http/Makefile
include core/json/Makefile
include core/network/Makefile
include core/polyfill/Makefile
include core/server/Makefile
include core/utils/Makefile
include core/websocket/Makefile

LIB_CORE				= core/core.a
LIB_CORE_DEPS			= $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_HTTP_DEPS) $(LIB_CORE_ENCODING_DEPS) $(LIB_CORE_NETWORK_DEPS) $(LIB_CORE_COMPRESS_DEPS) $(LIB_CORE_SERVER_DEPS) $(LIB_CORE_CRYPTO_DEPS) $(LIB_CORE_HTML_DEPS) $(LIB_CORE_JSON_DEPS) $(LIB_CORE_ERROR_DEPS) $(LIB_CORE_WEBSOCKET_DEPS) $(LIB_CORE_UTILS_DEPS)

# target object
lambda.core: $(LIB_CORE)

$(LIB_CORE): $(LIB_CORE_DEPS)
	ar rvs $(LIB_CORE) $(LIB_CORE_DEPS)
