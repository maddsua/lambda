LIBNAME						=	lambda
PROD_FLAGS					=	$(if $(filter $(target),prod),-s,-g)
CFLAGS						=	-Wall -Werror -std=c++23 -fPIC $(PROD_FLAGS)
LAMBDA_LIBSTATIC			=	dist/$(LIBNAME).a
LAMBDA_LIBSHARED			=	dist/$(LIBNAME).so

.PHONY: all all-before all-after action-custom
all: all-before libshared libstatic all-after

clean: action-custom
	mkdir -p .artifacts
	mkdir -p dist
	rm .artifacts/* dist/*


#############################################
#			SECTION: Network stuff			#
#############################################

LIB_LAMBDA_NET_OBJS		=	.artifacts/net_tcp_listen.o .artifacts/net_tcp_conn.o
.lib-objs-net: $(LIB_LAMBDA_NET_OBJS)
test-net_tcp: .artifacts/test-net_tcp
	.artifacts/test-net_tcp
.artifacts/test-net_tcp: .artifacts/test-net_tcp.o $(LIB_LAMBDA_NET_OBJS)
	g++ $(CFLAGS) .artifacts/test-net_tcp.o $(LIB_LAMBDA_NET_OBJS) -o .artifacts/test-net_tcp
.artifacts/test-net_tcp.o: net/test_tcp.cpp
	g++ -c $(CFLAGS) net/test_tcp.cpp -o .artifacts/test-net_tcp.o
.artifacts/net_tcp_listen.o: net/tcp_listen.cpp
	g++ -c $(CFLAGS) net/tcp_listen.cpp -o .artifacts/net_tcp_listen.o
.artifacts/net_tcp_conn.o: net/tcp_conn.cpp
	g++ -c $(CFLAGS) net/tcp_conn.cpp -o .artifacts/net_tcp_conn.o


#############################################
#			SECTION: http interface			#
#############################################

LIB_LAMBDA_HTTP_OBJS	=	.artifacts/http_values.o .artifacts/http_url_encode.o .artifacts/http_url_search_params.o .artifacts/http_url.o .artifacts/http_cookie.o .artifacts/http_date.o
.lib-objs-http: $(LIB_LAMBDA_HTTP_OBJS)
test-http_url: .artifacts/test-http_url
	.artifacts/test-http_url
.artifacts/test-http_url: .artifacts/test-http_url.o $(LIB_LAMBDA_HTTP_OBJS)
	g++ $(CFLAGS) .artifacts/test-http_url.o $(LIB_LAMBDA_HTTP_OBJS) -o .artifacts/test-http_url
.artifacts/test-http_url.o: http/test_url.cpp
	g++ -c $(CFLAGS) http/test_url.cpp -o .artifacts/test-http_url.o
.artifacts/http_values.o: http/values.cpp
	g++ -c $(CFLAGS) http/values.cpp -o .artifacts/http_values.o
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


#############################################
#				SECTION: server				#
#############################################

LIB_LAMBDA_SRV_OBJS		=	.artifacts/server.o .artifacts/server_pipeline_h1.o .artifacts/server_pipeline_h1_rx.o  .artifacts/server_pipeline_h1_tx.o
.lib-objs-srv: $(LIB_LAMBDA_SRV_OBJS)
test-server: .artifacts/test-server
	.artifacts/test-server
.artifacts/test-server: .artifacts/test-server.o $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_NET_OBJS)
	g++ $(CFLAGS) .artifacts/test-server.o $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_NET_OBJS) -o .artifacts/test-server
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
#			SECTION: File server			#
#############################################

LIB_LAMBDA_FS_OBJS		=	.artifacts/fs_handler.o .artifacts/fs_static.o .artifacts/fs_mimetypes.o
.lib-objs-fs: $(LIB_LAMBDA_FS_OBJS)
test-fs: .artifacts/test-fs
	.artifacts/test-fs
.artifacts/test-fs: .artifacts/test-fs.o $(LIB_LAMBDA_FS_OBJS) $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_NET_OBJS)
	g++ $(CFLAGS) .artifacts/test-fs.o $(LIB_LAMBDA_FS_OBJS) $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_NET_OBJS) -o .artifacts/test-fs
.artifacts/test-fs.o: fs/test_fs.cpp
	g++ -c $(CFLAGS) fs/test_fs.cpp -o .artifacts/test-fs.o
.artifacts/fs_handler.o: fs/fs_handler.cpp
	g++ -c $(CFLAGS) fs/fs_handler.cpp -o .artifacts/fs_handler.o
.artifacts/fs_static.o: fs/fs_static.cpp
	g++ -c $(CFLAGS) fs/fs_static.cpp -o .artifacts/fs_static.o
.artifacts/fs_mimetypes.o: fs/mimetypes.cpp
	g++ -c $(CFLAGS) fs/mimetypes.cpp -o .artifacts/fs_mimetypes.o


#############################################
#				LIBRARY BUILD				#
#############################################

# shared lib build
libshared: $(LAMBDA_LIBSHARED)
$(LAMBDA_LIBSHARED): $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_FS_OBJS)
	g++ $(CFLAGS) $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_FS_OBJS) -shared -o $(LAMBDA_LIBSHARED)

# static lib build
libstatic: $(LAMBDA_LIBSTATIC)
$(LAMBDA_LIBSTATIC): $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_FS_OBJS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS) $(LIB_LAMBDA_FS_OBJS)


#############################################
#					UTILS					#
#############################################

bin-page_gen: .artifacts/bin-page_gen
.artifacts/bin-page_gen: utils/page_gen/gen.cpp
	g++ -std=c++23 -s utils/page_gen/gen.cpp -o .artifacts/bin-page_gen
gen-page_404: .artifacts/bin-page_gen
	cat utils/page_gen/templates/page_404.html | .artifacts/bin-page_gen > .artifacts/generated-page_404.cpp
