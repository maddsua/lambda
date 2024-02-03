
# sample api server
EXAMPLE_APISERVER_TARGET = api_server.example$(EXEEXT)
example.api_server: $(EXAMPLE_APISERVER_TARGET)

$(EXAMPLE_APISERVER_TARGET): examples/api_server/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/api_server/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_APISERVER_TARGET)

examples/api_server/main.o: examples/api_server/main.cpp
	g++ -c $(CFLAGS) examples/api_server/main.cpp -o examples/api_server/main.o
