
# static server example
EXAMPLE_STATICSERVER_TARGET = static_server.example$(EXEEXT)
example.static_server: $(EXAMPLE_STATICSERVER_TARGET)

$(EXAMPLE_STATICSERVER_TARGET): examples/static_server/main.o $(LAMBDA_LIBSHARED)
	g++ $(CFLAGS) examples/static_server/main.o $(LAMBDA_LIBSHARED) $(EXTERNAL_LIBS) $(LINK_SYSTEM_LIBS) -o $(EXAMPLE_STATICSERVER_TARGET)

examples/static_server/main.o: examples/static_server/main.cpp
	g++ -c $(CFLAGS) examples/static_server/main.cpp -o examples/static_server/main.o
