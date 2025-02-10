LIBNAME						=	lambda
PROD_FLAGS					=	$(if $(filter $(target),prod),-s,-g -fsanitize=address -static-libasan)
CFLAGS						=	-Wall -Werror -std=c++23 -fPIC $(PROD_FLAGS)
LAMBDA_LIBSTATIC			=	.artifacts/dist/$(LIBNAME).a
LAMBDA_LIBSHARED			=	.artifacts/dist/$(LIBNAME).so

.PHONY: all all-before all-after action-custom
all: all-before libshared libstatic all-after

clean: action-custom
	rm -fr .artifacts/*
	mkdir -p .artifacts/dist


#############################################
#			PKG: Base64 encode				#
#############################################

LIB_OBJS_BASE64_OBJS = .artifacts/base64.o
.lib-objs-base64: $(LIB_OBJS_BASE64_OBJS)
test-base64: .artifacts/test-base64
	.artifacts/test-base64
.artifacts/test-base64: .artifacts/test-base64.o $(LIB_OBJS_BASE64_OBJS)
	g++ $(CFLAGS) .artifacts/test-base64.o $(LIB_OBJS_BASE64_OBJS) -o .artifacts/test-base64
.artifacts/test-base64.o: base64/test_base64.cpp
	g++ -c $(CFLAGS) base64/test_base64.cpp -o .artifacts/test-base64.o
.artifacts/base64.o: base64/base64.cpp
	g++ -c $(CFLAGS) base64/base64.cpp -o .artifacts/base64.o


#############################################
#				PKG: sha1 hash				#
#############################################

LIB_OBJS_HASH_OBJS = .artifacts/hash_sha1.o
.lib-objs-hash: $(LIB_OBJS_HASH_OBJS)
test-hash: .artifacts/test-hash
	.artifacts/test-hash
.artifacts/test-hash: .artifacts/test-hash.o $(LIB_OBJS_HASH_OBJS) $(LIB_OBJS_BASE64_OBJS)
	g++ $(CFLAGS) .artifacts/test-hash.o $(LIB_OBJS_HASH_OBJS) $(LIB_OBJS_BASE64_OBJS) -o .artifacts/test-hash
.artifacts/test-hash.o: hash/test_hash.cpp
	g++ -c $(CFLAGS) hash/test_hash.cpp -o .artifacts/test-hash.o
.artifacts/hash_sha1.o: hash/sha1.o
	g++ -c $(CFLAGS) hash/sha1.cpp -o .artifacts/hash_sha1.o


#############################################
#				PKG: Network stuff			#
#############################################

LIB_OBJS_NET_OBJS = .artifacts/net_tcp_listen.o .artifacts/net_tcp_conn.o
.lib-objs-net: $(LIB_OBJS_NET_OBJS)
test-net_tcp: .artifacts/test-net_tcp
	.artifacts/test-net_tcp
.artifacts/test-net_tcp: .artifacts/test-net_tcp.o $(LIB_OBJS_NET_OBJS)
	g++ $(CFLAGS) .artifacts/test-net_tcp.o $(LIB_OBJS_NET_OBJS) -o .artifacts/test-net_tcp
.artifacts/test-net_tcp.o: net/test_tcp.cpp
	g++ -c $(CFLAGS) net/test_tcp.cpp -o .artifacts/test-net_tcp.o
.artifacts/net_tcp_listen.o: net/tcp_listen.cpp
	g++ -c $(CFLAGS) net/tcp_listen.cpp -o .artifacts/net_tcp_listen.o
.artifacts/net_tcp_conn.o: net/tcp_conn.cpp
	g++ -c $(CFLAGS) net/tcp_conn.cpp -o .artifacts/net_tcp_conn.o


#############################################
#				PKG: http interface			#
#############################################

LIB_OBJS_HTTP_OBJS = .artifacts/http_values.o .artifacts/http_method.o .artifacts/http_url_encode.o .artifacts/http_url_search_params.o .artifacts/http_url.o .artifacts/http_cookie.o .artifacts/http_date.o .artifacts/http_basic_auth.o .artifacts/http_sse.o
.lib-objs-http: $(LIB_OBJS_HTTP_OBJS)
test-http_url: .artifacts/test-http_url
	.artifacts/test-http_url
.artifacts/test-http_url: .artifacts/test-http_url.o $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_BASE64_OBJS)
	g++ $(CFLAGS) .artifacts/test-http_url.o $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_BASE64_OBJS) -o .artifacts/test-http_url
.artifacts/test-http_url.o: http/test_url.cpp
	g++ -c $(CFLAGS) http/test_url.cpp -o .artifacts/test-http_url.o
.artifacts/http_values.o: http/values.cpp
	g++ -c $(CFLAGS) http/values.cpp -o .artifacts/http_values.o
.artifacts/http_method.o: http/method.cpp
	g++ -c $(CFLAGS) http/method.cpp -o .artifacts/http_method.o
.artifacts/http_url_encode.o: http/url_encode.cpp
	g++ -c $(CFLAGS) http/url_encode.cpp -o .artifacts/http_url_encode.o
.artifacts/http_url_search_params.o: http/url_search_params.cpp
	g++ -c $(CFLAGS) http/url_search_params.cpp -o .artifacts/http_url_search_params.o
.artifacts/http_url.o: http/url.cpp
	g++ -c $(CFLAGS) http/url.cpp -o .artifacts/http_url.o
.artifacts/http_cookie.o: http/cookie.cpp
	g++ -c $(CFLAGS) http/cookie.cpp -o .artifacts/http_cookie.o
.artifacts/http_date.o: http/date.cpp
	g++ -c $(CFLAGS) http/date.cpp -o .artifacts/http_date.o
.artifacts/http_basic_auth.o: http/basic_auth.cpp
	g++ -c $(CFLAGS) http/basic_auth.cpp -o .artifacts/http_basic_auth.o
.artifacts/http_sse.o: http/sse.cpp
	g++ -c $(CFLAGS) http/sse.cpp -o .artifacts/http_sse.o


#############################################
#				SECTION: Logger				#
#############################################

LIB_OBJS_LOG_OBJS = .artifacts/log_fmt.o
.lib-objs-log: $(LIB_OBJS_LOG_OBJS)
test-log: .artifacts/test-log
	.artifacts/test-log
.artifacts/test-log: .artifacts/test-log.o $(LIB_OBJS_LOG_OBJS)
	g++ $(CFLAGS) .artifacts/test-log.o $(LIB_OBJS_LOG_OBJS) -o .artifacts/test-log
.artifacts/test-log.o: log/test_log.cpp
	g++ -c $(CFLAGS) log/test_log.cpp -o .artifacts/test-log.o
.artifacts/log_fmt.o: log/fmt.cpp
	g++ -c $(CFLAGS) log/fmt.cpp -o .artifacts/log_fmt.o


#############################################
#				PKG: server					#
#############################################

LIB_OBJS_SRV_OBJS = .artifacts/server.o .artifacts/server_pipeline_h1.o .artifacts/server_pipeline_h1_rx.o .artifacts/server_pipeline_h1_tx.o
.lib-objs-srv: $(LIB_OBJS_SRV_OBJS)
test-server: .artifacts/test-server
	.artifacts/test-server
.artifacts/test-server: .artifacts/test-server.o $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_LOG_OBJS)
	g++ $(CFLAGS) .artifacts/test-server.o $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_LOG_OBJS) -o .artifacts/test-server
.artifacts/test-server.o: server/test_server.cpp
	g++ -c $(CFLAGS) server/test_server.cpp -o .artifacts/test-server.o
.artifacts/server.o: server/server.cpp
	g++ -c $(CFLAGS) server/server.cpp -o .artifacts/server.o
.artifacts/server_pipeline_h1.o: server/pipeline_h1.cpp
	g++ -c $(CFLAGS) server/pipeline_h1.cpp -o .artifacts/server_pipeline_h1.o
.artifacts/server_pipeline_h1_rx.o: server/pipeline_h1_rx.cpp
	g++ -c $(CFLAGS) server/pipeline_h1_rx.cpp -o .artifacts/server_pipeline_h1_rx.o
.artifacts/server_pipeline_h1_tx.o: server/pipeline_h1_tx.cpp
	g++ -c $(CFLAGS) server/pipeline_h1_tx.cpp -o .artifacts/server_pipeline_h1_tx.o


#############################################
#				PKG: File server			#
#############################################

LIB_OBJS_FS_OBJS = .artifacts/fs_static_serve.o .artifacts/fs_dir_reader.o .artifacts/fs_mimetypes.o
.lib-objs-fs: $(LIB_OBJS_FS_OBJS)
test-fs: .artifacts/test-fs
	.artifacts/test-fs
.artifacts/test-fs: .artifacts/test-fs.o $(LIB_OBJS_FS_OBJS) $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_LOG_OBJS)
	g++ $(CFLAGS) .artifacts/test-fs.o $(LIB_OBJS_FS_OBJS) $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_LOG_OBJS) -o .artifacts/test-fs
.artifacts/test-fs.o: fs/test_fs.cpp
	g++ -c $(CFLAGS) fs/test_fs.cpp -o .artifacts/test-fs.o
.artifacts/fs_static_serve.o: fs/static_serve.cpp
	g++ -c $(CFLAGS) fs/static_serve.cpp -o .artifacts/fs_static_serve.o
.artifacts/fs_dir_reader.o: fs/dir_reader.cpp
	g++ -c $(CFLAGS) fs/dir_reader.cpp -o .artifacts/fs_dir_reader.o
.artifacts/fs_mimetypes.o: fs/mimetypes.cpp
	g++ -c $(CFLAGS) fs/mimetypes.cpp -o .artifacts/fs_mimetypes.o


#############################################
#				PKG: JSON					#
#############################################

LIB_OBJS_JSON_OBJS = .artifacts/json_stringify.o .artifacts/json_parse.o
.lib-objs-json: $(LIB_OBJS_JSON_OBJS)
test-json: .artifacts/test-json
	.artifacts/test-json
.artifacts/test-json: .artifacts/test-json.o $(LIB_OBJS_JSON_OBJS)
	g++ $(CFLAGS) .artifacts/test-json.o $(LIB_OBJS_JSON_OBJS) -o .artifacts/test-json
.artifacts/test-json.o: json/test_json.cpp
	g++ -c $(CFLAGS) json/test_json.cpp -o .artifacts/test-json.o
.artifacts/json_stringify.o: json/stringify.cpp
	g++ -c $(CFLAGS) json/stringify.cpp -o .artifacts/json_stringify.o
.artifacts/json_parse.o: json/parse.cpp
	g++ -c $(CFLAGS) json/parse.cpp -o .artifacts/json_parse.o


#############################################
#				PKG: Websocket				#
#############################################

LIB_OBJS_WS_OBJS = .artifacts/websocket.o
.lib-objs-ws: $(LIB_OBJS_WS_OBJS)
test-ws: .artifacts/test-ws
	.artifacts/test-ws
.artifacts/test-ws: .artifacts/test-ws.o $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_HASH_OBJS) $(LIB_OBJS_WS_OBJS) $(LIB_OBJS_LOG_OBJS)
	g++ $(CFLAGS) .artifacts/test-ws.o $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_HASH_OBJS) $(LIB_OBJS_WS_OBJS) $(LIB_OBJS_LOG_OBJS) -o .artifacts/test-ws
.artifacts/test-ws.o: ws/test_ws.cpp
	g++ -c $(CFLAGS) ws/test_ws.cpp -o .artifacts/test-ws.o
.artifacts/websocket.o: ws/websocket.cpp
	g++ -c $(CFLAGS) ws/websocket.cpp -o .artifacts/websocket.o


#############################################
#				LIBRARY BUILD				#
#############################################

LIB_FULL_OBJS = $(LIB_OBJS_NET_OBJS) $(LIB_OBJS_HTTP_OBJS) $(LIB_OBJS_SRV_OBJS) $(LIB_OBJS_FS_OBJS) $(LIB_OBJS_BASE64_OBJS) $(LIB_OBJS_JSON_OBJS) $(LIB_OBJS_WS_OBJS) $(LIB_OBJS_HASH_OBJS) $(LIB_OBJS_LOG_OBJS)
libshared: $(LAMBDA_LIBSHARED)
$(LAMBDA_LIBSHARED): $(LIB_FULL_OBJS)
	g++ $(CFLAGS) $(LIB_FULL_OBJS) -shared -o $(LAMBDA_LIBSHARED)
libstatic: $(LAMBDA_LIBSTATIC)
$(LAMBDA_LIBSTATIC): $(LIB_FULL_OBJS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LIB_FULL_OBJS)


#############################################
#					UTILS					#
#############################################

bin-page_gen: .artifacts/bin-page_gen
.artifacts/bin-page_gen: utils/page_gen/gen.cpp
	g++ -std=c++23 -s utils/page_gen/gen.cpp -o .artifacts/bin-page_gen
gen-page_404: .artifacts/bin-page_gen
	cat utils/page_gen/templates/page_404.html | .artifacts/bin-page_gen > .artifacts/generated-page_404.cpp


#############################################
#				Examples					#
#############################################

example-helloworld: .artifacts/example-helloworld
	.artifacts/example-helloworld
.artifacts/example-helloworld: .artifacts/example-helloworld.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-helloworld.o $(LIB_FULL_OBJS) -o .artifacts/example-helloworld
.artifacts/example-helloworld.o: examples/helloworld.cpp
	g++ -c $(CFLAGS) examples/helloworld.cpp -o .artifacts/example-helloworld.o

example-kvserver: .artifacts/example-kvserver
	.artifacts/example-kvserver
.artifacts/example-kvserver: .artifacts/example-kvserver.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-kvserver.o $(LIB_FULL_OBJS) -o .artifacts/example-kvserver
.artifacts/example-kvserver.o: examples/kvserver.cpp
	g++ -c $(CFLAGS) examples/kvserver.cpp -o .artifacts/example-kvserver.o

example-reply: .artifacts/example-reply
	.artifacts/example-reply
.artifacts/example-reply: .artifacts/example-reply.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-reply.o $(LIB_FULL_OBJS) -o .artifacts/example-reply
.artifacts/example-reply.o: examples/reply.cpp
	g++ -c $(CFLAGS) examples/reply.cpp -o .artifacts/example-reply.o

example-sse: .artifacts/example-sse
	.artifacts/example-sse
.artifacts/example-sse: .artifacts/example-sse.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-sse.o $(LIB_FULL_OBJS) -o .artifacts/example-sse
.artifacts/example-sse.o: examples/sse.cpp
	g++ -c $(CFLAGS) examples/sse.cpp -o .artifacts/example-sse.o

example-api: .artifacts/example-api
	.artifacts/example-api
.artifacts/example-api: .artifacts/example-api.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-api.o $(LIB_FULL_OBJS) -o .artifacts/example-api
.artifacts/example-api.o: examples/api.cpp
	g++ -c $(CFLAGS) examples/api.cpp -o .artifacts/example-api.o

example-ws: .artifacts/example-ws
	.artifacts/example-ws
.artifacts/example-ws: .artifacts/example-ws.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-ws.o $(LIB_FULL_OBJS) -o .artifacts/example-ws
.artifacts/example-ws.o: examples/ws.cpp
	g++ -c $(CFLAGS) examples/ws.cpp -o .artifacts/example-ws.o

example-static: .artifacts/example-static
	.artifacts/example-static
.artifacts/example-static: .artifacts/example-static.o $(LIB_FULL_OBJS)
	g++ $(CFLAGS) .artifacts/example-static.o $(LIB_FULL_OBJS) -o .artifacts/example-static
.artifacts/example-static.o: examples/static.cpp
	g++ -c $(CFLAGS) examples/static.cpp -o .artifacts/example-static.o
