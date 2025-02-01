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

LIB_LAMBDA_HTTP_OBJS	=	.artifacts/http_values.o .artifacts/http_url_encode.o .artifacts/http_url_search_params.o .artifacts/http_url.o
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


#############################################
#				SECTION: server				#
#############################################

LIB_LAMBDA_SRV_OBJS		=	.artifacts/server.o .artifacts/server_pipeline_h1.o .artifacts/server_pipeline_h1_rx.o  .artifacts/server_pipeline_h1_tx.o  .artifacts/server_date.o
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
.artifacts/server_date.o: server/date.cpp
	g++ -c $(CFLAGS) server/date.cpp -o .artifacts/server_date.o


#############################################
#				LIBRARY BUILD				#
#############################################

# shared lib build
libshared: $(LAMBDA_LIBSHARED)
$(LAMBDA_LIBSHARED): $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS)
	g++ $(CFLAGS) $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS) -shared -o $(LAMBDA_LIBSHARED)

# static lib build
libstatic: $(LAMBDA_LIBSTATIC)
$(LAMBDA_LIBSTATIC): $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS)
	ar rvs $(LAMBDA_LIBSTATIC) $(LIB_LAMBDA_NET_OBJS) $(LIB_LAMBDA_HTTP_OBJS) $(LIB_LAMBDA_SRV_OBJS)
